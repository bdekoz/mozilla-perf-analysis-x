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

std::string
usage()
{
  std::string s("usage: moz-telemetry-x-analyze-radial.exe data.csv");
  return s;
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

  // Input CSV file.
  std::string idatacsv = argv[1];
  std::clog << "input files: " << idatacsv << std::endl;

 // Create svg canvas.
  const string fstem = file_path_to_stem(idatacsv);
  svg_form obj = initialize_svg(fstem);

  // Deserialize CSV files and find max value.
  int value_max(0);
  id_value_map iv = deserialize_id_value_map(idatacsv, value_max);
  radiate_ids_per_value_on_arc(obj, iv, value_max, 6, true);

  // Add metadata.
  environment env = deserialize_environment(fstem);
  render_metadata_environment(obj, env);
  render_metadata_title(obj, fstem, value_max);

  return 0;
}
