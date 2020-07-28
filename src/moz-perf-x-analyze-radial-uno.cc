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
  std::string s("usage: moz-perf-x-analyze-radial-uno.exe data.csv");
  s += '\n';
  return s;
}


} // namespace moz


int main(int argc, char* argv[])
{
   using namespace moz;

   // Sanity check.
  if (argc != 2)
    {
      std::cerr << usage() << std::endl;
      return 1;
    }

  // Input is CSV file.
  std::string idatacsv = argv[1];
  std::clog << "input files: " << idatacsv << std::endl;

  const string hilite("rumSpeedIndex");

  // Create svg canvas.
  init_id_render_state_cache(0.33, hilite);
  set_label_spaces(6);
  const string fstem = file_path_to_stem(idatacsv);
  svg_element obj = initialize_svg(fstem);
  const point_2t origin = obj.center_point();
  render_radial(obj, idatacsv, origin, hilite);

  // Add metadata.
  environment env = deserialize_environment(idatacsv);
  render_metadata_environment(obj, env);

  return 0;
}
