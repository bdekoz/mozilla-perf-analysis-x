// telemetry radial, sunburst / RAIL forms -*- mode: C++ -*-

// Copyright (c) 2018-2019, Mozilla
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

#ifndef moz_TELEMETRY_X_ANALYZE_H
#define moz_TELEMETRY_X_ANALYZE_H 1

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
using id_value_map = std::unordered_map<string, int>;


// Create an svg object with 1080p dimensions and return it.
svg_form
initialize_svg(const string ofile = "moz-telemetry-radiating-lines",
	       const int width = 1920, const int height = 1080)
{
  area<> a = { unit::pixel, width, height };
  svg_form obj(ofile, a);

  group_form g;
  g.start_element("mozilla viz experiment 20190102.v3");
  g.finish_element();
  obj.add_element(g);

  // Read SVG to insert.
  std::string ifile(datapath + "image/circle-arrow-red.svg");
  std::ifstream ifs(ifile);
  string isvg;
  if (ifs.good())
    {
      // Strip out any XML version line in the SVG file.
      // Search for and discard lines with "xml version", iff exists
      string xmlheader;
      getline(ifs, xmlheader);
      if (xmlheader.find("xml version") == string::npos)
	ifs.seekg(0, ifs.beg);

      std::ostringstream oss;
      oss << ifs.rdbuf();
      isvg = oss.str();
    }
  else
    throw std::runtime_error("initialize_svg:: insertion of nested SVG failed");

  // Insert nested SVG element of red arc with arrow (scaled and with offset).
  const int isize(182);
  const int x = width / 2 - isize / 2;
  const int y = height / 2 - isize / 2;

  string ts(transform::translate(x, y));
  group_form gsvg;
  gsvg.start_element("mozilla inset radial svg", transform(), ts);
  gsvg.add_raw(isvg);
  gsvg.finish_element();
  obj.add_element(gsvg);

   return obj;
}


typography
make_typography_id()
{
  // Probe/Marker name/value typographics.
  typography typo = k::ccode_typo;
  typo._M_size = 9;
  typo._M_style = k::b_style;
  typo._M_w = svg::typography::weight::xlight;
  typo._M_align = svg::typography::align::left;
  typo._M_a = svg::typography::anchor::start;
  return typo;
}


typography
make_typography_metadata()
{
  // Metadata typographics.
  typography typom = k::zslab_typo;
  typom._M_align = svg::typography::align::left;
  typom._M_a = svg::typography::anchor::start;
  typom._M_size = 14;
  typom._M_style = k::b_style;
  typom._M_w = svg::typography::weight::medium;
  typom._M_style._M_fill_color = colore::gray50;
  return typom;
}


void
place_text_at_point(svg_form& obj, const typography& typo, const string mtext,
		    const int tx, const int ty)
{
  text_form::data dt = { tx, ty, mtext, typo };
  text_form t;
  t.start_element();
  t.add_data(dt);
  t.finish_element();
  obj.add_element(t);
}


void
place_text_metadata(svg_form& obj, const typography& typo, string mtext)
{
  int tx = margin;
  static int ty = margin;

  if (!mtext.empty())
    {
      place_text_at_point(obj, typo, mtext, tx, ty);

      // Increment vertical, assume higher moves text down the page.
      ty += typo._M_size;
    }
}


void
place_text_id(svg_form& obj, const typography& typo, string label,
	      int tx, int ty, const double deg = 0.0)
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


