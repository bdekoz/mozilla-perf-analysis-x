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
initialize_svg()
{
  area<> a = { unit::pixel, 1920, 1080 };
  svg_form obj("moz-telemetry-radiating-lines", a);

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
  // Read CSV file of [marker name || probe name] and value, and
  // store in hash_map, along with the max value.
  std::unordered_map<string, int> probe_map;
  int probe_key_max(0);

  // Read probe names from input file, and put into vector<string>
  std::ifstream ifs(prefixpath + tier1outfile);
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

	      //std::clog << pname << " -> " << pvalue << std::endl;
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


  svg_form obj = initialize_svg();
}

} // namespace moz


int main(int argc, char* argv[])
{
  using namespace rapidjson;
  using namespace moz;

  // Extract data/values computed previously and draw.
  radiating_probe_lines_viz();

  return 0;
}
