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
initialize_svg()
{
  area<> a = { unit::pixel, 1920, 1080 };
  svg_form obj("moz-telemetry-radiating-lines.svg", a);

  group_form g;
  g.start_element("mozilla viz experiment 20181127.v2", k::b_style);
  g.finish_element();
  obj.add_element(g);

  return obj;
}


// Map a value to a point radiating out from a center.
void
radiate_probe_by_value(svg_form& obj, string probename, int pvalue, int pmax,
		       double r, color c = color::black)
{

}


// Create radial viz
void
radiating_probe_lines_viz()
{
  svg_form obj = initialize_svg();
}

} // namespace moz


int main(int argc, char* argv[])
{
  using namespace rapidjson;
  using namespace moz;

  // Sanity check.
  if (argc > 2)
    {
      std::cerr << usage() << std::endl;
      return 1;
    }

  // Input file, output directory.
  std::string ifile;
  if (argc > 1)
    {
      ifile = argv[1];
    }
  else
    {
      std::clog << usage() << std::endl;
      return 1;
    }
  std::clog << "input file: " << ifile << std::endl;

  // Extract data/values from json.


  return 0;
}
