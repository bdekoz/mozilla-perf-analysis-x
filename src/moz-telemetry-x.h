// mozilla serialize/deserialize forward declarations -*- mode: C++ -*-

// Copyright (c) 2018, Mozilla
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

#ifndef moz_TELEMETRY_X_H
#define moz_TELEMETRY_X_H 1

#include <fstream>
#include <experimental/filesystem>
#include <sstream>
#include <vector>


namespace moz
{
  namespace filesystem = std::experimental::filesystem;

  const string prefixpath("/home/bkoz/src/mozilla-telemetry-x/");
  const string datapath(prefixpath + "data/");

  const string errorprefix("error -> ");

  // Common output file extentions.
  const char* extract_ext = ".csv";
  //const char* analyze_ext = ".svg";

  // Sanity check input file and path exist, and then return stem.
  string
  file_path_to_stem(string ifile)
  {
    filesystem::path ipath(ifile);
    if (!exists(ipath))
      throw std::runtime_error("moz::path_to_stem:: could not find " + ifile);
    return ipath.stem().string();
  }

} // namespace moz

#endif
