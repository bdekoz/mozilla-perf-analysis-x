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
		"result-directory1 results-directory2 "
		"(metric-to-compare-or-highlight)");
  s += '\n';
  return s;
}

} // namespace moz


int main(int argc, char* argv[])
{
   using namespace moz;
   using std::cerr;
   using std::clog;
   using std::endl;

   // Sanity check.
  if (argc != 3 && argc != 4)
    {
      cerr << usage() << endl;
      return 1;
    }

  // Input are CSV dirs with same number of csv files in each.
  string idata1 = argv[1];
  string idata2 = argv[2];
  clog << "input directories: " << endl
       << idata1 << endl
       << idata2 << endl;
  strings files1 = populate_files(idata1, ".csv");
  strings files2 = populate_files(idata2, ".csv");
  if (files1.empty() || files2.empty() || files1.size() != files2.size())
    {
      cerr << "error: input directories are not valid" << endl;
      cerr << files1.size() << " files in directory: " << idata1 << endl;
      cerr << files2.size() << " files in directory: " << idata2 << endl;
      return 2;
    }

  string hilite = "ContentfulSpeedIndex";
  if (argc == 4)
    hilite = argv[3];
  clog << "key metric: " << hilite << endl;

  // Create svg canvas.
  init_id_render_state_cache(0.33, hilite);
  set_label_spaces(6);
  const svg::area canvas = svg::k::v1080p_h;
  auto [ width, height ] = canvas;

  const bool scalep = true;

  // For each unique TLD/site in directories, use CSV files to do...
  for (uint i = 0; i < files1.size(); ++i)
    {
      const string& f1 = files1[i];
      const string& f2 = files2[i];

      const string fstem = file_path_to_stem(f1) + "-duo-side-by-side";
      svg_element obj = initialize_svg(fstem, width, height);

      // Scaling, if desired.
      value_type vmax = 0;
      int radius1 = 80;
      int rspace1 = 24;
      int radius2 = 80;
      int rspace2 = 24;
      if (scalep)
	{
	  value_type max1 = largest_value_in(f1);
	  value_type max2 = largest_value_in(f2);
	  if (max1 > max2)
	    {
	      double ratio = max1 / max2;
	      radius1 *= ratio;
	    }
	  else
	    {
	      double ratio = max2 / max1;
	      radius2 *= ratio;
	    }
	}

      // Find arc centers.
      auto [ x, y ] = obj.center_point();
      const auto xdelta = width / 4;
      const auto x1 = x - xdelta;
      const auto x2 = x + xdelta;

      // Draw arcs.
      render_radial(obj, point_2t(x1, y), f1, hilite, vmax, radius1, rspace1);
      render_radial(obj, point_2t(x2, y), f2, hilite, vmax, radius2, rspace2);

      // Add metadata.
      environment env;
      try
	{
	  environment env1 = deserialize_environment(f1);
	  environment env2 = deserialize_environment(f2);
	  env = coalesce_environments(env1, env2);
	}
      catch (const std::runtime_error& e)
	{
	  env = deserialize_environment(f1);
	}
      render_metadata(obj, env, hilite, true);
    }

  return 0;
}
