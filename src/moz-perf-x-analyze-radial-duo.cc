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

#include "moz-perf-x-json.h"


namespace moz {

string
usage()
{
  string binname("moz-telemetry-x-analyze-radial-duo.exe");
  string s("usage:  " + binname + " data1.csv data2.csv (edit.txt)");
  s += '\n';
  s += "data1.csv is a CSV file containing a firefox telemetry main ping";
  s += '\n';
  s += "data2.csv is a CSV file containing firefox browsertime results";
  s += '\n';
  s += "edit.xt is an optional TEXT file containing higlight id matches";
  s += '\n';
  return s;
}

} // namespace moz


int main(int argc, char* argv[])
{
  using namespace moz;

   // Sanity check.
  if (argc < 3 || argc > 4)
    {
      std::cerr << usage() << std::endl;
      return 1;
    }

  // Input CSV files.
  string idata1csv = argv[1];
  string idata2csv = argv[2];
  string idatatxt;
  if (argc == 4)
    idatatxt = argv[3];
  std::clog << "input files: " << std::endl
	    << idata1csv << std::endl
	    << idata2csv << std::endl
	    << idatatxt << std::endl;

  // Create svg canvas.
  const string fstem1 = file_path_to_stem(idata1csv);
  const string fstem2 = file_path_to_stem(idata2csv);
  const string fstem(fstem1 + "-x-" + "browsertime");
  svg_element obj = initialize_svg(fstem);

  // Deserialize CSV files.
  value_type maxv1(0);
  id_value_umap iv1 = deserialize_csv_to_id_value_map(idata1csv, maxv1);

  value_type maxv2(0);
  id_value_umap iv2 = deserialize_csv_to_id_value_map(idata2csv, maxv2);

  // Find max value of all inputs.
  const int value_max(std::max(maxv1, maxv2));
  typography typo = make_typography_id();
  const point_2t origin = obj.center_point();

  // Draw radial rings on canvas  from inner to outter ripple.
  // Size is inverse of denomenator argument below.

  // If a highlight input file exists, split the first
  // id_value_umap object into a found matches object and a remaining object.
  // Otherwise, just use the first id_value_umap as-is.
  if (argc == 3)
    {
      // 1. Telemetry baseline ripple
      radiate_ids_per_uvalue_on_arc(obj, origin, typo, iv1, value_max, 10, 1);
      //kusama_ids_per_uvalue_on_arc(obj, origin, typof, iv1, value_max, 10, 5, false);
    }
  else
    {
      // 1. Telemetry baseline ripple.
      const strings hilights = deserialize_text_to_strings(idatatxt);
      std::clog << iv1.size() << " original map size" << std::endl;
      id_value_umap iv1hi = remove_matches_id_value_map(iv1, hilights);
      std::clog << iv1hi.size() << " found matches map size" << std::endl;
      std::clog << iv1.size() << " edited original map size" << std::endl;
      radiate_ids_per_uvalue_on_arc(obj, origin, typo, iv1, value_max, 10, 2);

      // 2. Moz Telemetry highlight blue ripple, same size as first
      if (!iv1hi.empty())
	{
	  typography typohi = typo;
	  typohi._M_size = 20;
	  typohi._M_style._M_fill_color = colore::ruriiro;
	  radiate_ids_per_uvalue_on_arc(obj, origin, typohi, iv1hi,
					value_max, 10, 2);
	}
    }

  // 3. Browsertime performance timings orange ripple, next bigger size
  typography typoc = typo;
  typoc._M_style._M_fill_color = colore::asamaorange;
  radiate_ids_per_uvalue_on_arc(obj, origin, typoc, iv2, value_max, 3, 2);
  //kusama_ids_per_uvalue_on_arc(obj, origin, typoc, iv2, value_max, 3, 5, false);

  // Add metadata.
  // Depending on the JSON schema, an environment file may not be generated
  // AKA, mozilla_android.
  environment env;
  try
    {
      environment env1 = deserialize_environment(idata1csv);
      environment env2 = deserialize_environment(idata2csv);
      env = coalesce_environments(env1, env2);
    }
  catch (const std::runtime_error& e)
    {
      env = deserialize_environment(idata2csv);
    }

  render_metadata_environment(obj, env);

  auto y = obj._M_area._M_height / 2;
  auto x = moz::k::margin;
  string s1 = "Mozilla Telemetry";
  string s2 = "Sitespeed.io Browsertime";
  render_metadata_title(obj, maxv1, s1, colore::ruriiro, x, y - 50);
  render_metadata_title(obj, maxv2, s2, colore::asamaorange, x, y + 50);

  render_input_files_title(obj, idata1csv, idata2csv,idatatxt);

  return 0;
}
