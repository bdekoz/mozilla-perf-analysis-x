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
		"result-directory1 results-directory2");
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
  if (argc != 3)
    {
      std::cerr << usage() << endl;
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

  const string hilite("rumSpeedIndex");

  // Create svg canvas.
  init_id_render_state_cache(0.33, hilite);
  set_label_spaces(6);
  const svg::area canvas = svg::k::v1080p_h;
  auto [ width, height ] = canvas;

  // For each unique TLD/site in directories, use CSV file to do...
  for (uint i = 0; i < files1.size(); ++i)
    {
      const string& f1 = files1[i];
      const string& f2 = files2[i];

      const string fstem = file_path_to_stem(f1) + "-duo-ripple";
      svg_element obj = initialize_svg(fstem, width, height);
      auto xdelta = width / 4;

      auto [ x, y ] = obj.center_point();
      auto x1 = x - xdelta;
      auto x2 = x + xdelta;
      render_radial(obj, f1, point_2t(x1, y));
      render_radial(obj, f2, point_2t(x2, y));

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
      render_metadata_environment(obj, env);
    }

  return 0;
}
