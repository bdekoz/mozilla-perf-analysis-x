// telemetry radial, sunburst / RAIL forms -*- mode: C++ -*-

// Copyright (c) 2018-2020, Mozilla
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

#include "moz-perf-x-radial.h"


namespace moz {

std::string
usage()
{
  std::string s("usage: moz-perf-x-analyze-radial-duo-side-by-side.exe "
		"data1.csv data2.csv");
  s += '\n';
  return s;
}

} // namespace moz


int main(int argc, char* argv[])
{
   using namespace moz;

   // Sanity check.
  if (argc != 3)
    {
      std::cerr << usage() << std::endl;
      return 1;
    }

  // Input are CSV dirs with same number of csv files in each.
  std::string idata1 = argv[1];
  std::string idata2 = argv[2];
  std::clog << "input directories: " << std::endl
	    << idata1 << std::endl
	    << idata2 << std::endl;

  const string hilite("rumSpeedIndex");

  // Create svg canvas.
  init_id_render_state_cache(0.33, hilite);
  set_label_spaces(6);

  // For each unique TLD/site in directories, use CSV file to do...
  for (uint i = 0; i < sz; ++i)
    {
      const string fstem = file_path_to_stem(idata1);
      svg_element obj = initialize_svg(fstem);
      auto xdelta = obj._M_area._M_width / 4;

      auto [ x, y ] = obj.center_point();
      auto x1 = x - xdelta;
      auto x2 = x + xdelta;
      render_radial(obj, idata1, point_2t(x1, y));
      render_radial(obj, idata2, point_2t(x2, y));

      // Add metadata.
      environment env = deserialize_environment(idata1);
      render_metadata_environment(obj, env);
    }

  return 0;
}
