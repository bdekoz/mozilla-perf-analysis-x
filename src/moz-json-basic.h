// alpha60 serialize/deserialize forward declarations -*- mode: C++ -*-

// alpha60
// bittorrent x scrape x data + analytics

// Copyright (c) 2018, Benjamin De Kosnik <b.dekosnik@gmail.com>

// This file is part of the alpha60 library.  This library is free
// software; you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software
// Foundation; either version 3, or (at your option) any later
// version.

// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

#ifndef a60_JSON_FWD_H
#define a60_JSON_FWD_H 1

#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"


namespace a60
{
  /// Types.
  using std::string;

  /// Scopes.
  namespace rj = rapidjson;
  using jsonstream = rj::PrettyWriter<rj::StringBuffer>;

  void
  serialize_to_json(jsonstream&, string);

  rj::Document
  deserialize_json_to_dom(string);

  rj::Document
  deserialize_json_to_dom_array(string);

  rj::Document
  deserialize_json_to_dom_object(string);

  // search_dom_for_string_match
  // search_dom_for_string_containing  
  
  string
  search_dom_for_string_field(const rj::Document&, const string);

  int
  search_dom_for_int_field(const rj::Document&, const string);
} // namespace a60
#endif
