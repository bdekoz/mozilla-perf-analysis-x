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
  double x(cx + (r * std::cos(angler)));
  double y(cy - (r * std::sin(angler)));
  return std::make_tuple(x, y);
}


// RADIAL 1
// Radiate clockwise from 0 to 35x degrees about origin, placing each
// id at a point on the circumference. Duplicate points overlap.

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
			     const id_value_umap& ivm, const int value_max,
			     const int rdenom, bool rotatep = true)
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


// RADIAL 2
// Radiate clockwise from 0 to 35x degrees about origin, placing each
// id at a point on the circumference. Duplicate ids splay, stack,
// or append/concatencate at, after, or around that point.

void
sort_ids_by_size(strings& ids)
{
  auto lsizeless = [](const string& s1, const string& s2)
		   { return s1.size() < s2.size(); };
  sort(ids.begin(), ids.end(), lsizeless);
}


// Spread ids on either side of an origin point, along circumference path.
void
splay_ids_around(svg_form& obj, const typography& typo, const strings& ids,
		 const double angled, const point origin, double r,
		 double rspace)
{
  const double maxdeg = rspace * (ids.size() - 1);
  double angled2 = angled - maxdeg / 2;
  for (const string& s: ids)
    {
      double angler2 = (k::pi / 180.0) * angled2;
      auto [ x2, y2 ] = get_circumference_point(angler2, r, origin);
      place_text_id(obj, typo, s, x2, y2, angled2);
      angled2 += rspace;
    }
}


// Spread ids past the origin point, along circumference path.
void
splay_ids_after(svg_form& obj, const typography& typo, const strings& ids,
		const double angled, const point origin, double r,
		double rspace)
{
  double angledt(angled);
  for (const string& s: ids)
    {
      double angler = (k::pi / 180.0) * angledt;
      auto [ x, y ] = get_circumference_point(angler, r, origin);
      place_text_id(obj, typo, s, x, y, angledt);
      angledt -= rspace;
    }
}


void
splay_ids_stagger(svg_form& obj, const typography& typo, const strings& ids,
		  const double angled, const point origin, double r,
		  double rspace)
{
  if (ids.size() > 1)
    {
      auto imiddle = ids.begin() + (ids.size() / 2);
      strings ids1(ids.begin(), imiddle);
      strings ids2(imiddle, ids.end());
      splay_ids_after(obj, typo, ids1, angled, origin, r, rspace);
      splay_ids_after(obj, typo, ids2, angled, origin, r + 125, rspace);
    }
  else
    splay_ids_after(obj, typo, ids, angled, origin, r, rspace);
}


// Rotate and stack ids at origin point, extending radius for each
// from point of origin.
void
stack_ids_at(svg_form& obj, const typography& typoo, const strings& ids,
	     const double angled, const point origin, double r,
	     const double rincrement = 10)
{
  // Rotate 90 CW around origin, and spread out .
  typography typo(typoo);
  typo._M_a = svg::typography::anchor::middle;
  typo._M_align = svg::typography::align::center;

  const double angler = (k::pi / 180.0) * angled;
  for (const string& s: ids)
    {
      auto [ x2, y2 ] = get_circumference_point(angler, r, origin);
      place_text_id(obj, typo, s, x2, y2, angled + 90);
      r += rincrement;
    }
}


void
append_ids_at(svg_form& obj, const typography& typo, const strings& ids,
	      const double angled, const point origin, double r)
{
  double angler = (k::pi / 180.0) * angled;
  auto [ x, y ] = get_circumference_point(angler, r, origin);
  string scat;
  for (const string& s: ids)
    {
      if (!scat.empty())
	scat += ", ";
      scat += s;
    }
  place_text_id(obj, typo, scat, x, y, angled);
}


