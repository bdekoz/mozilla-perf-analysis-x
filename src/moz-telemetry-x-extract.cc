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

namespace moz {

std::string
usage()
{
  std::string s("usage: moz-telemetry-x-extract.exe names.txt data.json");
  return s;
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
extract_tier_1_probes(string inames, string ifile)
{
  const string fstem = file_path_to_stem(ifile);

  // Read probe names from input file, and put into vector<string>
  std::ifstream ifs(inames);
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
		<< datapath + fstem << std::endl;
    }

  // Prepare output file.
  std::ofstream ofs(datapath + fstem + extract_ext);
  if (!ofs.good())
    {
      std::cerr << errorprefix
		<< "cannot open output file " << datapath + fstem
		<< std::endl;
    }

  // Load input JSON data file into DOM.
  rj::Document dom(deserialize_json_to_dom(ifile));

  // Given the known details of the telemetry data schema, save off
  // the most used data nodes for speedier manipulation for specific
  // probes.

  // Iff using RapidJSON pointers...
  // constexpr string ksimple("/payload/processes/parent/scalars");

  const string kpayload("payload");
  if (dom.HasMember(kpayload.c_str()))
    {
      // payload
      // payload/histograms
      // payload/simple
      const string khistograms("histograms");
      const string kscalars("scalars");
      const string ksimple("simpleMeasurements");
      const rj::Value& dpayload = dom[kpayload.c_str()];
      const rj::Value& dhisto = dpayload[khistograms.c_str()];
      const rj::Value& dsimple = dpayload[ksimple.c_str()];

      // payload/processes
      // payload/processes/content/histograms
      const string kproc("processes");
      const string kcontent("content");
      const rj::Value& dproc = dpayload[kproc.c_str()];
      const rj::Value& dcont = dproc[kcontent.c_str()][khistograms.c_str()];;

      // payload/processes/gpu
      const string kgpu("gpu");
      const rj::Value& dgpu = dproc[kgpu.c_str()][khistograms.c_str()];

      // payload/processes/parent/scalars
      const string kparent("parent");
      const rj::Value& dparent = dproc[kparent.c_str()][kscalars.c_str()];

      // Extract histogram values.
      // list_dom_nested_object_fields(dhistogram);

      strings foundhi = extract_histogram_fields_sum(dhisto, probes1, ofs);
      std::copy(foundhi.begin(), foundhi.end(),
		std::back_inserter(probes1found));
      probes1r = update_extract_lists(probes1, foundhi);

      strings foundco = extract_histogram_fields_sum(dcont, probes1r, ofs);
      std::copy(foundco.begin(), foundco.end(),
		std::back_inserter(probes1found));
      probes1r = update_extract_lists(probes1r, foundco);

      strings foundgp = extract_histogram_fields_sum(dgpu, probes1r, ofs);
      std::copy(foundgp.begin(), foundgp.end(),
		std::back_inserter(probes1found));
      probes1r = update_extract_lists(probes1r, foundgp);

      // Extract scalar values.
      // list_dom_nested_object_fields(dsimple);

      strings foundsc = extract_scalar_fields(dsimple, probes1r, ofs);
      std::copy(foundsc.begin(), foundsc.end(),
		std::back_inserter(probes1found));
      probes1r = update_extract_lists(probes1r, foundsc);

      strings foundpa = extract_scalar_fields(dparent, probes1r, ofs);
      std::copy(foundpa.begin(), foundpa.end(),
		std::back_inserter(probes1found));
      probes1r = update_extract_lists(probes1r, foundpa);

      // List remaining.
      std::clog << std::endl << "remaining probes" << std::endl;
      for (const string& s : probes1r)
	std::clog << s << std::endl;
    }
  else
    std::cerr << errorprefix << kpayload << " not found " << std::endl;
}

} // namespace moz


int main(int argc, char* argv[])
{
  using namespace rapidjson;
  using namespace moz;

  // Sanity check.
  if (argc != 3)
    {
      std::cerr << usage() << std::endl;
      return 1;
    }

  // Input names file, input data file
  std::string inames = argv[1];
  std::string idata = argv[2];
  std::clog << "input files: " << inames << " , " << idata << std::endl;

  // Extract data/values from json.
  list_fields(idata);
  extract_named_objects(idata);
  extract_tier_1_probes(inames, idata);

  return 0;
}
