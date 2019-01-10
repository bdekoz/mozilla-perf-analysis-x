// mozilla serialize/deserialize forward declarations -*- mode: C++ -*-

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

#ifndef moz_JSON_FWD_H
#define moz_JSON_FWD_H 1

#define RAPIDJSON_HAS_STDSTRING 1

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/pointer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/reader.h"

#include "moz-telemetry-x.h"


namespace moz {

/// Namespace aliases.
namespace rj = rapidjson;

/// Types.
using std::string;
using strings = std::vector<std::string>;
using jsonstream = rj::PrettyWriter<rj::StringBuffer>;

using vcmem_iterator = rj::Value::ConstMemberIterator;
using vcval_iterator = rj::Value::ConstValueIterator;

/// Constants.
const char* kTypeNames[] =
  { "Null", "False", "True", "Object", "Array", "String", "Number" };


void
serialize_to_json(jsonstream&, string);


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
      std::cerr << "error: cannot open input file "
		<< input_file << std::endl;
    }

  // Validate json file.

  // DOM
  rj::Document dom;
  dom.Parse(json.c_str());
  if (dom.HasParseError())
    {
      std::cerr << "error: cannot parse JSON file " << input_file << std::endl;
      std::cerr << rj::GetParseError_En(dom.GetParseError()) << std::endl;
      std::cerr << dom.GetErrorOffset() << std::endl;
    }

  return dom;
}


int
field_value_to_int(const rj::Value& v)
{
  int ret(0);
  if (v.IsNumber())
    ret = v.GetInt();
  return ret;
}


string
field_value_to_string(const rj::Value& v)
{
  string ret;
  if (v.IsNumber())
    ret = std::to_string(v.GetInt());
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
	      std::clog << errorprefix << "computed sum of " << sumcomputed
			<< " != extracted sum of " << sum << std::endl;
	    }

	  double mean(sum / nvalues);
	  found = to_string(mean);
	}
    }
  return found;
}


// Median is the value computed from a set of numbers such that the
// probability is equal that any number picked from the set has a
// value higher or lower than it.
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
	    }

	  if (!vvalues.empty())
	    {
	      const uint vvsize = vvalues.size();
	      std::nth_element(vvalues.begin(), vvalues.begin() + vvsize / 2,
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
	    }
	}
    }
  return found;
}


// Use with Browsertime pre-calculated histogram summary types.
int
extract_pseudo_histogram_field(const rj::Value& v, const histogram_view_t hview)
{
  int nvalue;
  switch (hview)
    {
      case histogram_view_t::median:
	{
	  const rj::Value& dnmedian = v["median"];
	  nvalue = dnmedian.GetInt();
	  break;
	}
      case histogram_view_t::mean:
	{
	  const rj::Value& dnmean = v["mean"];
	  nvalue = dnmean.GetInt();
	  break;
	}
      default:
	throw std::runtime_error(errorprefix + "histogram extract quantile");
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
	throw std::runtime_error(errorprefix + "histogram extract quantile");
      default:
	break;
    }
  return nvalue;
}


// Assume v is the histogram node.
strings
extract_histogram_fields(const rj::Value& v, const strings& probes,
			 std::ofstream& ofs,
			 const histogram_view_t hview = histogram_view_t::median)
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
		      std::ofstream& ofs)
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