// Map ids with one value to a point cluster radiating out from a center.
void
radiate_ids_by_uvalue(svg_form& obj, const typography& typo, const strings& ids,
		      int pvalue, int pmax, double r,
		      double rspace [[maybe_unused]])
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
  // splay_ids_around(obj, typo, ids, angled, origin, r + 65, rspace);
  // splay_ids_after(obj, typo, ids, angled, origin, r + 65, rspace);
  // splay_ids_stagger(obj, typo, ids, angled, origin, r + 65, rspace);

  // stack_ids_at(obj, typo, ids, angled, origin, r + 65, 10);

  append_ids_at(obj, typo, ids, angled, origin, r + 65);
}


// Radiate as above, but group similar values such that they are
// splayed, and not written on top of each other on the same
// arc/angle.
svg_form
radiate_ids_per_uvalue_on_arc(svg_form& obj, const typography& typo,
			      const id_value_umap& ivm, const int value_max,
			      const int rdenom, const int rspace)
{
  // Convert from string id-keys to int value-keys, plus an ordered set of all
  // the unique values.
  value_set uvalues;
  value_id_ummap uvaluemm = to_value_id_mmap(ivm, uvalues);

  const double r = get_radius(obj, rdenom);
  for (const auto& v : uvalues)
    {
      auto count = uvaluemm.count(v);
      std::clog << "value, count: " << v << ',' << count << std::endl;

      // Extract all the ids for a given value.
      auto irange = uvaluemm.equal_range(v);
      auto ibegin = irange.first;
      auto iend = irange.second;
      strings ids;
      for (auto i = ibegin; i != iend; ++i)
	ids.push_back(i->second);
      sort_ids_by_size(ids);

      if (v)
	radiate_ids_by_uvalue(obj, typo, ids, v, value_max, r, rspace);
    }

  return obj;
}


// RADIAL 3
// Radiate clockwise from 0 to 35x degrees about origin, placing each
// id at a point cluster on the circumference. A point cluster is a
// circle whos radius is proportionate to the number of duplicate ids
// at that point.  Duplicate ids splay, stack, or
// append/concatencate at, after, or around that point cluster.

double
distance_cartesian(const point& p1, const point& p2)
{
  auto [ x1, y1 ] = p1;
  auto [ x2, y2 ] = p2;
  auto distancex = (x2 - x1) * (x2 - x1);
  auto distancey = (y2 - y1) * (y2 - y1);
  double distance = sqrt(distancex + distancey);
  return distance;
}


// https://developer.mozilla.org x 2D_collision_detection
bool
is_collision_detected(const point& p1, const int r1,
		      const point& p2, const int r2)
{
  bool ret(false);
  if (distance_cartesian(p1, p2) < r1 + r2)
    ret = true;
  return ret;
}


// Rotate and stack ids at origin point, extending radius for each
// from point of origin.
void
kusama_ids_at_point(svg_form& obj, const typography& typo, const strings& ids,
		    const point p, double r)
{
  string scat;
  for (const string& s: ids)
    {
      if (!scat.empty())
	scat += ", ";
      scat += s;
    }
  auto [ x, y] = p;
  point_2d_to_circle(obj, x, y, typo._M_style, r);
}


