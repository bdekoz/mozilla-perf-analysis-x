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

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <experimental/filesystem>


namespace moz::constants {

  // Formatting.
  constexpr char space(' ');
  constexpr char quote('"');
  constexpr char hypen('-');
  constexpr char tab('\t');
  constexpr char newline('\n');
  constexpr char comma(',');
  constexpr char pathseparator('/');

  // Warning/Error prefixes.
  const std::string errorprefix = "error -> ";

  // Output file extentions.
  constexpr const char* csv_ext = ".csv";
  constexpr const char* environment_ext = ".environment.json";
  constexpr const char* analyze_ext = ".svg";

  // Margin in pixels.
  constexpr int margin = 100;

  constexpr double pi(3.14159);

} // namespace moz::constants



namespace moz {

/// Types.
using std::string;
using std::to_string;
using strings = std::vector<std::string>;

using point = std::tuple<double, double>;

// Previous unique value and size of ids with its value.
using uvalue_tuple = std::tuple<int, int>;


/// Alias namespace moz::filesystem to std::experimental::filesystem.
namespace filesystem = std::experimental::filesystem;

/// Alias namespace moz::k to mozilla::constants.
namespace constants { }
namespace k = moz::constants;


/**
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

constexpr histogram_view_t dhview_t = histogram_view_t::sum;


/// Compile time switches for input data processing, JSON format.
enum class json_t
{
  browsertime,
  har,
  mozilla,
  mozilla_android,
  w3c
};

constexpr json_t djson_t = json_t::mozilla_android;


/**
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

/// Hash map of unique id to (not necessarily) unique value.
/// Use this for sorting by id.
using id_value_umap = std::unordered_map<string, int>;

/// Hash multimap of unique value to (perhaps multiple) unique ids.
/// Use this form for sorting by value.
using value_id_ummap = std::unordered_multimap<int, string>;
using value_set = std::set<int>;

/// Remove all from map that match the input (matches) strings.
/// Return found match entries.
id_value_umap
remove_matches_id_value_map(id_value_umap& ivm, const strings& matches)
{
  id_value_umap foundmap;
  for (const auto& key: matches)
    {
      id_value_umap::iterator iter = ivm.find(key);
      if (iter != ivm.end())
	{
	  // Insert found element into return map....
	  foundmap.insert(*iter);

	  // Remove found elment from originating map (ivm)
	  ivm.erase(iter);
	}
    }
  return foundmap;
}


/// Convert id_value_umap to value_id_mmap + set of unique values.
value_id_ummap
to_value_id_mmap(const id_value_umap& ivm, value_set& uniquev)
{
  value_id_ummap vimm;
  for (auto& e: ivm)
    {
      string s = e.first;
      int i = e.second;
      vimm.insert(make_pair(i, s));
      uniquev.insert(i);
    }
  return vimm;
}


/// Sanity check input file and path exist, and then return stem.
string
file_path_to_stem(string ifile)
{
  string ret(ifile);
  if (!ifile.empty())
    {
      filesystem::path ipath(ifile);
      if (!exists(ipath))
	throw std::runtime_error("moz::path_to_stem:: could not find " + ifile);
      ret = ipath.stem().string();
    }
  return ret;
}


/// Get filesystem path to the toplevel of the source directory.
string
get_prefix_path()
{
  const char* mtxenv = "MOZTELEMETRYX";
  char* ppath;
  ppath = getenv(mtxenv);
  if (ppath == nullptr)
    {
      string m("moz::get_prefix_path environment variable ");
      m += mtxenv;
      m += " not set";
      throw std::runtime_error(m);
    }

  string spath(ppath);
  if (spath.back() != k::pathseparator)
    spath += k::pathseparator;
  std::clog << mtxenv << " is: " << spath << std::endl;
  return spath;
}


/// Get filesystem path to the toplevel of the data directory.
string
get_data_path()
{
  string prefixp(get_prefix_path());
  return prefixp + "data/";
}
} // namespace moz

#endif
