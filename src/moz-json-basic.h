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

#include <fstream>
#include <sstream>
#include <vector>

#define RAPIDJSON_HAS_STDSTRING 1

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/pointer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/reader.h"

namespace moz
{

/// Namespace aliaases.
namespace rj = rapidjson;

/// Types.
using std::string;
using strings = std::vector<std::string>;
using jsonstream = rj::PrettyWriter<rj::StringBuffer>;

using vcmem_iterator = rj::Value::ConstMemberIterator;
using vcval_iterator = rj::Value::ConstValueIterator;

/// RAPIDJSON constants.
const char* kTypeNames[] =
  { "Null", "False", "True", "Object", "Array", "String", "Number" };

// Telemetry data file object parition constants.
const string sapp("application");
const string spay("payload");
const string senv("environment");

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
      std::cerr << "error: cannot parse document" << std::endl;
      std::cerr << rj::GetParseError_En(dom.GetParseError()) << std::endl;
      std::cerr << dom.GetErrorOffset() << std::endl;
    }

  return dom;
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
    ret = v.GetBool();
  else
    {
      // Array or Object.
      ret = "non-plain old field";
    }
  return ret;
}


// Assume v is the histogram node.
strings
extract_dom_histogram_fields(const rj::Value& v, const strings& probes,
			     std::ofstream& ofs)
{
  strings found;
  if (v.IsObject())
    {
      for (const string& probe : probes)
	{
	  auto i = v.FindMember(probe.c_str());
	  if (i != v.MemberEnd())
	    {
	      const rj::Value& nv = i->value["sum"];
	      string nvalue = field_value_to_string(nv);
	      ofs << probe << "," << nvalue << std::endl;
	      found.push_back(probe);
	    }
	}
    }
  return found;
}


strings
extract_dom_scalar_fields(const rj::Value& v, const strings& probes,
			  std::ofstream& ofs)
{
  strings found;
  if (v.IsObject())
    {
      for (const string& probe : probes)
	{
	  auto i = v.FindMember(probe.c_str());
	  if (i != v.MemberEnd())
	    {
	      const rj::Value& nv = i->value;
	      string nvalue = field_value_to_string(nv);
	      ofs << probe << "," << nvalue << std::endl;
	      found.push_back(probe);
	    }
	}
    }
  return found;
}


void
list_dom_nested_object_fields(const rj::Value& v)
{
  if (v.IsObject())
    {
      for (vcmem_iterator i = v.MemberBegin(); i != v.MemberEnd(); ++i)
	{
	  // Iterate through object
	  string nname = i->name.GetString();
	  std::clog << nname;

	  const rj::Value& nv = i->value;
	  string ntype(kTypeNames[nv.GetType()]);
	  string nvalue = field_value_to_string(nv);
	  std::clog << " : " << ntype << " : " << nvalue << std::endl;

	  //const rj::Value& nestedv = v[nname.c_str()];
	}
    }
}


void
search_dom_object_field_contents(const rj::Document& dom, const string match)
{

  if (!dom.HasParseError() && dom.HasMember(match.c_str()))
    {
      std::clog << match << " nested object found with members:" << std::endl;
      const rj::Value& v = dom[match.c_str()];
      list_dom_nested_object_fields(v);
    }
}


/// Search DOM for objects.
void
list_dom_fields(const rj::Document& dom)
{
  if (!dom.HasParseError())
    {
      for (vcmem_iterator i = dom.MemberBegin(); i != dom.MemberEnd(); ++i)
	{
	  string fname(i->name.GetString());
	  string ftype(kTypeNames[i->value.GetType()]);
	  std::clog << fname << " " << ftype << std::endl;
	}
    }
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
