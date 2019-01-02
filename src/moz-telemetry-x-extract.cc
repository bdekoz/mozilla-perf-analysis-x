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


// Convert from input file name to an in-memory vector of strings
// representing names to match against field names in a JSON file.
strings
deserialize_extract_names(string inames)
{
  strings probes;

  std::ifstream ifs(inames);
  if (ifs.good())
    {
      string line;
      do
	{
	  std::getline(ifs, line);
	  probes.push_back(line);
	}
      while (ifs.good());
      std::sort(probes.begin(), probes.end());

      std::clog << probes.size() << " match names found in: " << std::endl;
      std::clog << inames << std::endl;
    }
  else
    {
      std::cerr << errorprefix
		<< "error: cannot open input file " << inames << std::endl;
    }

  return probes;
}


std::ofstream
make_extracted_data_file(string fstem)
{
  // Prepare output file.
  const string ofile(fstem + extract_ext);
  std::ofstream ofs(ofile);
  if (ofs.good())
    {
      // Add zero as starting point.
      ofs << "START" << "," << 0 << std::endl;
    }
  else
    std::cerr << errorprefix << "cannot open output file "
	      << ofile << std::endl;
  return ofs;
}


/*
  Takes two arguments

  1. text file with probe names to extract from Mozilla telemetry.
  2. input telemetry main ping JSON file

  Output is a CSV file of probe names with extracted values
 */
void
extract_moz_probe_names(string inames, string ifile)
{
  // Read probe names from input file, and put into vector<string>
  strings probes = deserialize_extract_names(inames);

  string ofname(file_path_to_stem(ifile) + "-x-" + file_path_to_stem(inames));
  std::ofstream ofs(make_extracted_data_file(ofname));

  // Load input JSON data file into DOM.
  rj::Document dom(deserialize_json_to_dom(ifile));

  // Given the known details of the Mozilla telemetry data schema,
  // save off the most used data nodes for speedier manipulation for
  // specific probes.

  // Iff using RapidJSON pointers...
  // constexpr string ksimple("/payload/processes/parent/scalars");

  const string kpayload("payload");
  if (dom.HasMember(kpayload.c_str()))
    {
      strings probesfound;
      strings probesr;

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

      strings foundhi = extract_histogram_fields(dhisto, probes, ofs);
      std::copy(foundhi.begin(), foundhi.end(),
		std::back_inserter(probesfound));
      probesr = update_extract_lists(probes, foundhi);

      strings foundco = extract_histogram_fields(dcont, probesr, ofs);
      std::copy(foundco.begin(), foundco.end(),
		std::back_inserter(probesfound));
      probesr = update_extract_lists(probesr, foundco);

      strings foundgp = extract_histogram_fields(dgpu, probesr, ofs);
      std::copy(foundgp.begin(), foundgp.end(),
		std::back_inserter(probesfound));
      probesr = update_extract_lists(probesr, foundgp);

      // Extract scalar values.
      // list_dom_nested_object_fields(dsimple);

      strings foundsc = extract_scalar_fields(dsimple, probesr, ofs);
      std::copy(foundsc.begin(), foundsc.end(),
		std::back_inserter(probesfound));
      probesr = update_extract_lists(probesr, foundsc);

      strings foundpa = extract_scalar_fields(dparent, probesr, ofs);
      std::copy(foundpa.begin(), foundpa.end(),
		std::back_inserter(probesfound));
      probesr = update_extract_lists(probesr, foundpa);

      // List remaining.
      std::clog << std::endl;
      std::clog << probesr.size() << " remaining probes: " << std::endl;
      for (const string& s : probesr)
	std::clog << '\t' << s << std::endl;
    }
  else
    std::cerr << errorprefix << kpayload << " not found " << std::endl;
}


/*
  Takes a text file with probe names to extract from a browsertime JSON file.

  Output is a CSV file of probe names with extracted values

  Browsertime
  https://www.sitespeed.io/documentation/browsertime/

  See the following for more detail

  Navigation Timing
  https://w3c.github.io/navigation-timing/

  User Timing
  https://www.w3.org/TR/user-timing/

  HAR 1.2
  http://www.softwareishard.com/blog/har-12-spec/
 */
void
extract_browsertime_probe_names(string inames, string ifile)
{
  // Read probe names from input file, and put into vector<string>
  strings probes = deserialize_extract_names(inames);

  string ofname(file_path_to_stem(ifile) + "-x-" + file_path_to_stem(inames));
  std::ofstream ofs(make_extracted_data_file(ofname));

  // Load input JSON data file into DOM.
  rj::Document dom(deserialize_json_to_dom(ifile));

  const string kinfo("info");
  if (dom.HasMember(kinfo.c_str()))
    {
      strings probesfound;
      strings probesr;
    }
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

  // Input match names file, input JSON data file

  std::string inames = argv[1];
  std::string idata = argv[2];
  std::clog << "input files: " << inames << " , " << idata << std::endl;
  std::clog << std::endl;

  // Extract data/values from json.
  // This is useful for generating a list of Histograms and Scalar probe names.

  list_json_fields(idata);

  // extract_moz_probe_names(inames, idata);
  extract_browsertime_probe_names(inames, idata);

  return 0;
}