void
place_metadata(svg_form& obj, const typography& typo, const environment& env)
{
  // place_text_metadata(obj, typo, env.os_vendor);
  place_text_metadata(obj, typo, env.os_name);
  place_text_metadata(obj, typo, env.os_version);
  place_text_metadata(obj, typo, env.os_locale);

  place_text_metadata(obj, typo, " ");

  typography typolarge = typo;
  typolarge._M_size = 20;
  place_text_metadata(obj, typolarge, to_string(env.hw_cpu) + " cores");
  int memi = std::round(env.hw_mem * .001);
  place_text_metadata(obj, typolarge, to_string(memi) + " GB");

  place_text_metadata(obj, typo, " ");

  place_text_metadata(obj, typo, env.sw_name);
  place_text_metadata(obj, typo, env.sw_arch);
  place_text_metadata(obj, typo, env.sw_version);
  place_text_metadata(obj, typo, env.sw_build_id);

  place_text_metadata(obj, typo, " ");

  place_text_metadata(obj, typo, to_string(env.uri_count) + " uri count");
  place_text_metadata(obj, typo, env.url);
  place_text_metadata(obj, typo, env.date_time_stamp);
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
radiate_name_by_value(svg_form& obj, const typography& typo, string pname,
		       int pvalue, int pmax, double r, bool rotatep)
{
  // Find center of SVG canvas.
  const double cx = obj._M_area._M_width / 2;
  const double cy = obj._M_area._M_height / 2;

  // Max number of non-overlapping degrees in circle, such that the
  // beginning and the end have a discernable gap. Total degrees in a
  // circle are 360, but then the beginning and the end of the radial
  // display are in the same place.
  const double maxdeg = 345;

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
    place_text_id(obj, typo, label, x, y, angled);
  else
    place_text_id(obj, typo, label, x, y, 0);

  std::clog << label << " " << angled << " " << '(' << x << ',' << y << ')'
	    << std::endl;
}


// Given rdenom scaling factor and SVG canvas, compute effective radius value.
inline double
find_radius(const svg_form& obj, const uint rdenom)
{ return std::min(obj._M_area._M_height, obj._M_area._M_width) / rdenom; }


// Read CSV file of [marker name || probe name] and value, and
// store in hash_map, return this plus the max value as a tuple.
id_value_map
deserialize_id_value_map(const string ifile, int& value_max)
{
  id_value_map probe_map;
  std::ifstream ifs(ifile);
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
	      ifs.ignore(79, k::newline);

	      probe_map.insert(make_pair(pname, pvalue));
	      value_max = std::max(pvalue, value_max);
	    }
	}
      while (ifs.good());
    }
  else
    {
      std::cerr << errorprefix << "cannot open input file "
		<< ifile << std::endl;
    }
  std::clog << probe_map.size() << " ids found with max value "
	    << value_max << std::endl;
  return probe_map;
}


// Remove all from map that match the input (matches) strings.
// Return found match entries.
id_value_map
remove_matches_id_value_map(id_value_map& ivm, const strings& matches)
{
  id_value_map foundmap;
  for (const auto& key: matches)
    {
      id_value_map::iterator iter = ivm.find(key);
      if (iter != ivm.end())
	{
	  // Insert found element into return map....
	  foundmap.insert(*iter);

	  // Remove found elment from originating map (ivm)
	  ivm.erase(iter);
	}
      else
	std::clog << errorprefix << key << std::endl;
    }
  return foundmap;
}


/*
  Create radial viz of names from input file arranged clockwise around
  the edge of a circle circumference. The text of the names can be
  rotated, or not.

 Arguments are:

 ifile == CSV file of extracted marker/probe names to display.

 rdenom == scaling factor for radius of circle used for display, where
  larger values (used as a denominator) make smaller (tighter) circles.

 rotatep == rotate name text to be on an arc from the origin of the circle.

*/
svg_form
radiate_ids_per_value_on_arc(svg_form& obj, const typography& typo,
			     const id_value_map& probe_map,
			     const int value_max, const int rdenom,
			     bool rotatep = true)
{
  // Probe/Marker display.
  // Loop through map key/values and put on canvas.
  const double r = find_radius(obj, rdenom);
  for (const auto& v : probe_map)
    {
      string pname(v.first);
      int pvalue(v.second);
      if (pvalue)
	radiate_name_by_value(obj, typo, pname, pvalue, value_max, r, rotatep);
    }

  return obj;
}


// Metadata display.
void
render_metadata_environment(svg_form& obj, const environment& env)
{
  typography typom = make_typography_metadata();
  place_metadata(obj, typom, env);
}


void
render_metadata_title(svg_form& obj, const int time_max, const string fstem1,
		      const string fstem2 = "", const string hilights = "")
{
  // Vertical offset.
  const typography typom = make_typography_metadata();
  int y = obj._M_area._M_height - margin;
  int ydelta  = typom._M_size * 1.5;

  // Input data files.
  place_text_at_point(obj, typom, fstem1, margin, y);
  y -= ydelta;

  if (!fstem2.empty())
    {
      place_text_at_point(obj, typom, fstem2, margin, y);
      y -= ydelta;
    }

  if (!hilights.empty())
    {
      place_text_at_point(obj, typom, hilights, margin, y);
      y -= ydelta;
    }

  // Total time.
  typography typot(typom);
  typot._M_size = 48;
  typot._M_style._M_fill_color = colore::red;
  std::ostringstream oss;
  oss.imbue(std::locale(""));
  oss << time_max << " ms";
  place_text_at_point(obj, typot, oss.str(), margin, obj._M_area._M_height / 2);
}

} // namespace moz

#endif
