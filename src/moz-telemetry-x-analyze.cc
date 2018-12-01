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
place_metadata_text(svg_form& obj, typography& typo, string mtext)
{
  // Margin in pixels.
  const int margin = 100;
  int tx = margin;
  static int ty = margin;

  text_form::data dt = { tx, ty, mtext, typo };
  text_form t;
  t.start_element();
  t.add_data(dt);
  t.finish_element();
  obj.add_element(t);

  // Increment vertical, assume higher moves text down the page.
  ty += typo._M_size;
}


void
place_probe_text(svg_form& obj, typography& typo, string label, int tx, int ty,
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
radiate_probe_by_value(svg_form& obj, string pname, int pvalue, int pmax,
		       double r, bool rotatep = false)
{
  // Find center of SVG canvas.
  const double cx = obj._M_area._M_width / 2;
  const double cy = obj._M_area._M_height / 2;

  // Normalize [0, pmax] to range [0, 360] and put pvalue in it.
  //const double kangle = (360 / pmax) * static_cast<double>(pvalue);
  int angled = normalize_on_range(pvalue, 0, pmax, 0, 359);
  std::clog << pname << " -> " << pvalue << " " << angled << std::endl;

  /*
    Draw text on the circumference of a circle of radius r centered (cx, cy)
    corresponding to the angle above.
  */
  constexpr double kpi(22/7);
  double angler = (kpi / 180) * angled;
  double x(cx + (r * std::cos(angler)));
  double y(cy - (r * std::sin(angler)));

  // Consolidate label text to be "VALUE -> NAME"
  std::ostringstream oss;
  oss.imbue(std::locale(""));
  oss << std::setfill(' ') << std::setw(6);
  oss << pvalue << " -> " << pname;
  string label = oss.str();

  // Common typographics.
  typography typo = k::ccode_typo;
  typo._M_size = 12;
  typo._M_style = k::b_style;
  typo._M_w = svg::typography::weight::xlight;
  // typo._M_style._M_fill_color = c;

  if (rotatep)
    {
      typo._M_a = svg::typography::anchor::start;
      typo._M_align = svg::typography::align::left;
      //      typo._M_a = svg::typography::anchor::middle;
      place_probe_text(obj, typo, label, x, y, angled);
    }
  else
    {
      typo._M_align = svg::typography::align::left;
      typo._M_a = svg::typography::anchor::start;
      place_probe_text(obj, typo, label, x, y, 0);
    }
}


void
place_metadata(svg_form& obj, environment& env)
{
  // Common typographics.
  typography typo = k::zslab_typo;
  typo._M_align = svg::typography::align::left;
  typo._M_a = svg::typography::anchor::start;
  typo._M_size = 14;
  typo._M_style = k::b_style;
  typo._M_w = svg::typography::weight::medium;
  typo._M_style._M_fill_color = colore::gray50;

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


// Create radial viz
void
radiating_probe_lines_viz(string ifile)
{
  // Read CSV file of [marker name || probe name] and value, and
  // store in hash_map, along with the max value.
  std::unordered_map<string, int> probe_map;
  int probe_key_max(0);

  const string fstem = file_path_to_stem(ifile);
  const string ifilecsv(datapath + fstem + extract_ext);
  file_path_to_stem(ifilecsv);

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
	      probe_key_max = std::max(pvalue, probe_key_max);
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
	    << probe_key_max << std::endl;

  // Create svg canvas.
  svg_form obj = initialize_svg(fstem);

  // Probe/Marker display.
  // Loop through map key/values and put on canvas.
  for (const auto& v : probe_map)
    {
      string pname(v.first);
      int pvalue(v.second);
      double r = std::min(obj._M_area._M_height, obj._M_area._M_width) / 4;

      // Don't Rotate.
      //radiate_probe_by_value(obj, pname, pvalue, probe_key_max, r, false);

      // Rotate
      radiate_probe_by_value(obj, pname, pvalue, probe_key_max, r, true);
    }

  // Metadata display.
  environment env = extract_environment(ifile);
  place_metadata(obj, env);
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

  radiating_probe_lines_viz(idata);

  return 0;
}
