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

#include "moz-json-basic.h"
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
  const string fstem(fstem1 + "-X-" + fstem2);
  svg_form obj = initialize_svg(fstem);

  // Deserialize CSV files.
  int maxv1(0);
  id_value_umap iv1 = deserialize_id_value_map(idata1csv, maxv1);

  int maxv2(0);
  id_value_umap iv2 = deserialize_id_value_map(idata2csv, maxv2);

  // Find max value of all inputs.
  const int value_max(std::max(maxv1, maxv2));
  typography typo = make_typography_id();

  // Draw radial rings on canvas  from inner to outter ripple.
  // Size is inverse of denomenator argument below.

  // 1. Firefox inner, blue (2 spacer)
  typography typof = typo;
  typof._M_style._M_fill_color = colore::red;
  radiate_ids_per_uvalue_on_arc(obj, typof, iv1, value_max, 10, 2);


  // 2. Chrome outer, red (1 spacer)
  typography typoc = typo;
  typoc._M_style._M_fill_color = colore::blue;
  radiate_ids_per_uvalue_on_arc(obj, typoc, iv2, value_max, 3, 1);

  // Add metadata.
  environment env1 = deserialize_environment(idata1csv);
  environment env2 = deserialize_environment(idata2csv);
  render_metadata_environment(obj, env1, env2);

  auto y = obj._M_area._M_height / 2;

  render_metadata_title(obj, maxv1, fstem1, colore::red, y - 50);
  render_metadata_title(obj, maxv2, fstem2, colore::blue, y + 50);

  return 0;
}