/**
   Radiate as above *_per_uvalue_on_arc function, but group similar
   values such that they are globbed into a sattelite circle, ids
   splayed around the satellite, and not written on top of each other
   on the same arc/angle.

   Framing circle radius is the result of dividing page dimensions by (rdenom).

   Satellite circle radius is the product of the number of ids with
   the same value times a base multipler (rbase).
*/
svg_form
kusama_ids_per_uvalue_on_arc(svg_form& obj, const typography& typo,
			      const id_value_umap& ivm, const int value_max,
			      const int rdenom, const double rbase = 5)
{
  auto& area = obj._M_area;
  const point origin = std::make_tuple(area._M_width / 2, area._M_height / 2);
  const double r = get_radius(obj, rdenom);

  // Make circle perimiter with an arrow to orientate display of data.
  insert_svg_at_center(obj);

  // Convert from string id-keys to int value-keys, plus an ordered set of all
  // the unique values.
  value_set uvalues;
  value_id_ummap uvaluemm = to_value_id_mmap(ivm, uvalues);

  // Map out preliminary data points.
  std::vector<int> vuvalues(uvalues.begin(), uvalues.end());
  std::vector<strings> vids;
  std::vector<pointn> vpointns;
  for (const auto& v : vuvalues)
    {
      auto count = uvaluemm.count(v);
      std::clog << "value, count: " << v << ',' << count << std::endl;

      // Extract all the ids for a given value.
      auto irange = uvaluemm.equal_range(v);
      auto ibegin = irange.first;
      auto iend = irange.second;
      strings ids;
      for (auto i = ibegin; i != iend; ++i)
	ids.push_back(i->second);
      sort_ids_by_size(ids);
      vids.push_back(ids);

      // Compute satellite circle radius, so that the satellite
      // circles all have an outermost point (not center) that touches
      // the inset svg glyph.
      double rext = r + (rbase * ids.size());

      // Find initial point on the circumference of the circle closest to
      // current value, aka initial circumference point (ICP).
      const double angled = get_angle(v, value_max);
      double angler = (k::pi / 180.0) * angled;
      point p = get_circumference_point(angler, rext, origin);
      vpointns.push_back(std::make_tuple(p, ids.size()));
    }

  // Remove zero value ids.
  if (vuvalues.front() == 0)
    {
      auto& ids = vids[0];
      std::clog << "eliding ids with value zero: " << std::endl;
      for (const auto& s: ids)
	std::clog << s << std::endl;

      vuvalues.erase(vuvalues.begin());
      vids.erase(vids.begin());
      vpointns.erase(vpointns.begin());
    }

  // Threshold is the range such that a value is considered adjacent
  // for collisions. If v > previous value + threshold, then the
  // points are not considered neighbors.
  const int threshold(200);

  // Massage data to fit without overlaps, given:
  // 1 unique set of values
  // 2 for each unique value, the ids that match
  // 3 for each unique value, the proposed point and size of the circle
  for (uint i = 0; i < vpointns.size(); ++i)
    {
      auto& pn = vpointns[i];
      auto& [ p, n ] = pn;
      double rcur = rbase * n;

      // Fixed angle, just push point along ray from origin until no conflicts.
      const double angled = get_angle(vuvalues[i], value_max);
      double angler = (k::pi / 180.0) * angled;

      // Points near p that are under threshold (and lower-indexed in vpointns).
      uint neighbors(0);

      // Find neighbors.
      uint j(i);
      while (i > 0 && j > 0)
	{
	  j -= 1;

	  // Only if within threshold, know values are ordered.
	  if (vuvalues[i] - vuvalues[j] < threshold)
	    ++neighbors;
	  else
	    break;
	}
      std::clog << i << k::tab << "neighbors: " << neighbors << std::endl;

      // Search backward and fixup.... in practice results in overlap with
      // lowest-index or highest-index neighbor.
      //
      // So... search forward and fixup. Not ideal; either this or
      // move to collision detection with multiple neighbor points.
      //
      // If collisions, extend radius outward from origin and try
      // again until the collision is removed and the previous
      // neighbor circles don't overlap.
      double rext = r + rcur;
      for (uint k = neighbors; k > 0; --k)
	{
	  // Get neighbor point, starting with lowest-index neighbor.
	  auto& prevpn = vpointns[i - k];
	  auto& [ prevp, prevn ] = prevpn;
	  double rprev = rbase * prevn;

	  while (is_collision_detected(p, rcur, prevp, rprev))
	    {
	      // Find new point further out from origin.
	      rext += (2.5 * rcur);
	      p = get_circumference_point(angler, rext, origin);
	    }
	}
    }

  // Draw resulting points.
  for (uint i = 0; i < vpointns.size(); ++i)
    {
      auto& pn = vpointns[i];
      auto& [ p, n ] = pn;
      auto r = rbase * n;
      kusama_ids_at_point(obj, typo, vids[i], p, r);
    }

  return obj;
}

} // namespace moz

#endif
