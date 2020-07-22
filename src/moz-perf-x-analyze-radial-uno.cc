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
#include "moz-perf-x-svg.h"


namespace moz {

std::string
usage()
{
  std::string s("usage: moz-telemetry-x-analyze-radial.exe data.csv");
  s += '\n';
  return s;
}


strings
init_id_render_state_cache()
{
  strings special;
  id_render_state_umap& cache = get_id_render_state_cache();

  // Default.
  auto opacity = 0.33;
  id_render_state default_state(svg::k::b_style);
  default_state.styl._M_fill_opacity = opacity;
  default_state.styl._M_stroke_opacity = opacity;
  default_state.set(default_state.visible_mode, svg::k::select::glyph);
  cache.insert(std::make_pair("", default_state));
  special.push_back("");

  return special;
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

  // Input CSV file.
  std::string idatacsv = argv[1];
  std::clog << "input files: " << idatacsv << std::endl;

 // Create svg canvas.
  const string fstem = file_path_to_stem(idatacsv);
  svg_element obj = initialize_svg(fstem);
  init_id_render_state_cache();

  // Deserialize CSV files and find max value.
  value_type value_max(0);
  typography typo = make_typography_id();
  const point_2t origin = obj.center_point();

  // Get id map, if in nanoseconds scale to milliseconds
#if 1
  id_value_umap iv = deserialize_csv_to_id_value_map(idatacsv, value_max);
#else
  // nanoseconds
  // Glean
  id_value_umap iv = deserialize_csv_to_id_value_map(idatacsv, value_max,
						     1000000);
#endif

  //radiate_ids_per_uvalue_on_arc(obj, origin, typo, iv, value_max, 60, 10);

  // weigh-by-value, collision-avoidance, insert-arrow
  kusama_ids_per_uvalue_on_arc(obj, origin, typo, iv, value_max, 80, 22,
			       false, false, true);

  // Add metadata.
  environment env = deserialize_environment(idatacsv);
  render_metadata_environment(obj, env);
  render_metadata_title(obj, value_max, fstem);

  return 0;
}
