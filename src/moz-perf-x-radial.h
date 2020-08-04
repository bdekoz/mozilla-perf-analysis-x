// support for radial, sunburst / RAIL forms -*- mode: C++ -*-

// Copyright (c) 2020, Mozilla
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

#ifndef moz_X_RADIAL_H
#define moz_X_RADIAL_H 1

#include "moz-perf-x-svg.h"
#include "moz-perf-x-json.h"


namespace moz {


// Convert from input file name to an in-memory vector of strings
// representing identifiers/names to match against field names in a
// JSON file.
strings
deserialize_text_to_strings(string inames)
{
  strings probes;
  std::ifstream ifs(inames);
  if (ifs.good())
    {
      string line;
      do
	{
	  std::getline(ifs, line);
	  if (ifs.good())
	    probes.push_back(line);
	}
      while (!ifs.eof());
      std::sort(probes.begin(), probes.end());

      std::clog << probes.size() << " match names found in: " << std::endl;
      std::clog << inames << std::endl;
      std::clog << std::endl;
    }
  else
    {
      std::cerr << k::errorprefix
		<< "error: cannot open input file " << inames << std::endl;
    }

  return probes;
}

// Read CSV file of [marker name || probe name] and value, and
// store in hash_map, return this plus the max value as a tuple.
// ifile == input csv file
// value_max == maximum value of all inputs
// scale == default 1, otherwise conversion factor so that value/scale
id_value_umap
deserialize_id_value_map(istream& istr, value_type& value_max,
			 value_type scale = 1)
{
  id_value_umap probe_map;
  do
    {
      string pname;
      getline(istr, pname, ',');
      if (istr.good())
	{
	  value_type pvalue(0);
	  istr >> pvalue;

	  if (pvalue != 0 && scale != 1)
	    {
	      // Then scale by given...
	      pvalue /= scale;
	    }

	  // Extract remaining newline.
	  istr.ignore(79, k::newline);

	  probe_map.insert(make_pair(pname, pvalue));
	  value_max = std::max(pvalue, value_max);
	}
    }
  while (istr.good());
  return probe_map;
}


id_value_umap
deserialize_csv_to_id_value_map(const string& ifile, value_type& value_max,
				value_type scale = 1)
{
  std::ifstream ifs(ifile);
  if (!ifs.good())
    {
      ostringstream mss;
      mss << k::errorprefix << "cannot open input file " << ifile << std::endl;
      throw std::runtime_error(mss.str());
    }
  else
    return deserialize_id_value_map(ifs, value_max, scale);
}


value_type
largest_value_in(const string f1, const string f2 = "")
{
  // Find max value in input files...
  value_type maxv1(0);
  if (!f1.empty())
    id_value_umap iv1 = deserialize_csv_to_id_value_map(f1, maxv1);

  value_type maxv2(0);
  if (!f2.empty())
    id_value_umap iv2 = deserialize_csv_to_id_value_map(f2, maxv2);
  return std::max(maxv1, maxv2);
}


void
init_id_render_state_cache(double opacity = 0.33,
			   const string hilite = "rumSpeedIndex")
{
  const svg::k::select dviz = svg::k::select::glyph | svg::k::select::vector;

  // Default.
  style dstyl = { colore::black, opacity, colore::white, opacity, 3 };
  add_to_id_render_state_cache("", dstyl, dviz);

  // Pull out one it to highlight, say rumSpeedIndex.
  style histyl = { colore::red, 1.0, colore::red, 0, 3 };
  add_to_id_render_state_cache(hilite, histyl, dviz);

  // Colors: colore::ruriiro, colore::asamaorange.
  style webvitalsstyl = { colore::asamablue, 1.0, colore::asamablue, 0, 3 };
  add_to_id_render_state_cache("TTFB", webvitalsstyl, dviz);
  add_to_id_render_state_cache("firstPaint", webvitalsstyl, dviz);
  add_to_id_render_state_cache("FCP", webvitalsstyl, dviz);
  add_to_id_render_state_cache("LCP", webvitalsstyl, dviz);
}


svg_element
render_radial(svg_element& obj, const point_2t origin, const string idatacsv,
	      const string hilite = "rumSpeedIndex", const value_type vmax = 0,
	      const int radius = 80, const int rspace = 24,
	      const bool rendercontextp = true)
{
  typography typo = make_typography_id();

  auto [ x, y ] = origin;

  // Get id map and outcomes.
  // Iif in nanoseconds scale to milliseconds
  // Iif vmax non-zero, scale rendered radials to vmax.
  value_type ts = 1;
#if 0
  // nanoseconds
  // Glean
  ts = 1000000;
#endif

  value_type value_max(0);
  id_value_umap iv = deserialize_csv_to_id_value_map(idatacsv, value_max, ts);
  if (vmax != 0)
    value_max = vmax;

  // Render
#if 0
  radiate_ids_per_uvalue_on_arc(obj, origin, typo, iv, value_max,
				radius, rspace);
#else
  // weigh-by-value, collision-avoidance, insert-arrow
  kusama_ids_per_uvalue_on_arc(obj, origin, typo, iv, value_max,
			       radius, rspace, false, false, true);
#endif

  // Render titles, times, or other context.
  // NB: Assumes an environment data file exists, which may not be the case...
  if (rendercontextp)
    {
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
      place_text_at_point(obj, typot, browserua, x, yprime + (2 * tsz));
    }

  return obj;
}

} // namespace moz

#endif
