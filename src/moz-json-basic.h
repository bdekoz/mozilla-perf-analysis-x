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
  using jsonstream = rj::PrettyWriter<rj::StringBuffer>;

  //using citerator = rj::Value::ConstMemberIterator;
  using citerator_m = rj::Value::ConstMemberIterator;
  using citerator_v = rj::Value::ConstValueIterator;

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
	std::cerr << "error: cannot parse document" << std::endl;
	std::cerr << rj::GetParseError_En(dom.GetParseError()) << std::endl;
	std::cerr << dom.GetErrorOffset() << std::endl;
      }

    return dom;
  }

  rj::Document
  deserialize_json_to_dom_array(string);

  rj::Document
  deserialize_json_to_dom_object(string);

  // Telemetry data file object parition constants.
  const string sapp("application");
  const string spay("payload");
  const string senv("environment");

  /// Search DOM for objects.
  void
  search_dom_for_object_field_matching(const rj::Document& dom,
				       const string match)
  {
#if 0
    if (!dom.HasParseError() && dom.HasMember(match.c_str()))
      {
	// Get value for field matching...
	const rj::Value& v = dom[match.c_str()];
	if (v.IsObject())
	  {
	    std::clog << match << " object:" << std::endl;
#if 0
	    // All objects in JSON file, XXX could be top-level
	    for (auto& m : v.GetObject())
	      {
		string fname(m.name.GetString());
		string ftype(kTypeNames[m.value.GetType()]);
		std::clog << fname << " " << ftype << std::endl;
	      }

#endif

#if 1
	    for (citerator_mem itr = v.value.MemberBegin(); itr != v.value.MemberEnd(); ++itr)
	      {
		string fname(itr->name.GetString());
		string ftype(kTypeNames[itr->value.GetType()]);
		std::clog << fname << " " << ftype << std::endl;
	      }
#endif
	  }
      }
#else

    if (!dom.HasParseError())
      {
	// http://rapidjson.org/md_doc_pointer.html
	const rj::Value* pv = rapidjson::Pointer(match.c_str()).Get(dom);

	if (pv->IsObject())
	  {
	    for (citerator_m itr = pv->MemberBegin(); itr != pv->MemberEnd(); ++itr)
	      {
		string fname(itr->name.GetString());
		string ftype(kTypeNames[itr->value.GetType()]);
		std::clog << fname << " " << ftype << std::endl;
	      }
	  }
      }
#endif
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

  void
  field_match_extract_v(const rj::Document& dom, const string m, string fname)
  {
    if (fname.compare(0, m.size(), m) == 0)
      {
	const rj::Value& a = dom[fname.c_str()];
	string v;
	if (a.IsString())
	  v = a.GetString();
	else
	  v = std::to_string(a.GetInt());

	std::clog << fname << " : " << v << std::endl;
      }
  }

  void
  walk_dom_for_string_fields_matching(const rj::Document& dom, const string m)
  {
    if (!dom.HasParseError())
      {
	for (citerator_m itr = dom.MemberBegin(); itr != dom.MemberEnd(); ++itr)
	  {
	    auto& dmem = *itr;
	    string fname(dmem.name.GetString());
	    string ftype(kTypeNames[dmem.value.GetType()]);
	    std::clog << fname << " " << ftype << std::endl;

#if 0
	    // Walk matching to clog.
	    if (ftype == "Object")
	      for (uint i = 0; i < dom.Size(); ++i)
		{
		  const rj::Value& v = dom[i];
		  field_match_extract_v(dom, m, fname);
		}
#endif
	  }

      }
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
