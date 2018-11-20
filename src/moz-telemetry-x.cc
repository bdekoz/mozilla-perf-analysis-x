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
#include "moz-json-basic.h"


using namespace moz;

std::string
usage()
{
  std::string s("usage: a60-analyze file.json");
  return s;
}


void
extract_fields(std::string ifile)
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
}


/*
  Takes a text file with all Tier 1 probes.
 */
void
extract_tier_1_probes(std::string ifile)
{
  // Read probe names from input file.
  const string dpath("/home/bkoz/src/mozilla-telemetry-x/data/");
  const string tier1file("mozilla-telemetry-names-tier-1.txt");
  std::ifstream ifs(dpath + tier1file);
  if (!ifs.good())
    {
      std::cerr << "error: cannot open input file "
		<< dpath + tier1file << std::endl;
    }

  // Prepare output file.
  const string tier1outfile("mozilla-telemetry-names-tier-1-data.txt");
  std::ofstream ofs(dpath + "../" + tier1outfile);
  if (!ofs.good())
    {
      std::cerr << "error: cannot open output file "
		<< dpath + "../" + tier1outfile << std::endl;
    }

  // Load input JSON data file into DOM.
  rj::Document dom(deserialize_json_to_dom(ifile));

  // Loop through probes, extract values, output as CSV
  // DNW for deeply nested data.
  const string errormissing("error not found in data");
  string line;
  do
    {
      std::getline(ifs, line);
      std::cerr << line << std::endl;

      if (dom.HasMember(line.c_str()))
	{
	  const rj::Value& a = dom[line.c_str()];
	  string s = field_value_to_string(a);
	  ofs << line << "," << s << std::endl;
	}
      else
	{
	  // std::cerr << line << " " << errormissing << std::endl;
	}
    }
  while (!line.empty()); // ifs.good()
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

  // Extract from json.
  extract_fields(ifile);

  extract_named_objects(ifile);

  extract_tier_1_probes(ifile);

  return 0;
}
