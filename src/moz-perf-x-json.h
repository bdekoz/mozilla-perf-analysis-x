// mozilla serialize/deserialize forward declarations -*- mode: C++ -*-

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

#ifndef moz_X_JSON_H
#define moz_X_JSON_H 1

#define RAPIDJSON_HAS_STDSTRING 1

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/pointer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/reader.h"

#include "moz-perf-x.h"


namespace moz {
  namespace {
    const std::string s1 = "histogram-sanity-check-single";
    const std::string s2 = "histogram-sanity-check-multi";
    static std::ofstream ofssinglev = make_log_file(s1);
    static std::ofstream ofsmultiv = make_log_file(s2);
  } // anonymous namespace
} // namespace moz


namespace moz {

/// Namespace aliases.
namespace rj = rapidjson;

/// Types.
using jsonstream = rj::PrettyWriter<rj::StringBuffer>;

using vcmem_iterator = rj::Value::ConstMemberIterator;
using vcval_iterator = rj::Value::ConstValueIterator;

/// Constants.
const char* kTypeNames[] =
{ "Null", "False", "True", "Object", "Array", "String", "Number" };


rj::Document
parse_stringified_json_to_dom(string stringified)
{
 rj::Document dom;
 dom.Parse(stringified.c_str());
 if (dom.HasParseError())
   {
     std::cerr << "error: cannot parse JSON string" << std::endl;
     std::cerr << rj::GetParseError_En(dom.GetParseError()) << std::endl;
     std::cerr << dom.GetErrorOffset() << std::endl;
   }
 return dom;
}


rj::Document
deserialize_json_to_dom(string input_file)
{
  // Deserialize input file.
  std::ifstream ifs(input_file);
  string json;
  if (ifs.good())
    {
      std::ostringstream oss;
      oss << ifs.rdbuf();
      json = oss.str();
    }
  else
    {
      ostringstream mss;
      mss<< k::errorprefix << "deserialize_jason_to_dom:: "
	      << "cannot open input file: "
	      << input_file << std::endl;
      throw std::runtime_error(mss.str());
    }

  // Validate json file, or parse immediately and report error?
  return parse_stringified_json_to_dom(json);
}


// Convert from input file name to an in-memory vector of strings
// representing identifiers/names to match against field names in a
// JSON file.
strings
deserialize_file_to_strings(string inames)
{
  strings probes;
  if (!inames.empty())
    {
      std::ifstream ifs(inames);
      if (ifs.good())
	{
	  string line;
	  do
	    {
	      std::getline(ifs, line);
	      if (ifs.good())
		probes.push_back(line);
	    }
	  while (!ifs.eof());
	  std::sort(probes.begin(), probes.end());

	  std::clog << probes.size() << " match names found in: " << std::endl;
	  std::clog << inames << std::endl;
	  std::clog << std::endl;
	}
      else
	{
	  std::cerr << k::errorprefix << "deserialize_file_to_strings:: "
		    << "cannot open input file: "
		    << inames << std::endl;
	}
    }
  else
    {
      std::clog << "Metric edit file is empty, extracting all metrics.";
      std::clog << std::endl;
    }

  return probes;
}


int
field_value_to_int(const rj::Value& v)
{
  int ret(0);
  if (v.IsNumber())
    ret = v.GetInt64();
  return ret;
}


string
field_value_to_string(const rj::Value& v)
{
  string ret;
  if (v.IsNumber())
    ret = std::to_string(v.GetInt64());
  else if (v.IsString())
    ret = v.GetString();
  else if (v.IsBool())
    ret = std::to_string(v.GetBool());
  else
    {
      // Array or Object.
      ret = "array or object field";
    }
  return ret;
}


string
extract_histogram_field_sum(const rj::Value& v, const string& probe)
{
  string found;
  auto i = v.FindMember(probe.c_str());
  if (i != v.MemberEnd())
    {
      const rj::Value& nv = i->value["sum"];
      found = field_value_to_string(nv);
    }
  return found;
}


// Mean is the sum of the histogram values divided by the number of
// values.
string
extract_histogram_field_mean(const rj::Value& v, const string& probe)
{
  string found;
  auto i = v.FindMember(probe.c_str());
  if (i != v.MemberEnd())
    {
      // Get histogram type.
      const rj::Value& vht = i->value["histogram_type"];
      histogram_t htype = static_cast<histogram_t>(field_value_to_int(vht));
      bool htypecp = htype == histogram_t::categorical;
      bool htypekp = htype == histogram_t::keyed;

      // Get number of buckets.
      const rj::Value& vbcount = i->value["bucket_count"];
      int bcount [[gnu::unused]] = field_value_to_int(vbcount);

      // Get sum.
      const rj::Value& vsum = i->value["sum"];
      int sum = field_value_to_int(vsum);

      // Get (value, count) for each bucket, in the form of (string, int).
      const rj::Value& vvs = i->value["values"];
      if (vvs.IsObject())
	{
	  // Iterate through object.
	  int sumcomputed(0);
	  int nvalues(0);
	  for (vcmem_iterator j = vvs.MemberBegin(); j != vvs.MemberEnd(); ++j)
	    {
	      const rj::Value& vbktcount = j->value;
	      int bktcount = field_value_to_int(vbktcount);
	      nvalues += bktcount;

	      if (!htypecp && !htypekp)
		{
		  // For "most" histograms, the name of the bucket
		  // corresponds to a particular value. So, convert the
		  // buck name above to an int value.
		  string bktname = j->name.GetString();
		  int bktv(std::stoi(bktname));
		  sumcomputed += (bktv * bktcount);
		}
	    }

	  // Sanity check computed sum matches extracted sum.
	  if (sumcomputed != sum || htypecp || htypekp)
	    {
	      std::clog << k::errorprefix << "computed sum of " << sumcomputed
			<< " != extracted sum of " << sum << std::endl;
	    }

	  double mean(sum / nvalues);
	  found = to_string(mean);
	}
    }
  return found;
}


/*
   Median is the value computed from a set of numbers such that the
   probability is equal that any number picked from the set has a
   value higher or lower than it.

   Mozilla telemetry histograms have a particular characteristic, in
   that the bucket immediately to the left (aka, less) of the first
   non-zero value is represented (with a zero count), and the bucket
   immediately to the right (aka more) of the last non-zero value is
   represented (with a zero count).

   Because of this, some single-sample (aka one non-zero value)
   histograms can be exactly represented (or flattened to scalar) by
   using the value of the sum in the case, not computing from the
   histogram buckets. This case will have one value and three entries.
*/
string
extract_histogram_field_median(const rj::Value& v, const string& probe)
{
  string found;
  auto i = v.FindMember(probe.c_str());
  if (i != v.MemberEnd())
    {
      // Get histogram type.
      const rj::Value& vht = i->value["histogram_type"];
      histogram_t htype = static_cast<histogram_t>(field_value_to_int(vht));
      bool htypecp = htype == histogram_t::categorical;
      bool htypekp = htype == histogram_t::keyed;

      // Get (value, count) for each bucket, in the form of (string, int).
      const rj::Value& vvs = i->value["values"];
      if (vvs.IsObject())
	{
	  // Iterate through object.
	  int nvalues(0);
	  std::vector<int> vvalues;
	  for (vcmem_iterator j = vvs.MemberBegin(); j != vvs.MemberEnd(); ++j)
	    {
	      const rj::Value& vbktcount = j->value;
	      int bktcount = field_value_to_int(vbktcount);

	      if (bktcount != 0 && !htypecp && !htypekp)
		{
		  // For "most" histograms, the name of the bucket
		  // corresponds to a particular value. So, convert the
		  // buck name above to an int value.
		  string bktname = j->name.GetString();
		  int bktv(std::stoi(bktname));

		  // Add bktcount number of bktv values to histogram vector.
		  vvalues.insert(vvalues.end(), bktcount, bktv);
		}

	      ++nvalues;
	    }

	  if (!vvalues.empty())
	    {
	      const uint vvsize = vvalues.size();

	      std::ostringstream oss;
	      oss << std::left << std::setfill(' ') << std::setw(48) << probe
		  << k::tab << "sample size: " << std::setw(6) << vvsize
		  << k::tab << "values: " << std::setw(6) << nvalues;

	      // Check for single-sample case, and if true return sum
	      // instead.  Sanity check that there exist zero-fill
	      // buckets to each side, will assume 3 values (zero
	      // left, value, zero right) is exactly this case...
	      if (vvsize == 1 && nvalues == 3)
		{
		  const rj::Value& sum = i->value["sum"];
		  found = field_value_to_string(sum);
		  ofssinglev << oss.str() << std::endl;
		}
	      else
		{
		  std::nth_element(vvalues.begin(),
				   vvalues.begin() + vvsize / 2,
				   vvalues.end());

		  // Median differs by even/odd number of elements...
		  double median(0);
		  if (vvsize % 2 != 0)
		    median = vvalues[vvsize / 2];
		  else
		    {
		      auto m1 = vvalues[vvsize / 2];
		      auto m2 = vvalues[(vvsize / 2) - 1];
		      median = (m1 + m2) / 2;
		    }
		  found = to_string(static_cast<uint>(median));
		  ofsmultiv << oss.str() << std::endl;
		}
	    }
	}
    }
  return found;
}


/// Browsertime pre-calculated histogram summary types.
/// @hview is what type of value to extract
auto
extract_pseudo_histogram_field(const rj::Value& v, const histogram_view_t hview)
{
  auto nvalue = 0;
  switch (hview)
    {
      case histogram_view_t::median:
	{
	  const rj::Value& dnmedian = v["median"];
	  if (dnmedian.IsInt())
	    nvalue = dnmedian.GetInt();
	  else
	    nvalue = dnmedian.GetDouble();
	  break;
	}
      case histogram_view_t::mean:
	{
	  const rj::Value& dnmean = v["mean"];
	  if (dnmean.IsInt())
	    nvalue = dnmean.GetInt();
	  else
	    nvalue = dnmean.GetDouble();
	  break;
	}
      default:
	throw std::runtime_error(k::errorprefix + "histogram extract quantile");
	break;
    }
  return nvalue;
}


string
extract_histogram_field(const rj::Value& v, const string& probe,
			const histogram_view_t hview)
{
  string nvalue;
  switch (hview)
    {
      case histogram_view_t::median:
	nvalue = extract_histogram_field_median(v, probe);
	break;
      case histogram_view_t::mean:
	nvalue = extract_histogram_field_mean(v, probe);
	break;
      case histogram_view_t::sum:
	nvalue = extract_histogram_field_sum(v, probe);
	break;
      case histogram_view_t::quantile:
	throw std::runtime_error(k::errorprefix + "histogram extract quantile");
      default:
	break;
    }
  return nvalue;
}


// Assume v is the base histogram node, probes is the list of
// histogram names to extract.
strings
extract_histogram_fields(const rj::Value& v, const strings& probes,
			 ostream& ofs,
			 const histogram_view_t hview)
{
  strings found;
  if (v.IsObject())
    {
      for (const string& probe : probes)
	{
	  string hvalue = extract_histogram_field(v, probe, hview);
	  if (!hvalue.empty())
	    {
	      ofs << probe << "," << hvalue << std::endl;
	      found.push_back(probe);
	    }
	}
    }
  return found;
}


// Assume v is the base histogram node, extract all sub-nodes as objects.
// Use sum only.
strings
extract_histogram_fields(const rj::Value& v, ostream& ofs)
{
  strings found;
  if (v.IsObject())
    {
      for (vcmem_iterator i = v.MemberBegin(); i != v.MemberEnd(); ++i)
	{
	  string nname = i->name.GetString();

	  const rj::Value& nv = i->value["sum"];
	  string nvalue = field_value_to_string(nv);
	  if (!nvalue.empty())
	    {
	      ofs << nname << "," << nvalue << std::endl;
	      found.push_back(nname);
	    }
	}
    }
  return found;
}


string
extract_scalar_field(const rj::Value& v, const string& probe)
{
  string found;
  auto i = v.FindMember(probe.c_str());
  if (i != v.MemberEnd())
    {
      const rj::Value& nv = i->value;
      found = field_value_to_string(nv);
    }
  return found;
}


strings
extract_scalar_fields(const rj::Value& v, const strings& probes,
		      ostream& ofs)
{
  strings found;
  if (v.IsObject())
    {
      for (const string& probe : probes)
	{
	  string nvalue = extract_scalar_field(v, probe);
	  if (!nvalue.empty())
	    {
	      ofs << probe << "," << nvalue << std::endl;
	      found.push_back(probe);
	    }
	}
    }
  return found;
}


// Environment node only.
environment
extract_environment_mozilla(const rj::Value& denv, bool)
{
  const string kbuild("build");
  const string ksystem("system");
  const string kcpu("cpu");
  const string kos("os");
  const rj::Value& dbuild = denv[kbuild.c_str()];
  const rj::Value& dsystem = denv[ksystem.c_str()];
  const rj::Value& dcpu = dsystem[kcpu.c_str()];
  const rj::Value& dkos = dsystem[kos.c_str()];

  environment env = { };

  env.os_name = field_value_to_string(dkos["name"]);
  env.os_version = field_value_to_string(dkos["version"]);
  env.os_locale = field_value_to_string(dkos["locale"]);

  if (dsystem.HasMember("device"))
    {
      const rj::Value& device = dsystem["device"];
      string manu = field_value_to_string(device["manufacturer"]);
      string model = field_value_to_string(device["model"]);
      env.hw_name = manu + " " + model;
    }
  env.hw_cpu = field_value_to_int(dcpu["count"]);
  env.hw_mem = field_value_to_int(dsystem["memoryMB"]);

  env.sw_name = field_value_to_string(dbuild["applicationName"]);
  env.sw_arch = field_value_to_string(dbuild["architecture"]);
  env.sw_version = field_value_to_string(dbuild["version"]);
  env.sw_build_id = field_value_to_string(dbuild["buildId"]);

  return env;
}


environment
extract_environment_mozilla(const rj::Document& dom)
{
  environment env = { };
  const string kenv("environment");
  if (dom.HasMember(kenv.c_str()))
    {
      const rj::Value& denv = dom[kenv.c_str()];
      env = extract_environment_mozilla(denv, true);

      //payload/processes/parent/scalars
      const char* kparentscalars = "/payload/processes/parent/scalars";
      const rj::Value* pv = rj::Pointer(kparentscalars).Get(dom);
      if (pv)
	{
	  const rj::Value& dscalars = *pv;

	  const char* suri = "browser.engagement.unfiltered_uri_count";
	  if (dscalars.HasMember(suri))
	    {
	      const rj::Value& duri = dscalars[suri];
	      env.uri_count = duri.GetInt();
	    }
	}
    }
  return env;
}


environment
extract_environment_browsertime(const rj::Value& v)
{
  environment env = { };
  const string kinfo("info");
  const string kbrowser("browser");
  const string kua("userAgent");
  if (v.HasMember(kinfo.c_str()))
    {
      const rj::Value& dinfo = v[kinfo.c_str()];

      const string kurl("url");
      const rj::Value& durl = dinfo[kurl.c_str()];

      const string ktimestamp("timestamp");
      const rj::Value& dts = dinfo[ktimestamp.c_str()];

      const string kandroid("android");
      if (dinfo.HasMember(kandroid.c_str()))
	{
	  const rj::Value& ddroid = dinfo[kandroid.c_str()];
	  const rj::Value& ddroidm = ddroid["model"];
	  const rj::Value& ddroidv = ddroid["androidVersion"];
	  env.hw_name = field_value_to_string(ddroidm);
	  env.os_version = field_value_to_string(ddroidv);
	}

      env.os_name = "Android";
      env.uri_count = 1;
      env.url = field_value_to_string(durl);
      env.date_time_stamp = field_value_to_string(dts);

      const string kbrowsers("browserScripts");
      const rj::Value& dbscripts = v[kbrowsers.c_str()];

      if (dbscripts.IsArray())
	{
	  vcval_iterator j = dbscripts.Begin();
	  if (j != dbscripts.End())
	    {
	      const rj::Value& vbscript = *j;

	      // Look for browser info, current Chrome removes this...
	      if (vbscript.HasMember(kbrowser.c_str()))
		{
		  const rj::Value& dbrowser = vbscript[kbrowser.c_str()];
		  if (dbrowser.HasMember(kua.c_str()))
		    {
		      const rj::Value& dua = dbrowser[kua.c_str()];

		      // Remove other user agent compatibility strings.
		      string s = dua.GetString();
		      s = s.substr(0, s.find(')') + 1);
		      env.sw_name = s;
		    }
		}
	    }
	}
    }
  return env;
}


environment
extract_environment_har(const string& harfile)
{
  // Load input JSON data file into DOM.
  rj::Document dom(deserialize_json_to_dom(harfile));

  environment env = { };
  const string klog("log");
  const string kbrowser("browser");
  if (dom.HasMember(klog.c_str()))
    {
      const rj::Value& dlog = dom[klog.c_str()];
      const rj::Value& dbrowser = dlog[kbrowser.c_str()];

      const string kname("name");
      const rj::Value& dname = dbrowser[kname.c_str()];

      const string kver("version");
      const rj::Value& dver = dbrowser[kver.c_str()];

      env.sw_name = field_value_to_string(dname);
      env.sw_version = field_value_to_string(dver);
    }
  return env;
}


/*
  Take environment objects from both the browsertime and mozilla
  telemetry data, and make one unified environment object that
  contains all the fields missing from the separate environment objects.
 */
environment
coalesce_environments(const environment& envmoz, const environment& envbt)
{
  environment env(envmoz);
  env.url = envbt.url;
  env.date_time_stamp = envbt.date_time_stamp;
  return env;
}


void
serialize_environment(const environment& env, string ofile)
{
  rj::StringBuffer sb;
  rj::PrettyWriter<rj::StringBuffer> writer(sb);

  writer.StartObject();

  writer.String("os_vendor");
  writer.String(env.os_vendor);
  writer.String("os_name");
  writer.String(env.os_name);
  writer.String("os_version");
  writer.String(env.os_version);
  writer.String("os_locale");
  writer.String(env.os_locale);

  writer.String("hw_name");
  writer.String(env.hw_name);
  writer.String("hw_cpu");
  writer.Int(env.hw_cpu);
  writer.String("hw_mem");
  writer.Int(env.hw_mem);

  writer.String("sw_name");
  writer.String(env.sw_name);
  writer.String("sw_arch");
  writer.String(env.sw_arch);
  writer.String("sw_version");
  writer.String(env.sw_version);
  writer.String("sw_build_id");
  writer.String(env.sw_build_id);

  writer.String("uri_count");
  writer.Int(env.uri_count);
  writer.String("url");
  writer.String(env.url);
  writer.String("date_time_stamp");
  writer.String(env.date_time_stamp);

  writer.EndObject();

  // Serialize generated output to JSON data file.
  std::ofstream of = make_data_file(ofile, k::environment_ext);
  if (of.good())
    of << sb.GetString();
}


/// Take environment JSON file and return in-memory environment object.
environment
deserialize_json_to_environment(const string ifile)
{
  // Load input JSON data file into DOM.
  rj::Document dom(deserialize_json_to_dom(ifile));

  environment env { };
  if (dom.IsObject() && dom.HasMember("sw_name"))
    {
      env.os_vendor = dom["os_vendor"].GetString();
      env.os_name = dom["os_name"].GetString();
      env.os_version = dom["os_version"].GetString();
      env.os_locale = dom["os_locale"].GetString();

      env.hw_name = dom["hw_name"].GetString();
      env.hw_cpu = dom["hw_cpu"].GetInt();
      env.hw_mem = dom["hw_mem"].GetInt();

      env.sw_name = dom["sw_name"].GetString();
      env.sw_arch = dom["sw_arch"].GetString();
      env.sw_version = dom["sw_version"].GetString();
      env.sw_build_id = dom["sw_build_id"].GetString();

      env.uri_count = dom["uri_count"].GetInt();
      env.url = dom["url"].GetString();
      env.date_time_stamp = dom["date_time_stamp"].GetString();
    }
  else
    {
      string m(k::errorprefix + "deserialize_environment:: JSON error in ");
      m += ifile;
      throw std::runtime_error(m);
    }

  return env;
}


/// Find json file from known last position, and extract environment.
/// @ifile is generated CSV file, aka workingdir/[csv | csv3]/this.csv
environment
deserialize_environment(const string cifile)
{
  // Find environment JSON file from input cfile (.csv) based on
  // assumed or known details about the result directory layout...
  string jfile(cifile);
  auto extpos = jfile.rfind(k::csv_ext);
  if (extpos != string::npos)
    {
      // Replace extension.
      jfile.replace(extpos, 4, k::environment_ext);

      // Replace top level directory with json.
      const string jdir("json");
      auto csv3pos = jfile.rfind("csv3");
      if (csv3pos != string::npos)
	jfile.replace(csv3pos, 4, jdir);
      else
	{
	  auto csvpos = jfile.rfind("csv");
	  if (csvpos != string::npos)
	    jfile.replace(csvpos, 3, jdir);
	  else
	    {
	      std::clog << "deserialize_environment:: no csv in path: " << jfile
			<< std::endl;
	    }
	}

      // Remove -verbose. in *-verbose.environment.json.
      const string verbose("-verbose.");
      auto vpos = jfile.rfind(verbose);
      if (vpos != string::npos)
	jfile.erase(vpos, verbose.size() - 1); // leave the period.
    }
  else
    {
      string m("deserialize_environment:: error environment not found in: ");
      m += jfile;
      throw std::runtime_error(m);
    }
  return deserialize_json_to_environment(jfile);
}


/*
 Assuming a 'Scalars.json' file generated from the canonical source file:
 gecko/toolkit/components/telemetry/Scalars.yaml
 Take it and parse out the individual scalar probe names, like
 timestamps.first_paint
*/
uint
list_object_fields(const rj::Value& v, const string parentfield = "",
		   uint recursen = 1, bool ftypep = false,
		   bool fvalp = false)
{
  uint nfields(0);
  if (v.IsObject())
    {
      for (vcmem_iterator i = v.MemberBegin(); i != v.MemberEnd(); ++i)
	{
	  // Iterate through object
	  string nname = i->name.GetString();
	  string nfield;
	  if (!parentfield.empty())
	    nfield += parentfield + ".";
	  nfield += nname;
	  std::clog << nfield;

	  const rj::Value& nv = i->value;
	  if (ftypep)
	    {
	      string ntype(kTypeNames[nv.GetType()]);
	      std::clog << " " << ntype;
	      if (fvalp)
		{
		  string nvalue = field_value_to_string(nv);
		  std::clog << " " << nvalue;
		}
	    }
	  std::clog << std::endl;

	  if (recursen > 1)
	    {
	      recursen--;
	      list_object_fields(nv, nfield, recursen);
	    }
	  ++nfields;
	}
    }
  return nfields;
}


/// Search DOM for objects.
/// Arguments: first is DOM object, second is display field type
void
list_dom_object_fields(const rj::Document& dom,
		       uint recursen = 1, bool ftypep = false)
{
  if (dom.IsObject() && !dom.HasParseError())
    {
      for (vcmem_iterator i = dom.MemberBegin(); i != dom.MemberEnd(); ++i)
	{
	  const rj::Value& nv = i->value;
	  string nname(i->name.GetString());
	  std::clog << nname << std::endl;
	  if (recursen > 0)
	    list_object_fields(nv, nname, recursen, ftypep);
	}
    }
  else
    std::clog << "list_dom_object_fields:: no DOM object or JSON parse error"
	      << std::endl;
}


/// Search DOM for arrays.
/// Arguments: first is DOM object, second is display field type
void
list_dom_array_fields(const rj::Document& dom,
		      uint recursen = 1, bool ftypep = false)
{
  if (dom.IsArray() && !dom.HasParseError())
    {
      std::clog << "list_dom_array_fields size " << dom.Size() << std::endl;

      uint i(0);
      for (const auto& v : dom.GetArray())
	{
	  if (v.IsObject())
	    list_object_fields(v, "", recursen, ftypep);
	  if (v.IsArray())
	    std::clog << "array " << ++i << " size " << v.Size() << std::endl;
	}
    }
  else
    std::clog << "list_dom_array_fields:: no DOM array or JSON parse error"
	      << std::endl;
}


// List fields in ifile.json.
void
list_json_fields(std::string ifile,  uint recursen)
{
  rj::Document dom(deserialize_json_to_dom(ifile));
  if (dom.IsObject())
    list_dom_object_fields(dom, recursen);
  else
    list_dom_array_fields(dom, recursen);
}


/// Search DOM for string literals.
string
search_dom_for_string_field(const rj::Document& dom, const string finds)
{
  string ret;
  if (!dom.HasParseError() && dom.HasMember(finds.c_str()))
    {
      const rj::Value& a = dom[finds.c_str()];
      if (a.IsString())
	ret = a.GetString();
      else
	ret = std::to_string(a.GetInt());
    }
  return ret;
}


/// Search DOM for integer values.
int
search_dom_for_int_field(const rj::Document& dom, const string finds)
{
  int ret(0);
  if (!dom.HasParseError() && dom.HasMember(finds.c_str()))
    {
      const rj::Value& a = dom[finds.c_str()];
      if (a.IsInt())
	ret = a.GetInt();
    }
  return ret;
}

} // namespace moz
#endif
