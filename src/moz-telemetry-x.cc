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
extract_probes(std::string ifile)
{
  rj::Document dom(deserialize_json_to_dom(ifile));
  walk_dom_for_string_fields_matching(dom, "TIME_TO");
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
  extract_probes(ifile);

  return 0;
}