environment
extract_environment_mozilla(string ifile)
{
  const string fstem = file_path_to_stem(ifile);

  // Load input JSON data file into DOM.
  rj::Document dom(deserialize_json_to_dom(ifile));

  environment env = { };
  const string kenv("environment");
  if (dom.HasMember(kenv.c_str()))
    {
      const string kbuild("build");
      const string kpartner("partner");
      const string ksystem("system");
      const string kcpu("cpu");
      const string kos("os");
      const rj::Value& denv = dom[kenv.c_str()];
      const rj::Value& dbuild = denv[kbuild.c_str()];
      const rj::Value& dpartner = denv[kpartner.c_str()];
      const rj::Value& dsystem = denv[ksystem.c_str()];
      const rj::Value& dcpu = dsystem[kcpu.c_str()];
      const rj::Value& dkos = dsystem[kos.c_str()];

      env.os_vendor = field_value_to_string(dpartner["distributor"]);
      env.os_name = field_value_to_string(dkos["name"]);
      env.os_version = field_value_to_string(dkos["version"]);
      env.os_locale = field_value_to_string(dkos["locale"]);

      env.hw_cpu = field_value_to_int(dcpu["count"]);
      env.hw_mem = field_value_to_int(dsystem["memoryMB"]);

      env.sw_name = field_value_to_string(dbuild["applicationName"]);
      env.sw_arch = field_value_to_string(dbuild["architecture"]);
      env.sw_version = field_value_to_string(dbuild["version"]);
      env.sw_build_id = field_value_to_string(dbuild["buildId"]);

      //payload/processes/parent/scalars
      rj::Value* pv = rj::Pointer("/payload/processes/parent/scalars").Get(dom);
      if (pv)
	{
	  const rj::Value& dscalars = *pv;

	  const char* suri = "browser.engagement.unfiltered_uri_count";
	  const rj::Value& duri = dscalars[suri];
	  env.uri_count = duri.GetInt();
	}
    }
  return env;
}


environment
extract_environment_browsertime(string ifile)
{
  const string fstem = file_path_to_stem(ifile);

  // Load input JSON data file into DOM.
  rj::Document dom(deserialize_json_to_dom(ifile));

  environment env = { };
  const string kinfo("info");
  if (dom.HasMember(kinfo.c_str()))
    {
      const rj::Value& dinfo = dom[kinfo.c_str()];

      const string kurl("url");
      const rj::Value& durl = dinfo[kurl.c_str()];

      const string ktimestamp("timestamp");
      const rj::Value& dts = dinfo[ktimestamp.c_str()];

      env.uri_count = 1;
      env.url = field_value_to_string(durl);
      env.date_time_stamp = field_value_to_string(dts);

      const string kbrowsers("browserScripts");
      const rj::Value& dbscripts = dom[kbrowsers.c_str()];

      if (dbscripts.IsArray())
	{
	  vcval_iterator j = dbscripts.Begin();
	  if (j != dbscripts.End())
	    {
	      const rj::Value& vbscript = *j;

	      const rj::Value& dbrowser = vbscript["browser"];
	      const rj::Value& dua = dbrowser["userAgent"];
	      env.sw_name = dua.GetString();
	    }
	}
    }
  return env;
}


// Expecting JSON input.
environment
extract_environment(string ifile, const json_t dformat)
{
  // Extract from known format.
  if (dformat == json_t::mozilla)
    return extract_environment_mozilla(ifile);
  if (dformat == json_t::browsertime)
    return extract_environment_browsertime(ifile);

  // Else error.
  string m(errorprefix + "extract_environment unsupported JSON format");
  throw std::runtime_error(m);
}


void
serialize_environment(const environment & env, string ofname)
{
}


/*
 Assuming a 'Scalars.json' file generated from the canonical source file:
 gecko/toolkit/components/telemetry/Scalars.yaml
 Take it and parse out the individual scalar probe names, like
 timestamps.first_paint
*/
uint
list_object_fields(const string parentfield, const rj::Value& v,
		   bool recursep = true, bool ftypep = false,
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

	  if (recursep)
	    list_object_fields(nfield, nv);
	  ++nfields;
	}
    }
  return nfields;
}


/// Search DOM for objects.
/// Arguments: first is DOM object, second is display field type
void
list_dom_fields(const rj::Document& dom,
		bool nestedp = true, bool ftypep = false)
{
  if (!dom.HasParseError())
    {
      for (vcmem_iterator i = dom.MemberBegin(); i != dom.MemberEnd(); ++i)
	{
	  string nname(i->name.GetString());
	  std::clog << nname;

	  const rj::Value& nv = i->value;
	  if (ftypep)
	    {
	      string ntype(kTypeNames[nv.GetType()]);
	      std::clog << " " << ntype;
	    }
	  std::clog << std::endl;

	  if (nestedp)
	    list_object_fields(nname, nv, true);
	}
    }
}


// List fields in ifile.json.
void
list_json_fields(std::string ifile)
{
  rj::Document dom(deserialize_json_to_dom(ifile));
  list_dom_fields(dom);
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
