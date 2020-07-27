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
  std::string s("usage: moz-telemetry-x-analyze-radial-duo-side-by-side.exe "
		"data1.csv data2.csv");
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


svg_element
render_radial(svg_element& obj, const string idatacsv, const point_2t origin)
{
  value_type value_max(0);
  typography typo = make_typography_id();

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

  value_type timev;
  if (iv.count(hilite))
    timev = iv[hilite];
  else
    timev = value_max;

  auto yprime = obj._M_area._M_height - moz::k::margin;
  render_metadata_time(obj, timev, colore::red, x, yprime);

  environment env = deserialize_environment(idatacsv);
  value_type tsz = 14;
  typography typot = make_typography_metadata(tsz, true);
  string browserua = env.sw_name;
  if (browserua.empty())
    browserua = "Chrome";
  place_text_at_point(obj, typot, env.sw_name, x, yprime + (2 * tsz));

  return obj;
}

} // namespace moz


int main(int argc, char* argv[])
{
   using namespace moz;

   // Sanity check.
  if (argc != 3)
    {
      std::cerr << usage() << std::endl;
      return 1;
    }

  // Input CSV dir with same number of csv files in each.
  std::string idata1 = argv[1];
  std::string idata2 = argv[2];
  std::clog << "input directories: " << std::endl
	    << idata1 << std::endl
	    << idata2 << std::endl;

  
  // Create svg canvas.
  init_id_render_state_cache();
  set_label_spaces(6);

  // For each unique TLD/site in directories, use CSV file to do...
  for (uint i = 0; i < sz; ++i)
    {
      const string fstem = file_path_to_stem(idata1);
      svg_element obj = initialize_svg(fstem);
      auto xdelta = obj._M_area._M_width / 4;
      
      auto [ x, y ] = obj.center_point();
      auto x1 = x - xdelta;
      auto x2 = x + xdelta;
      render_radial(obj, idata1, point_2t(x1, y));
      render_radial(obj, idata2, point_2t(x2, y));
      
      // Add metadata.
      environment env = deserialize_environment(idata1);
      render_metadata_environment(obj, env);
    }
  
  return 0;
}
