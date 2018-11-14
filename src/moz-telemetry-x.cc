// telemetry sunburst / RAIL forms -*- mode: C++ -*-

// Copyright (c) 2018, Mozilla
// Benjamin De Kosnik <bdekoz@mozilla.com>

// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

#include <chrono>
#include <iostream>
#include "moz-json-basic.h"

using std::string;


string
usage()
{
  string s("usage: a60-analyze file.json");
  return s;
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

  // Output.
#if 0  
  string odir = io::get_output_directory("tmp");
  io::sanity_check_output_directory(odir);
#endif
  
  // Input file, output directory.
  if (argc > 1)
    {
      string input_file(argv[1]);
      //io::sanity_check_input(input_file);
    }

  

  return 0;
}
