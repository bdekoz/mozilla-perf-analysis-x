// telemetry radial, sunburst / RAIL forms -*- mode: C++ -*-

// Copyright (c) 2018-2021, Mozilla
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
  std::string s("usage: moz-perf-x-analyze-radial-uno.exe data.csv "
		"metric-cosmology (metric-key-to-compare-or-highlight)");
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

  // Input is CSV file.
  std::string idata = argv[1];
  std::string imetrictype = argv[2];
  clog << "input files: " << idata << endl;
  clog << "metric type: " << imetrictype << endl;

  string hilite = "VisualComplete95";
  if (argc == 4)
    hilite = argv[3];
  clog << "key metric: " << hilite << endl;

  // Create svg canvas.
  init_id_render_state_cache(0.33, hilite);
  set_label_spaces(6);
  const string fstem = file_path_to_stem(idata);
  svg_element obj = initialize_svg(fstem);
  const point_2t origin = obj.center_point();
  value_type timev = render_radial(obj, origin, idata, imetrictype, hilite);

  // Add metadata.
  environment env = deserialize_environment(idata);
  render_metadata(obj, env);

  // Render metadata titles, times, or context.
  auto x = obj._M_area._M_width / 2;
  auto y = obj._M_area._M_height - moz::k::margin;
  render_metadata_time(obj, timev, color::red, x, y);

  value_type tsz = 18;
  typography typot = make_typography_metadata(tsz, true, color::red);
  place_text_at_point(obj, typot, hilite, x, y + (2 * tsz));

  return 0;
}
