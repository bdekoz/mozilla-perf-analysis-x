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

string
usage()
{
  string binname("moz-telemetry-x-analyze-radial-duo.exe");
  string s("usage:  " + binname + " csvdir1 csvdir2 "
	   "(metric-to-compare-or-highlight)");
  s += '\n';
  s += "csvdir1 is a CSV directory of browsertime JSON converted to csv files";
  s += "csvdir2 is a CSV directory of browsertime LOG converted to csv files";
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
  if (argc != 3 || argc != 4)
    {
      std::cerr << usage() << std::endl;
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

  // For each unique TLD/site in directories, use CSV files to do...
  for (uint i = 0; i < files1.size(); ++i)
    {
      const string& f1 = files1[i];
      const string& f2 = files2[i];

      const string fstem = file_path_to_stem(f1) + "-duo-ripple";

      clog << i << moz::k::tab << fstem << endl;
      clog << f1 << endl;
      clog << f2 << endl;

      svg_element obj = initialize_svg(fstem, width, height);
      const point_2t origin = obj.center_point();

      value_type value_max = largest_value_in_files(f1, f2);
      clog << "value_max: " << value_max << endl;

      render_radial(obj, origin, f1, hilite, value_max, 80, 24);
      render_radial(obj, origin, f2, hilite, value_max, 320, 24, false);

      clog << "done render" << endl;

      // Add metadata.
      environment env = deserialize_environment(f1);
      render_metadata_environment(obj, env);
    }

  return 0;
}
