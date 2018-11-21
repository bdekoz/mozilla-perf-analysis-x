// telemetry sunburst / RAIL forms -*- mode: C++ -*-

// Copyright (c) 2018, Mozilla
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

#include <chrono>
#include <iostream>
#include <algorithm>
#include "moz-json-basic.h"


using namespace moz;

const string errorprefix("error -> ");

std::string
usage()
{
  std::string s("usage: a60-analyze file.json");
  return s;
}


void
list_fields(std::string ifile)
{
  rj::Document dom(deserialize_json_to_dom(ifile));
  list_dom_fields(dom);
}


/*
  network
  parse
  style
  layout
  paint/composite

  TIME_TO_*
  *_MS
 */
void
extract_named_objects(std::string ifile)
{
  rj::Document dom(deserialize_json_to_dom(ifile));

  std::cout << " 1 " << std::endl;
  search_dom_object_field_contents(dom, sapp);

  std::cout << " 2 " << std::endl;
  search_dom_object_field_contents(dom, spay);

  std::cout << " 3 " << std::endl;
  search_dom_object_field_contents(dom, senv);

  std::cout << std::endl;
}


strings
update_extract_lists(const strings& total, strings& found)
{
  // Update accounting of found, to-find.
  strings probes1r;
  std::set_difference(total.begin(), total.end(),
		      found.begin(), found.end(),
		      std::inserter(probes1r, probes1r.begin()));

  std::clog << std::endl;
  std::clog << "probes total: " << total.size() << std::endl;
  std::clog << "probes found: " << found.size() << std::endl;
  std::clog << "probes remaining: " << probes1r.size() << std::endl;
  std::clog << std::endl;

  return probes1r;
}


/*
  Takes a text file with all Tier 1 probes.
 */
void
extract_tier_1_probes(std::string ifile)
{
  // Read probe names from input file, and put into vector<string>
  const string dpath("/home/bkoz/src/mozilla-telemetry-x/data/");
  const string tier1file("mozilla-telemetry-names-tier-1.txt");
  std::ifstream ifs(dpath + tier1file);
  strings probes1;
  strings probes1found;
  strings probes1r;
  if (ifs.good())
    {
      string line;
      do
	{
	  std::getline(ifs, line);
	  probes1.push_back(line);
	}
      while (ifs.good());
      std::sort(probes1.begin(), probes1.end());

      std::clog << probes1.size() << " tier 1 probes found" << std::endl;
    }
  else
    {
      std::cerr << errorprefix
		<< "error: cannot open input file "
		<< dpath + tier1file << std::endl;
    }

  // Prepare output file.
  const string tier1outfile("mozilla-telemetry-names-tier-1-data.txt");
  std::ofstream ofs(dpath + "../" + tier1outfile);
  if (!ofs.good())
    {
      std::cerr << errorprefix
		<< "cannot open output file " << dpath + "../" + tier1outfile
		<< std::endl;
    }

  // Load input JSON data file into DOM.
  rj::Document dom(deserialize_json_to_dom(ifile));

  // Given the known details of the telemetry data schema, save off
  // the most used data nodes for speedier manipulation for specific
  // probes.
  const string kpayload("payload");
  const string khistograms("histograms");
  const string ksimple("simpleMeasurements");

  // Iff using RapidJSON pointers...
  // constexpr string ksimple("/payload/processes/parent/scalars");

  if (dom.HasMember(kpayload.c_str()))
    {
      const rj::Value& dpayload = dom[kpayload.c_str()];
      const rj::Value& dhistogram = dom[kpayload.c_str()][khistograms.c_str()];
      const rj::Value& dsimple = dom[kpayload.c_str()][ksimple.c_str()];

      // Extract histogram values.
      // list_dom_nested_object_fields(dhistogram);
      strings foundhi = extract_dom_histogram_fields(dhistogram, probes1, ofs);
      std::copy(foundhi.begin(), foundhi.end(),
		std::back_inserter(probes1found));
      probes1r = update_extract_lists(probes1, foundhi);

      // Extract scalar values.
      // list_dom_nested_object_fields(dsimple);
      strings foundsc = extract_dom_scalar_fields(dsimple, probes1r, ofs);
      std::copy(foundsc.begin(), foundsc.end(),
		std::back_inserter(probes1found));
      probes1r = update_extract_lists(probes1r, foundsc);
    }
  else
    std::cerr << errorprefix << kpayload << " not found " << std::endl;
}


int main(int argc, char* argv[])
{
  using namespace rapidjson;

  // Sanity check.
  if (argc > 2)
    {
      std::cerr << usage() << std::endl;
      return 1;
    }

  // Input file, output directory.
  std::string ifile;
  if (argc > 1)
    {
      ifile = argv[1];
    }
  else
    {
      std::clog << usage() << std::endl;
      return 1;
    }
  std::clog << "input file: " << ifile << std::endl;

  // Extract data/values from json.
  list_fields(ifile);
  extract_named_objects(ifile);
  extract_tier_1_probes(ifile);

  return 0;
}
