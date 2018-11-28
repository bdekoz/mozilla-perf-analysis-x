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
#include <sstream>
#include <vector>


namespace moz
{
  const string prefixpath("/home/bkoz/src/mozilla-telemetry-x/");
  const string datapath(prefixpath + "data/");

  const string testfile("mozilla-telemetry-test-data.txt");

  const string tier1file("mozilla-telemetry-names-tier-1.txt");
  const string tier1outfile("mozilla-telemetry-names-tier-1-data.txt");

  const string errorprefix("error -> ");
} // namespace moz

#endif
