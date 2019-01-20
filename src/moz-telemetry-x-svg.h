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

#ifndef moz_TELEMETRY_X_SVG_H
#define moz_TELEMETRY_X_SVG_H 1

#include "a60-svg-base.h"

#include "moz-telemetry-x.h"


namespace moz {

using namespace svg;
using color = svg::colore;


// Create an svg object with 1080p dimensions and return it.
svg_form
initialize_svg(const string ofile = "moz-telemetry-radiating-lines",
	       const int width = 1920, const int height = 1080)
{
  area<> a = { unit::pixel, width, height };
  svg_form obj(ofile, a);

  group_form g;
  g.start_element("mozilla viz experiment 20190102.v3");
  g.finish_element();
  obj.add_element(g);

  // Read SVG to insert.
  std::string ifile(k::datapath + "image/circle-arrow-red.svg");
  std::ifstream ifs(ifile);
  string isvg;
  if (ifs.good())
    {
      // Strip out any XML version line in the SVG file.
      // Search for and discard lines with "xml version", iff exists
      string xmlheader;
      getline(ifs, xmlheader);
      if (xmlheader.find("xml version") == string::npos)
	ifs.seekg(0, ifs.beg);

      std::ostringstream oss;
      oss << ifs.rdbuf();
      isvg = oss.str();
    }
  else
    throw std::runtime_error("initialize_svg:: insertion of nested SVG failed");

  // Insert nested SVG element of red arc with arrow (scaled and with offset).
  const int isize(182);
  const int x = width / 2 - isize / 2;
  const int y = height / 2 - isize / 2;

  string ts(transform::translate(x, y));
  group_form gsvg;
  gsvg.start_element("mozilla inset radial svg", transform(), ts);
  gsvg.add_raw(isvg);
  gsvg.finish_element();
  obj.add_element(gsvg);

   return obj;
}


typography
make_typography_id()
{
  // Probe/Marker name/value typographics.
  typography typo = svg::k::ccode_typo;
  typo._M_size = 9;
  typo._M_style = svg::k::b_style;
  typo._M_w = svg::typography::weight::xlight;
  typo._M_align = svg::typography::align::left;
  typo._M_a = svg::typography::anchor::start;
  return typo;
}


typography
make_typography_metadata()
{
  // Metadata typographics.
  typography typom = svg::k::zslab_typo;
  typom._M_align = svg::typography::align::left;
  typom._M_a = svg::typography::anchor::start;
  typom._M_size = 14;
  typom._M_style = svg::k::b_style;
  typom._M_w = svg::typography::weight::medium;
  typom._M_style._M_fill_color = colore::gray50;
  return typom;
}


void
place_text_at_point(svg_form& obj, const typography& typo, const string mtext,
		    const int tx, const int ty)
{
  text_form::data dt = { tx, ty, mtext, typo };
  text_form t;
  t.start_element();
  t.add_data(dt);
  t.finish_element();
  obj.add_element(t);
}


void
place_text_metadata(svg_form& obj, const typography& typo, string mtext)
{
  int tx = k::margin;
  static int ty = k::margin;

  if (!mtext.empty())
    {
      place_text_at_point(obj, typo, mtext, tx, ty);

      // Increment vertical, assume higher moves text down the page.
      ty += typo._M_size;
    }
}


void
place_text_id(svg_form& obj, const typography& typo, string label,
	      int tx, int ty, const double deg = 0.0)
{
  text_form::data dt = { tx, ty, label, typo };
  text_form t;
  t.start_element();

  // IFF degrees, then rotate text.
  if (deg > 0)
    t.add_data(dt, svg::transform::rotate(360 - deg, tx, ty));
  else
    t.add_data(dt);

  t.finish_element();
  obj.add_element(t);
}


void
place_metadata(svg_form& obj, const typography& typo, const environment& env)
{
  // place_text_metadata(obj, typo, env.os_vendor);
  place_text_metadata(obj, typo, env.os_name);
  place_text_metadata(obj, typo, env.os_version);
  place_text_metadata(obj, typo, env.os_locale);

  place_text_metadata(obj, typo, " ");

  typography typolarge = typo;
  typolarge._M_size = 20;
  place_text_metadata(obj, typolarge, to_string(env.hw_cpu) + " cores");
  int memi = std::round(env.hw_mem * .001);
  place_text_metadata(obj, typolarge, to_string(memi) + " GB");

  place_text_metadata(obj, typo, " ");

  place_text_metadata(obj, typo, env.sw_name);
  place_text_metadata(obj, typo, env.sw_arch);
  place_text_metadata(obj, typo, env.sw_version);
  place_text_metadata(obj, typo, env.sw_build_id);

  place_text_metadata(obj, typo, " ");

  place_text_metadata(obj, typo, to_string(env.uri_count) + " uri count");
  place_text_metadata(obj, typo, env.url);
  place_text_metadata(obj, typo, env.date_time_stamp);
}


// Metadata display.
void
render_metadata_environment(svg_form& obj, const environment& env)
{
  typography typom = make_typography_metadata();
  place_metadata(obj, typom, env);
}


void
render_metadata_title(svg_form& obj, const int time_max, const string fstem1,
		      const string fstem2 = "", const string hilights = "")
{
  // Vertical offset.
  const typography typom = make_typography_metadata();
  int y = obj._M_area._M_height - k::margin;
  int ydelta  = typom._M_size * 1.5;

  // Input data files.
  place_text_at_point(obj, typom, fstem1, k::margin, y);
  y -= ydelta;

  if (!fstem2.empty())
    {
      place_text_at_point(obj, typom, fstem2, k::margin, y);
      y -= ydelta;
    }

  if (!hilights.empty())
    {
      place_text_at_point(obj, typom, hilights, k::margin, y);
      y -= ydelta;
    }

  // Total time.
  typography typot(typom);
  typot._M_size = 48;
  typot._M_style._M_fill_color = colore::red;
  std::ostringstream oss;
  oss.imbue(std::locale(""));
  oss << time_max << " ms";
  place_text_at_point(obj, typot, oss.str(), k::margin,
		      obj._M_area._M_height / 2);
}

} // namespace moz

#endif
