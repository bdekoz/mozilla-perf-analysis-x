// mozilla serialize/deserialize forward declarations -*- mode: C++ -*-

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

#ifndef moz_TELEMETRY_X_H
#define moz_TELEMETRY_X_H 1

#include <fstream>
#include <experimental/filesystem>
#include <sstream>
#include <vector>


namespace moz {

namespace filesystem = std::experimental::filesystem;

/// Types.
using std::string;
using std::to_string;
using strings = std::vector<std::string>;

const string prefixpath("/home/bkoz/src/mozilla-telemetry-x/");
const string datapath(prefixpath + "data/");

const string errorprefix("error -> ");

// Common output file extentions.
const char* extract_ext = ".csv";
const char* analyze_ext = ".svg";

// Margin in pixels.
const int margin = 100;

/*
  Histogram types, from nsITelemetry.idl

  HISTOGRAM_EXPONENTIAL - buckets increase exponentially
  HISTOGRAM_LINEAR - buckets increase linearly
  HISTOGRAM_BOOLEAN - For storing 0/1 values
  HISTOGRAM_FLAG - For storing a single value; its count is always == 1.
  HISTOGRAM_COUNT - For storing counter values without bucketing.
  HISTOGRAM_CATEGORICAL - For storing enumerated values by label.
  */
enum class histogram_t
{
  exponential = 0,
  linear = 1,
  boolean = 2,
  flag = 3,
  count = 4,
  categorical = 5,
  keyed = 6
};


/// Compile time switches for histogram extraction.
enum class histogram_view_t
{
  sum = 0,
  median = 1,
  mean = 2,
  quantile = 3
};


/// Compile time switches for input data processing, JSON format.
enum class json_t
{
  browsertime,
  mozilla,
  w3c
};


/*
  Environmental Metadata

  distributionId
  os.name
  os.version
  os.locale
  cpu.count
  memoryMB
  applicationName
  architecture
  version
    buildId
    browser.engagement.total_uri_count
*/
struct environment
{
  string	os_vendor;
  string	os_name;
  string	os_version;
  string	os_locale;

  int		hw_cpu;
  int		hw_mem;

  string	sw_name;
  string	sw_arch;
  string	sw_version;
  string	sw_build_id;

  int		uri_count;
  string	url;
  string	date_time_stamp;
};


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
