// mozilla performance analysis -*- mode: C++ -*-

// Copyright (c) 2018-2020, Mozilla
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

#ifndef moz_PERF_X_H
#define moz_PERF_X_H 1

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iomanip>
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

  // Whitespace constants in pixels.
  constexpr int margin = 100;
  constexpr int spacer = 10;

} // namespace moz::constants



namespace moz {

/// Types.
using std::string;
using std::to_string;
using strings = std::vector<std::string>;

using ostream = std::ostream;
using ofstream = std::ofstream;
using ostringstream = std::ostringstream;

using istream = std::istream;
using iftream = std::ifstream;
using istringstream = std::istringstream;

using point = std::tuple<double, double>;
using pointn = std::tuple<point, int>;

/// Alias namespace moz::filesystem to std::experimental::filesystem.
namespace filesystem = std::experimental::filesystem;

/// Alias namespace moz::k to mozilla::constants.
namespace k = moz::constants;


/// Point to string.
string
to_string(point p)
{
  auto [ x, y ] = p;
  std::ostringstream oss;
  oss << x << k::comma << y;
  return oss.str();
}


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
  quantile = 3,
  range = 4
};


/// Compile time switches for input data processing.
enum class json_t
{
  browsertime,
  browsertime_log,
  har,
  hybrid,
  mozilla_desktop,
  mozilla_android,
  mozilla_snapshot_e,
  mozilla_snapshot_h,
  mozilla_snapshot_s,
  mozilla_glean,
  w3c
};


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

  string	hw_name;
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


std::ofstream
make_data_file(const string fstem, const string ext,
	       const std::ios_base::openmode mode = std::ios_base::out)
{
  // Prepare output file.
  const string ofile(fstem + ext);
  std::ofstream ofs(ofile, mode);
  if (!ofs.good())
    std::cerr << k::errorprefix << "cannot open output file "
	      << ofile << std::endl;
  return ofs;
}


std::ofstream
make_log_file(const string fstem)
{
  // Log details of the histogram to output files.
  using openmode = std::ios_base::openmode;
  openmode mode = std::ios_base::app | std::ios_base::out;
  std::ofstream ofs = make_data_file(fstem, ".log", mode);
  return ofs;
}


/// Get filesystem path to the toplevel of the source directory.
string
get_prefix_path()
{
  const char* mtxenv = "MOZPERFAX";
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
