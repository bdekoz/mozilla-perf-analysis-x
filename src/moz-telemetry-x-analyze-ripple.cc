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
  std::string s("usage: moz-telemetry-x-analyze-ripple.exe data1.csv data2.csv");
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

  // Input CSV, JSON files.
  std::string idata1csv = argv[1];
  std::string idata2csv = argv[2];
  std::clog << "input files: " << std::endl
	    << idata1csv << std::endl
	    << idata2csv << std::endl;

#if 0
  svg_form obj = radiate_names_per_value_on_arc(idatacsv, 6, true);

  // Add environment metadata.
  string idataenv = file_path_to_stem(idatacsv) + extract_environment_ext;
  environment env = deserialize_environment(idataenv);
  render_environment_metadata(obj, env);
#endif

  return 0;
}
