// support for radial, sunburst / RAIL forms -*- mode: C++ -*-

// Copyright (c) 2020-2021, Mozilla
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
#include "a60-svg-radial-arc.h"


namespace moz {


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
      mss << k::errorprefix << "deserialize_csv_to_id_value_map:: "
	  << "cannot open input file: "
	  << ifile << std::endl;
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
init_id_render_state_cache(const double opacity, const string hilite)
{
  using svg::k::select;
  const select dviz = select::glyph | select::vector;

  // Default.
  style dstyl = { color::black, opacity, color::white, opacity, 3 };
  add_to_id_render_state_cache("", dstyl, dviz);

  // Metric cosmologies.
  style webvitalstyl = { color::asamablue, 1.0, color::asamablue, 0, 3 };
  add_to_id_render_state_cache(k::webvitals, webvitalstyl, dviz);

  style telemetrystyl = { color::asamaorange, 1.0, color::asamaorange, 0, 3 };
  add_to_id_render_state_cache(k::telemetry, telemetrystyl, dviz);

  style vizmetricstyl = { color::ruriiro, 1.0, color::ruriiro, 0, 3 };
  add_to_id_render_state_cache(k::visualmetrics, vizmetricstyl, dviz);

  // Highlight style.
  if (!hilite.empty())
    {
      style histyl = { color::red, 1.0, color::red, 0, 3 };
      add_to_id_render_state_cache(hilite, histyl, dviz);
    }

  // Explicitly-styled metrics.
  add_to_id_render_state_cache("TTFB", webvitalstyl, dviz);
  add_to_id_render_state_cache("firstPaint", webvitalstyl, dviz);
  add_to_id_render_state_cache("FCP", webvitalstyl, dviz);
  add_to_id_render_state_cache("LCP", webvitalstyl, dviz);
}


/**
   Render metrics in an arc centered at origin, starting at 0 degrees
   north and continuing around clockwise, according to metrics and
   values in csv file.

   hilite	== metric to highlight

   vmax		== maximum value corresponding with end of arc, if not the
		   maximum value from the csv file. (ie doing relational arcs).

   radius       == radius of arc
   rspace       == space between arc end and label text begin

   contextp	== add in metadata about context if true, otherwise just do arc.

   Returns the time of the highlight metric or vmax.
 */
value_type
render_radial(svg_element& obj, const point_2t origin, const string idatacsv,
	      const string imetrictype, const string hilite,
	      const value_type vmax = 0,
	      const int radius = 80, const int rspace = 24)
{
  // Get id map and outcomes.
  // Iif in nanoseconds scale to milliseconds
  // Iif vmax non-zero, scale rendered radials to vmax.
  value_type ts = 1;

  // Glean is in nanoseconds, assumes glean-generated files include "glean".
  if (imetrictype.find("glean") != string::npos)
    ts = 1000000;


  value_type value_max(0);
  id_value_umap iv = deserialize_csv_to_id_value_map(idatacsv, value_max, ts);
  if (vmax != 0)
    value_max = vmax;

  // Render radial elements.
  typography typo = make_typography_id();
  point_2t& rrange = get_radial_range();
  rrange = { 0, 270 };

#if 0
  radiate_ids_per_uvalue_on_arc(obj, origin, typo, iv, value_max,
				radius, rspace);
#else
  // Match imetrictype to style.
  id_render_state rst = get_id_render_state(imetrictype);

  // Direction glyph.
  svg::style stylinset(rst.styl);
  stylinset._M_fill_opacity = 0;
  stylinset._M_stroke_opacity = 1;
  stylinset._M_stroke_size = 3;
  direction_arc_at(obj, origin, radius, stylinset);
  direction_arc_title_at(obj, origin, radius, rst.styl, k::webvitals);

  // bool values: weigh-by-value, collision-avoidance
  kusama_ids_per_uvalue_on_arc(obj, origin, typo, iv, value_max,
			       radius, rspace, false, false);
#endif

  value_type timev = iv.count(hilite) ? iv[hilite] : value_max;
  return timev;
}

} // namespace moz

#endif
