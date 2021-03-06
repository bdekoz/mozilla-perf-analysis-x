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

#ifndef moz_X_SVG_H
#define moz_X_SVG_H 1

#include "a60-svg.h"
#include "a60-svg-radial.h"
#include "moz-perf-x.h"


namespace moz {


using namespace svg;


// Create an svg object with 1080p dimensions and return it.
svg_element
initialize_svg(const string ofile = "moz-telemetry-radiating-lines",
	       const int width = 1920, const int height = 1080)
{
  area<> a = { width, height };
  svg_element obj(ofile, a);

  group_element g;
  g.start_element("mozilla viz experiment 20200724.v6");
  g.finish_element();
  obj.add_element(g);

  return obj;
}


typography
make_typography_id()
{
  // Probe/Marker name typographics.
  typography typo = svg::k::ccode_typo;
  typo._M_size = 9;
  typo._M_style = svg::k::b_style;
  typo._M_w = svg::typography::weight::xlight;
  typo._M_align = svg::typography::align::left;
  typo._M_anchor = svg::typography::anchor::start;
  return typo;
}


typography
make_typography_values()
{
  // Probe/Marker value typographics.
  typography typo = svg::k::ccode_typo;
  typo._M_size = 12;
  typo._M_style = svg::k::b_style;
  typo._M_w = svg::typography::weight::medium;
  typo._M_align = svg::typography::align::right;
  typo._M_anchor = svg::typography::anchor::end;
  return typo;
}


typography
make_typography_metadata(const size_type fs = 14, const bool centerp = false,
			 const color klr = color::gray50)
{
  // Metadata typographics.
  typography typom = svg::k::zslab_typo;
  typom._M_w = svg::typography::weight::medium;
  typom._M_size = fs;
  typom._M_style = svg::k::b_style;
  typom._M_style._M_fill_color = klr;

  if (centerp)
    {
      typom._M_align = typography::align::center;
      typom._M_anchor = typography::anchor::middle;
    }
  else
    {
      typom._M_align = svg::typography::align::left;
      typom._M_anchor = svg::typography::anchor::start;
    }

  return typom;
}


void
place_text_at_point(svg_element& obj, const typography& typo,
		    const string mtext, const int tx, const int ty)
{
  text_element::data dt = { tx, ty, mtext, typo };
  text_element t;
  t.start_element();
  t.add_data(dt);
  t.finish_element();
  obj.add_element(t);
}


void
place_text_id(svg_element& obj, const typography& typo, string label,
	      int tx, int ty, const double deg = 0.0)
{
  text_element::data dt = { tx, ty, label, typo };
  text_element t;
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
render_metadata(svg_element& obj, const environment& env,
		const bool centerp = false, const bool swmetadatap = false)
{
  int tx = centerp ? obj._M_area._M_width / 2 : k::margin;
  int ty = obj._M_area._M_height - k::margin;

  typography typom = make_typography_metadata(12, centerp);
  typography typolarge = typom;
  typolarge._M_size = 16; // time is 48

  typography typohi = typolarge;
  typohi._M_size = 24; // time is 48
  typohi._M_style._M_fill_color = color::black;

  // Browser
  string browserua = env.sw_name;
  if (browserua.empty())
    browserua = "Chrome";
  place_text_at_point(obj, typohi, browserua, tx, ty);
  ty += typohi._M_size;

  // SW/HW environment.
  string osnvloc(env.os_name + " " + env.os_version);
  if (!env.os_locale.empty())
    osnvloc += " " + env.os_locale;

  if (!env.hw_name.empty())
    {
      string hwsw = env.hw_name + " / " + osnvloc;
      place_text_at_point(obj, typolarge, hwsw, tx, ty);
      ty += typolarge._M_size;

      if (env.hw_cpu)
	{
	  string corestr = to_string(env.hw_cpu) + " cores";
	  place_text_at_point(obj, typolarge, corestr, tx, ty);
	  ty += typolarge._M_size;

	  int memi = std::round(env.hw_mem * .001);
	  place_text_at_point(obj, typolarge, to_string(memi) + " GB", tx, ty);
	  ty += typolarge._M_size;
	}
    }

  if (swmetadatap)
    {
      place_text_at_point(obj, typom, env.sw_name, tx, ty);
      ty += typom._M_size;
      place_text_at_point(obj, typom, env.sw_arch, tx, ty);
      ty += typom._M_size;
      place_text_at_point(obj, typom, env.sw_version, tx, ty);
      ty += typom._M_size;
      place_text_at_point(obj, typom, env.sw_build_id, tx, ty);
      ty += typom._M_size;
    }

  // URL/Date
  ty += typom._M_size;
  place_text_at_point(obj, typom, env.date_time_stamp, tx, ty);
  ty += typom._M_size;
  place_text_at_point(obj, typom, env.url, tx, ty);
}


void
render_metadata_time(svg_element& obj, const int timen, const color& c,
		     int x, int y)
{
  // Total time.
  typography typot = make_typography_metadata(48, true);
  typot._M_style._M_fill_color = c;

  std::ostringstream oss;
  oss.imbue(std::locale(""));
  oss << timen << " ms";
  place_text_at_point(obj, typot, oss.str(), x, y);
}


void
render_input_files_title(svg_element& obj, const string fstem1,
			 const string fstem2, const string hilights)
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
}


void
render_metadata_title(svg_element& obj, const int time_max, const string fstem1,
		      const string fstem2 = "", const string hilights = "")
{
  render_metadata_time(obj, time_max, color::red,
		       moz::k::margin, obj._M_area._M_height / 2);
  render_input_files_title(obj, fstem1, fstem2, hilights);
}


void
render_metadata_title(svg_element& obj, const int time_max, const string fstem,
		      const color& c, int x, int y)
{
  const typography typom = make_typography_metadata();
  render_metadata_time(obj, time_max, c, x, y);
  place_text_at_point(obj, typom, fstem, x, y + 20);
}

} // namespace moz

#endif
