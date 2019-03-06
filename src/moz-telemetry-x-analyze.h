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

#ifndef moz_TELEMETRY_X_ANALYZE_H
#define moz_TELEMETRY_X_ANALYZE_H 1

#include <cmath>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "moz-telemetry-x-svg.h"


namespace moz {

double
normalize_on_range(uint value, uint min, uint max, uint nfloor, uint nceil)
{
  auto weightn_numer = ((nceil - nfloor) * (value - min));
  auto weightn_denom = (max - min) + nfloor;
  auto weightn = std::round(weightn_numer / weightn_denom);
  return weightn;
}


// Given rdenom scaling factor and SVG canvas, compute effective radius value.
inline double
get_radius(const svg_form& obj, const uint rdenom)
{ return std::min(obj._M_area._M_height, obj._M_area._M_width) / rdenom; }


inline double
get_angle(int pvalue, int pmax)
{
  // Max number of non-overlapping degrees in circle, such that the
  // beginning and the end have a discernable gap. Total degrees in a
  // circle are 360, but then the beginning and the end of the radial
  // display are in the same place.
  const double maxdeg = 345;

  // Normalize [0, pmax] to range [0, maxdeg] and put pvalue in it.
  double angled = normalize_on_range(pvalue, 0, pmax, 0, maxdeg);

  // Change rotation to CW instead of CCW (or anti-clockwise).
  angled = 360 - angled;

  // Rotate 90 CCW, so that the first element will be at the top
  // vertical axis, instead of the right middle axis.
  angled += 90;

  return angled;
}


point
get_circumference_point(double angler, double r, const point origin)
{
  auto [ cx, cy ] = origin;
  //  const double cy = obj._M_area._M_height / 2;
  double x(cx + (r * std::cos(angler)));
  double y(cy - (r * std::sin(angler)));

  return std::make_tuple(x, y);
}


void
splay_ids(svg_form& obj, const typography& typo, const strings ids,
	  const double angled, const point origin, double rr, double rspace)
{
  const double maxdeg = rspace * (ids.size() - 1);
  double angled2 = angled - maxdeg / 2;
  for (const string& s: ids)
    {
      double angler2 = (k::pi / 180.0) * angled2;
      auto [ x2, y2 ] = get_circumference_point(angler2, rr, origin);
      place_text_id(obj, typo, s, x2, y2, angled2);
      angled2 += rspace;
    }
}


// Map ids with one value to a point cluster radiating out from a center.
void
radiate_ids_by_uvalue(svg_form& obj, const typography& typo, const strings ids,
		      int pvalue, int pmax, double r, double rspace)
{
  auto& area = obj._M_area;
  const point origin = std::make_tuple(area._M_width / 2, area._M_height / 2);

  // Find point on the circumference of the circle closest to value (pvalue).
  const double angled = get_angle(pvalue, pmax);
  double angler = (k::pi / 180.0) * angled;
  auto [ x, y ] = get_circumference_point(angler, r, origin);

  // Consolidate label text to be "VALUE -> "
  constexpr uint valuewidth(9);
  std::ostringstream oss;
  oss.imbue(std::locale(""));
  oss << std::setfill(' ') << std::setw(valuewidth) << std::left << pvalue;
  string label = oss.str() + " -> ";
  place_text_id(obj, typo, label, x, y, angled);

  // Next, print out the various id's on an arc with a bigger radius.
#if 1
  splay_ids(obj, typo, ids, angled, origin, r + 65, rspace);
#else
  auto imiddle = ids.begin() + (ids.size() / 2);
  strings ids1(ids.begin(), imiddle);
  strings ids2(imiddle, ids.end());
  splay_ids(obj, typo, ids1, angled, origin, r + 65, rspace);
  splay_ids(obj, typo, ids2, angled, origin, r + 80, rspace);
#endif
}


/*
  Draw text on the circumference of a circle of radius r centered at (cx, cy)
  corresponding to the angle above.
*/
void
radiate_id_by_value(svg_form& obj, const typography& typo, string pname,
		    int pvalue, int pmax, double r, bool rotatep)
{
  const double angled = get_angle(pvalue, pmax);
  double angler = (k::pi / 180.0) * angled;
  auto& area = obj._M_area;
  const point origin = std::make_tuple(area._M_width / 2, area._M_height / 2);
  auto [ x, y ] = get_circumference_point(angler, r, origin);

  // Consolidate label text to be "VALUE -> NAME"
  constexpr uint valuewidth(9);
  std::ostringstream oss;
  oss.imbue(std::locale(""));
  oss << std::setfill(' ') << std::setw(valuewidth) << std::left << pvalue;
  string label = oss.str() + " -> " + pname;

  if (rotatep)
    place_text_id(obj, typo, label, x, y, angled);
  else
    place_text_id(obj, typo, label, x, y, 0);
}


/*
  Create radial viz of names from input file arranged clockwise around
  the edge of a circle circumference. The text of the names can be
  rotated, or not.

 Arguments are:

 ifile == CSV file of extracted marker/probe names to display.

 rdenom == scaling factor for radius of circle used for display, where
  larger values (used as a denominator) make smaller (tighter) circles.

 rotatep == rotate name text to be on an arc from the origin of the circle.

*/
svg_form
radiate_ids_per_value_on_arc(svg_form& obj, const typography& typo,
			     const id_value_umap& ivm,
			     const int value_max, const int rdenom,
			     bool rotatep = true)
{
  // Probe/Marker display.
  // Loop through map key/values and put on canvas.
  const double r = get_radius(obj, rdenom);
  for (const auto& v : ivm)
    {
      string pname(v.first);
      int pvalue(v.second);
      if (pvalue)
	radiate_id_by_value(obj, typo, pname, pvalue, value_max, r, rotatep);
    }

  return obj;
}


// Radiate as above, but group similar values such that they are
// splayed, and not written on top of each other on the same
// arc/angle.
svg_form
radiate_ids_per_uvalue_on_arc(svg_form& obj, const typography& typo,
			      const id_value_umap& ivm,
			      const int value_max,
			      const int rdenom, const int rspace)
{
  const double r = get_radius(obj, rdenom);

  // Convert from string id-keys to int value-keys, plus an ordered set of all
  // the unique values.
  uvalue_set uvalues;
  value_id_ummap uvaluemm = to_value_id_mmap(ivm, uvalues);

  int last = 0;
  for (const auto& v : uvalues)
    {
      auto count = uvaluemm.count(v);
      if (count == 1)
	{
	  auto i = uvaluemm.find(v);
	  radiate_id_by_value(obj, typo, i->second, v, value_max, r, true);
	}
      else
	{
	  // Extract all the ids for a given value.
	  auto irange = uvaluemm.equal_range(v);
	  auto ibegin = irange.first;
	  auto iend = irange.second;
	  strings ids;
	  for (auto i = ibegin; i != iend; ++i)
	    ids.push_back(i->second);

	  if (v)
	    radiate_ids_by_uvalue(obj, typo, ids, v, value_max, r, rspace);
	}

      // Set last value to the current value.
      last = v;
    }

  return obj;
}

} // namespace moz

#endif
