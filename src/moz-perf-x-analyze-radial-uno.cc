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

const string hilite("rumSpeedIndex");

void
init_id_render_state_cache(double opacity = 0.33)
{
  const svg::k::select dviz = svg::k::select::glyph | svg::k::select::vector;

  // Default.
  style dstyl = { colore::black, opacity, colore::white, opacity, 3 };
  add_to_id_render_state_cache("", dstyl, dviz);

  // Pull out one it to highlight, say rumSpeedIndex.
  style histyl = { colore::red, 1.0, colore::red, 0, 3 };
  add_to_id_render_state_cache(hilite, histyl, dviz);
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
  init_id_render_state_cache();
  set_label_spaces(6);

  const string fstem = file_path_to_stem(idatacsv);
  svg_element obj = initialize_svg(fstem);
  typography typo = make_typography_id();

  // Deserialize CSV files and find max value.
  value_type value_max(0);
  const point_2t origin = obj.center_point();
  auto [ x, y ] = origin;

  // Get id map, if in nanoseconds scale to milliseconds
  value_type ts = 1;
#if 0
  // nanoseconds
  // Glean
  ts = 1000000;
#endif
  id_value_umap iv = deserialize_csv_to_id_value_map(idatacsv, value_max, ts);

  // Render
#if 0
  radiate_ids_per_uvalue_on_arc(obj, origin, typo, iv, value_max, 60, 10);
#else
  // weigh-by-value, collision-avoidance, insert-arrow
  kusama_ids_per_uvalue_on_arc(obj, origin, typo, iv, value_max, 80, 24,
			       false, false, true);
#endif

  // Add metadata.
  environment env = deserialize_environment(idatacsv);
  render_metadata_environment(obj, env);

  value_type timev;
  if (iv.count(hilite))
    timev = iv[hilite];
  else
    timev = value_max;

  auto yprime = obj._M_area._M_height - moz::k::margin;
  render_metadata_time(obj, timev, colore::red, x, yprime);

  value_type tsz = 14;
  typography typot = make_typography_metadata(tsz, true);
  string browserua = env.sw_name;
  if (browserua.empty())
    browserua = "Chrome";
  place_text_at_point(obj, typot, env.sw_name, x, yprime + (2 * tsz));

  return 0;
}
