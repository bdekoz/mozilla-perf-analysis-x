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
#include <algorithm>
#include <unordered_map>

#include "moz-json-basic.h"
#include "moz-telemetry-x.h"
#include "a60-svg-base.h"


namespace moz {

using namespace svg;
using color = svg::colore;

std::string
usage()
{
  std::string s("usage: a60-analyze file.json");
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
place_probe_text(svg_form& obj, string label, int tx, int ty,
		 const double deg = 0.0, const color c = color::black)
{
  // Common typographics.
  svg::typography typo = k::apercu_typo;
  typo._M_size = 12;
  typo._M_a = svg::typography::anchor::start;
  typo._M_align = svg::typography::align::left;
  typo._M_style = k::b_style;
  typo._M_w = svg::typography::weight::xlight;
  typo._M_style._M_fill_color = c;

  text_form::data dt = { tx, ty, label, typo };
  text_form t;
  t.start_element();

  // IFF degrees, then rotate text.
  // NB or XXX, should rotations be about the center axis?
  if (false && deg > 0)
    t.add_data(dt, svg::transform::rotate(deg, tx, ty));
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
		       double r)
{
  // Find center of SVG canvas.
  const double cx = obj._M_area._M_width / 2;
  const double cy = obj._M_area._M_height / 2;

  // Normalize [0, pmax] to range [0, 360] and put pvalue in it.
  //const double kangle = (360 / pmax) * static_cast<double>(pvalue);
  int kangle = normalize_on_range(pvalue, 0, pmax, 0, 360);
  std::clog << pname << " -> " << pvalue << " " << kangle << std::endl;

  /*
    Draw text on the circumference of a circle of radius r centered (cx, cy)
    corresponding to the angle above.

    Divide into 4 quadrants.

    Quad 1 [0-90]
    Quad 2 [91-180]
    Quad 2 [181-270]
    Quad 4 [271-360]
  */
  enum quadrant { q1, q2, q3, q4 };
  quadrant q;
  if (0 <= kangle && kangle <= 90)
    q = q1;
  else if (90 < kangle && kangle <= 180)
    q = q2;
  else if (180 < kangle && kangle <= 270)
    q = q3;
  else
    q = q4;

  double x(0);
  double y(0);
  switch (q)
    {
    case q1:
       x = cx + (r * std::cos(kangle));
       y = cy - (r * std::sin(kangle));
      break;
    case q2:
       x = cx - (r * std::cos(kangle));
       y = cy - (r * std::sin(kangle));
      break;
    case q3:
       x = cx - (r * std::cos(kangle));
       y = cy + (r * std::sin(kangle));
      break;
    case q4:
       x = cx + (r * std::cos(kangle));
       y = cy + (r * std::sin(kangle));
      break;
    default:
      throw std::runtime_error("radiate out of quadrant");
      break;
    }

  // Consolidate label text to be "VALUE -> NAME"
  string label = std::to_string(pvalue) + " -> " + pname;
  place_probe_text(obj, label, x, y, kangle);

}


// Create radial viz
void
radiating_probe_lines_viz(string ifile)
{
  // Read CSV file of [marker name || probe name] and value, and
  // store in hash_map, along with the max value.
  std::unordered_map<string, int> probe_map;
  int probe_key_max(0);

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

	      probe_map.insert(make_pair(pname, pvalue));
	      probe_key_max = std::max(pvalue, probe_key_max);
	    }
	}
      while (ifs.good());
    }
  else
    {
      std::cerr << errorprefix
		<< "error: cannot open input file "
		<< datapath + tier1file << std::endl;
    }
  std::clog << probe_map.size() << " probes found with max value "
	    << probe_key_max << std::endl;

  // Create svg canvas.
  svg_form obj = initialize_svg();

  // Loop through map key/values and put on canvas.
  for (const auto& v : probe_map)
    {
      string pname(v.first);
      int pvalue(v.second);
      double r = std::min(obj._M_area._M_height, obj._M_area._M_width) / 4;
      radiate_probe_by_value(obj, pname, pvalue, probe_key_max, r);
    }

}

} // namespace moz


int main(void)
{
  using namespace rapidjson;
  using namespace moz;

  // Extract data/values computed previously and draw.
  string ifile(prefixpath + testfile);
  //string ifile(prefixpath + tier1outfile);

  radiating_probe_lines_viz(ifile);

  return 0;
}
