// telemetry radial, sunburst / RAIL forms -*- mode: C++ -*-

// Copyright (c) 2018, Mozilla
// Benjamin De Kosnik <bdekoz@mozilla.com>

// This file is part of the MOZILLA TELEMETRY X library.

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.

// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

#include <cmath>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>

#include "moz-json-basic.h"
#include "a60-svg-base.h"


namespace moz {

using namespace svg;
using color = svg::colore;

std::string
usage()
{
  std::string s("usage: moz-telemetry-x-analyze.exe data.json");
  return s;
}


// Create an svg object with 1080p dimensions and return it.
svg_form
initialize_svg(string ofile = "moz-telemetry-radiating-lines")
{
  area<> a = { unit::pixel, 1920, 1080 };
  svg_form obj(ofile, a);

  group_form g;
  g.start_element("mozilla viz experiment 20181127.v2", k::b_style);
  g.finish_element();
  obj.add_element(g);

  return obj;
}


void
place_text_at_point(svg_form& obj, typography& typo, string mtext,
		    int tx, int ty)
{
  text_form::data dt = { tx, ty, mtext, typo };
  text_form t;
  t.start_element();
  t.add_data(dt);
  t.finish_element();
  obj.add_element(t);
}


void
place_metadata_text(svg_form& obj, typography& typo, string mtext)
{
  int tx = margin;
  static int ty = margin;

  place_text_at_point(obj, typo, mtext, tx, ty);

  // Increment vertical, assume higher moves text down the page.
  ty += typo._M_size;
}


void
place_name_text(svg_form& obj, typography& typo, string label, int tx, int ty,
		 const double deg = 0.0)
{
  text_form::data dt = { tx, ty, label, typo };
  text_form t;
  t.start_element();

  // IFF degrees, then rotate text.
  if (deg > 0)
    t.add_data(dt, svg::transform::rotate(360 - deg, tx, ty));
  else
    t.add_data(dt);

  t.finish_element();
  obj.add_element(t);
}


double
normalize_on_range(uint value, uint min, uint max, uint nfloor, uint nceil)
{
  auto weightn_numer = ((nceil - nfloor) * (value - min));
  auto weightn_denom = (max - min) + nfloor;
  auto weightn = std::round(weightn_numer / weightn_denom);
  return weightn;
}


// Map a value to a point radiating out from a center.
void
radiate_name_by_value(svg_form& obj, typography& typo, string pname,
		       int pvalue, int pmax, double r, bool rotatep = true)
{
  // Find center of SVG canvas.
  const double cx = obj._M_area._M_width / 2;
  const double cy = obj._M_area._M_height / 2;

  // Max number of non-overlapping degrees in circle.
  const double maxdeg = 359;

  // Normalize [0, pmax] to range [0, maxdeg] and put pvalue in it.
  double angled = normalize_on_range(pvalue, 0, pmax, 0, maxdeg);

  // Change rotation to CW instead of CCW (or anti-clockwise).
  angled = 360 - angled;

  // Rotate 90 CCW, so that the first element will be at the top
  // vertical axis, instead of the right middle axis.
  angled += 90;

  /*
    Draw text on the circumference of a circle of radius r centered (cx, cy)
    corresponding to the angle above.
  */
  constexpr double kpi(3.14159);
  double angler = (kpi / 180.0) * angled;
  double x(cx + (r * std::cos(angler)));
  double y(cy - (r * std::sin(angler)));

  // Consolidate label text to be "VALUE -> NAME"
  constexpr uint valuewidth(9);
  std::ostringstream oss;
  oss.imbue(std::locale(""));
  oss << std::setfill(' ') << std::setw(valuewidth) << std::left << pvalue;
  string label = oss.str() + " -> " + pname;

  if (rotatep)
    place_name_text(obj, typo, label, x, y, angled);
  else
    place_name_text(obj, typo, label, x, y, 0);

  std::clog << label << " " << angled << " " << '(' << x << ',' << y << ')'
	    << std::endl;
}


void
place_metadata(svg_form& obj, typography& typo, environment& env)
{
  // place_metadata_text(obj, typo, env.os_vendor);
  place_metadata_text(obj, typo, env.os_name);
  place_metadata_text(obj, typo, env.os_version);
  place_metadata_text(obj, typo, env.os_locale);

  place_metadata_text(obj, typo, " ");

  typo._M_size = 20;
  place_metadata_text(obj, typo, to_string(env.hw_cpu) + " cores");
  int memi = std::round(env.hw_mem * .001);
  place_metadata_text(obj, typo, to_string(memi) + " GB");
  typo._M_size = 14;

  place_metadata_text(obj, typo, " ");

  place_metadata_text(obj, typo, env.sw_name);
  place_metadata_text(obj, typo, env.sw_arch);
  place_metadata_text(obj, typo, env.sw_version);
  place_metadata_text(obj, typo, env.sw_build_id);

  place_metadata_text(obj, typo, " ");

  place_metadata_text(obj, typo, to_string(env.fx_uri_count) + " uri count");
}


// Given rdenom scaling factor and SVG canvas, compute radius value.
inline double
find_radius(const svg_form& obj, const uint rdenom)
{ return std::min(obj._M_area._M_height, obj._M_area._M_width) / rdenom; }


/*
  Create radial viz of names from input file arranged clockwise around
  the edge of a circle circumference. The text of the names can be
  rotated, or not.

 Arguments are:

 ifile == CSV or JSON file of extracted marker/probe names to display.

 rdenom == scaling factor for radius of circle used for display, where
  larger values (used as a denominator) make smaller (tighter) circles.

 rotatep == rotate name text to be on an arc from the origin of the circle.
*/
void
radiate_names_per_value_on_arc(string ifile, const uint rdenom, bool rotatep)
{
  // Read CSV file of [marker name || probe name] and value, and
  // store in hash_map, along with the max value.
  std::unordered_map<string, int> probe_map;
  int probe_value_max(0);

  string fstem;
  string ifilecsv;
  bool jsonp = false;
  if (ifile.find(".json") != string::npos)
    {
      fstem = file_path_to_stem(ifile);
      ifilecsv = string(datapath + fstem + extract_ext);
      file_path_to_stem(ifilecsv);
      jsonp = true;
    }
  else
    {
      fstem = file_path_to_stem(ifile);
      ifilecsv = ifile;
    }

  std::ifstream ifs(ifilecsv);
  if (ifs.good())
    {
      do
	{
	  string pname;
	  getline(ifs, pname, ',');
	  if (ifs.good())
	    {
	      int pvalue;
	      ifs >> pvalue;

	      // Extract remaining newline.
	      ifs.ignore(1, k::newline);

	      probe_map.insert(make_pair(pname, pvalue));
	      probe_value_max = std::max(pvalue, probe_value_max);
	    }
	}
      while (ifs.good());
    }
  else
    {
      std::cerr << errorprefix << "cannot open input file "
		<< ifile << std::endl;
    }
  std::clog << probe_map.size() << " probes found with max value "
	    << probe_value_max << std::endl;

  // Create svg canvas.
  svg_form obj = initialize_svg(fstem);

  // Circle glyph for center of arc.
  const double cx = obj._M_area._M_width / 2;
  const double cy = obj._M_area._M_height / 2;
  const double r = find_radius(obj, rdenom);

  std::clog << "r is " << r << std::endl;
  std::clog << "origin (x,y) == " << cx << ',' << cy << std::endl;

  style sty = k::b_style;
  sty._M_stroke_color = colore::red;
  sty._M_stroke_opacity = 1.0;
  sty._M_stroke_size = 2.0;
  sty._M_fill_opacity = 0.0;
  point_2d_to_circle(obj, cx, cy, sty, r/2);

  // Probe/Marker name/value typographics.
  typography typo = k::ccode_typo;
  typo._M_size = 9;
  typo._M_style = k::b_style;
  typo._M_w = svg::typography::weight::xlight;
  typo._M_align = svg::typography::align::left;
  typo._M_a = svg::typography::anchor::start;

  // Probe/Marker display.
  // Loop through map key/values and put on canvas.
  for (const auto& v : probe_map)
    {
      string pname(v.first);
      int pvalue(v.second);
      if (pvalue)
	radiate_name_by_value(obj, typo, pname, pvalue, probe_value_max, r,
			      rotatep);
    }

  // Metadata typographics.
  typography typom = k::zslab_typo;
  typom._M_align = svg::typography::align::left;
  typom._M_a = svg::typography::anchor::start;
  typom._M_size = 14;
  typom._M_style = k::b_style;
  typom._M_w = svg::typography::weight::medium;
  typom._M_style._M_fill_color = colore::gray50;

  // Metadata display.
  if (jsonp)
    {
      environment env = extract_environment(ifile);
      place_metadata(obj, typom, env);
    }

  // Input data file.
  place_text_at_point(obj, typom, file_path_to_stem(ifile),
		      margin, obj._M_area._M_height - margin);

  // Total time.
  typom._M_size = 48;
  typom._M_style._M_fill_color = colore::red;
  place_text_at_point(obj, typom, to_string(probe_value_max) + " ms",
		      margin, obj._M_area._M_height - margin - 14 * 2);
}

} // namespace moz


int main(int argc, char* argv[])
{
  using namespace rapidjson;
  using namespace moz;

   // Sanity check.
  if (argc != 2)
    {
      std::cerr << usage() << std::endl;
      return 1;
    }

  // Input names file, input data file
  std::string idata = argv[1];
  std::clog << "input files: " << idata << std::endl;

  radiate_names_per_value_on_arc(idata, 6, true);

  return 0;
}
