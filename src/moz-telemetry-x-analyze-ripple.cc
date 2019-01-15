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

#include <cmath>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>

#include "moz-telemetry-x-analyze.h"


namespace moz {

string
usage()
{
  string s("usage: moz-telemetry-x-analyze-ripple.exe data1.csv data2.csv");
  s += '\n';
  s += "data1.csv is a JSON file containing a mozilla telemetry main ping";
  s += "data2.csv is a JSON file containing a browsertime results file";
  return s;
}

} // namespace moz


int main(int argc, char* argv[])
{
  using namespace rapidjson;
  using namespace moz;

   // Sanity check.
  if (argc != 3)
    {
      std::cerr << usage() << std::endl;
      return 1;
    }

  // Input CSV files.
  string idata1csv = argv[1];
  string idata2csv = argv[2];
  std::clog << "input files: " << std::endl
	    << idata1csv << std::endl
	    << idata2csv << std::endl;

  // Create svg canvas.
  const string fstem1 = file_path_to_stem(idata1csv);
  const string fstem2 = file_path_to_stem(idata2csv);
  const string fstem(fstem1 + "-X-" + fstem2);
  svg_form obj = initialize_svg(fstem);

  // Deserialize CSV files and find max value.
  int maxv1(0);
  id_value_map iv1 = deserialize_id_value_map(idata1csv, maxv1);

  int maxv2(0);
  id_value_map iv2 = deserialize_id_value_map(idata2csv, maxv2);

  int value_max(std::max(maxv1, maxv2));


  // Then draw radial image from inner to outter ripple.
  // Size is inverse of denomenator argument below.

  // 1. Moz Telemetry baseline ripple
  radiate_ids_per_value_on_arc(obj, iv1, value_max, 7);

  // 2. Moz Telemetry highlight blue ripple, same size as first
  style histyl = k::b_style;
  histyl._M_fill_color = colore::asagiiro;

  // 3. Browsertime performance timings green ripple, next bigger size
  style browstyl = k::b_style;
  browstyl._M_fill_color = colore::green;
  radiate_ids_per_value_on_arc(obj, iv2, value_max, 5, browstyl);

  // Add metadata.
  environment env1 = deserialize_environment(fstem1);
  environment env2 = deserialize_environment(fstem2);
  environment env = coalesce_environments(env1, env2);
  render_metadata_environment(obj, env);
  render_metadata_title(obj, fstem, value_max);

  return 0;
}
