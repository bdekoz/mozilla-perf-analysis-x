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
  string binname("moz-telemetry-x-analyze-ripple.exe");
  string s("usage:  " + binname + " data1.csv data2.csv (edit.txt)");
  s += '\n';
  s += "data1.csv is a JSON file containing a mozilla telemetry main ping";
  s += "data2.csv is a JSON file containing a browsertime results file";
  s += "edit.xt is an optional TEXT file containing higlight id matches";
  return s;
}

} // namespace moz


int main(int argc, char* argv[])
{
  using namespace rapidjson;
  using namespace moz;

   // Sanity check.
  if (argc != 3 || argc != 4)
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

  // Deserialize CSV files.
  int maxv1(0);
  id_value_map iv1 = deserialize_id_value_map(idata1csv, maxv1);

  int maxv2(0);
  id_value_map iv2 = deserialize_id_value_map(idata2csv, maxv2);

  // Find max value of all inputs.
  const int value_max(std::max(maxv1, maxv2));
  typography typo = make_typography_id();

  // Draw radial rings on canvas  from inner to outter ripple.
  // Size is inverse of denomenator argument below.

  // If a highlight input file exists, split the first
  // id_value_map object into a found matches object and a remaining object.
  // Otherwise, just use the first id_value_map as-is.
  if (argc == 3)
    {
      // 1. Moz Telemetry baseline ripple
      radiate_ids_per_value_on_arc(obj, typo, iv1, value_max, 7);
    }
  else
    {
      // Split map into highlights and remaining...
      string idatatxt = argv[3];
      strings matches;
      std::ifstream ifs(idatatxt);
      while (ifs.good())
	{
	  string line;
	  std::getline(ifs, line);

	  if (ifs)
	    matches.push_back(line);
	  else
	    break;
	}

      if (matches.empty())
	std::clog << "no matches found in: " << idatatxt << std::endl;

      // 1. Moz Telemetry baseline ripple.
      id_value_map iv1hi = remove_matches_id_value_map(iv1, matches);
      radiate_ids_per_value_on_arc(obj, typo, iv1, value_max, 7);

      // 2. Moz Telemetry highlight blue ripple, same size as first
      if (!iv1hi.empty())
	{
	  typography typohi = typo;
	  typohi._M_size = 18;
	  typohi._M_style._M_fill_color = colore::asagiiro;
	  radiate_ids_per_value_on_arc(obj, typohi, iv1hi, value_max, 7);
	}
    }

  // 3. Browsertime performance timings green ripple, next bigger size
  typography typobt = typo;
  typobt._M_style._M_fill_color = colore::green;
  radiate_ids_per_value_on_arc(obj, typobt, iv2, value_max, 5);

  // Add metadata.
  environment env1 = deserialize_environment(fstem1);
  environment env2 = deserialize_environment(fstem2);
  environment env = coalesce_environments(env1, env2);
  render_metadata_environment(obj, env);
  render_metadata_title(obj, fstem, value_max);

  return 0;
}
