// telemetry sunburst / RAIL forms -*- mode: C++ -*-

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

#include <chrono>
#include <iostream>
#include <algorithm>

#include "moz-json-basic.h"

namespace moz {

std::string
usage()
{
  std::string s("usage: moz-telemetry-x-extract.exe names.txt data.json");
  return s;
}


strings
find_remaining(const strings& total, const strings& found)
{
  // Update accounting of found, to-find.
  strings probes1r;
  std::set_difference(total.begin(), total.end(),
		      found.begin(), found.end(),
		      std::inserter(probes1r, probes1r.begin()));

  std::clog << std::endl;
  std::clog << "probes total: " << total.size() << std::endl;
  std::clog << "probes found: " << found.size() << std::endl;
  std::clog << "probes remaining: " << probes1r.size() << std::endl;
  std::clog << std::endl;

  return probes1r;
}


void
extract(const strings& found, strings& tremaining, strings& tfound)
{
  std::copy(found.begin(), found.end(), std::back_inserter(tfound));
  tremaining = find_remaining(tremaining, found);
}


std::ofstream
make_extracted_data_file(string fstem)
{
  // Prepare output file.
  const string ofile(fstem + k::csv_ext);
  std::ofstream ofs(ofile);
  if (ofs.good())
    {
      // Add zero as starting point.
      ofs << "START" << "," << 0 << std::endl;
    }
  else
    std::cerr << k::errorprefix << "cannot open output file "
	      << ofile << std::endl;
  return ofs;
}


/*
  Takes two arguments

  1. text file with probe names to extract from Mozilla telemetry.
  2. input telemetry main ping JSON file

  Output is a CSV file of probe names with extracted values

  Top-level fields:
  type
  id
  creationDate
  version
  application
  payload
  clientId
  environment
 */
void
extract_mozilla_names(string inames, string ifile)
{
  // Read probe names from input file, and put into vector<string>
  strings probes = deserialize_text_to_strings(inames);

  string ofname(file_path_to_stem(ifile) + "-x-" + file_path_to_stem(inames));
  std::ofstream ofs(make_extracted_data_file(ofname));

  // Load input JSON data file into DOM.
  rj::Document dom(deserialize_json_to_dom(ifile));

  // Given the known details of the Mozilla telemetry data schema,
  // save off the most used data nodes for speedier manipulation for
  // specific probes.
  const string kpayload("payload");
  if (dom.HasMember(kpayload.c_str()))
    {
      strings found;
      strings left(probes);

      // payload
      // payload/histograms
      // payload/simple
      const string khistograms("histograms");
      const string kscalars("scalars");
      const string ksimple("simpleMeasurements");
      const rj::Value& dpayload = dom[kpayload.c_str()];
      const rj::Value& dhisto = dpayload[khistograms.c_str()];
      const rj::Value& dsimple = dpayload[ksimple.c_str()];

      // payload/processes
      // payload/processes/content/histograms
      const string kproc("processes");
      const string kcontent("content");
      const rj::Value& dproc = dpayload[kproc.c_str()];
      const rj::Value& dcont = dproc[kcontent.c_str()][khistograms.c_str()];;

      // payload/processes/gpu
      const string kgpu("gpu");
      const rj::Value& dgpu = dproc[kgpu.c_str()][khistograms.c_str()];

      // payload/processes/parent/scalars
      const string kparent("parent");
      const rj::Value& dparent = dproc[kparent.c_str()][kscalars.c_str()];

      // Extract histogram values.
      // list_object_fields(dhistogram);
      extract(extract_histogram_fields(dhisto, left, ofs), left, found);
      extract(extract_histogram_fields(dcont, left, ofs), left, found);
      extract(extract_histogram_fields(dgpu, left, ofs), left, found);

      // Extract scalar values.
      // list_object_fields(dsimple);
      extract(extract_scalar_fields(dsimple, left, ofs), left, found);
      extract(extract_scalar_fields(dparent, left, ofs), left, found);

      // List remaining.
      std::clog << std::endl;
      std::clog << left.size() << " remaining probes: " << std::endl;
      for (const string& s : left)
	std::clog << '\t' << s << std::endl;
      std::clog << std::endl;

      // Extract and serialize environmental metadata.
      std::clog << "extracing environment metadata: ";
      environment env = extract_environment_mozilla(dom);
      serialize_environment(env, ofname);
      std::clog << "done" << std::endl;
    }
  else
    std::cerr << k::errorprefix << kpayload << " not found " << std::endl;
}


/*
  Takes two arguments

  1. text file with probe names to extract from Mozilla telemetry.
  2. input telemetry main ping JSON file

  Output is a CSV file of probe names with extracted values

  Top-level fields:
  scalars
  keyedScalars
  histograms
  keyedHistograms
 */
void
extract_mozilla_android_names(string inames, string ifile)
{
  // Read probe names from input file, and put into vector<string>
  strings probes = deserialize_text_to_strings(inames);

  string ofname(file_path_to_stem(ifile) + "-x-" + file_path_to_stem(inames));
  std::ofstream ofs(make_extracted_data_file(ofname));

  // Load input JSON data file into DOM.
  rj::Document dom(deserialize_json_to_dom(ifile));

  // Given the known details of the Mozilla android telemetry data schema,
  // save off the most used data nodes for speedier manipulation for
  // specific probes.
  const string kscalar("scalars");
  const string kkeyedscalar("keyedScalars");
  const string khistogram("histograms");
  const string kkeyedhistogram("keyedHistograms");

  const string kparent("parent");
  const string kdynamic("dynamic");
  const string ksocket("socket");
  const string kgpu("gpu");

  strings found;
  strings left(probes);
  if (dom.HasMember(kscalar.c_str()))
    {
      const rj::Value& ds = dom[kscalar.c_str()];
      extract(extract_scalar_fields(ds, left, ofs), left, found);
    }

  if (dom.HasMember(kkeyedscalar.c_str()))
    {
      const rj::Value& dks = dom[kkeyedscalar.c_str()];
      extract(extract_scalar_fields(dks, left, ofs), left, found);
    }

  if (dom.HasMember(khistogram.c_str()))
    {
      const rj::Value& dhi = dom[khistogram.c_str()];
      const rj::Value& dgpu = dhi[kgpu.c_str()];
      const rj::Value& dparent = dhi[kparent.c_str()];
      const rj::Value& ddynam = dhi[kdynamic.c_str()];
      const rj::Value& dsocket = dhi[ksocket.c_str()];

      extract(extract_histogram_fields(dhi, left, ofs), left, found);
      extract(extract_histogram_fields(dgpu, left, ofs), left, found);
      extract(extract_histogram_fields(dparent, left, ofs), left, found);
      extract(extract_histogram_fields(ddynam, left, ofs), left, found);
      extract(extract_histogram_fields(dsocket, left, ofs), left, found);
    }

  if (dom.HasMember(kkeyedhistogram.c_str()))
    {
      const rj::Value& dkhi = dom[kkeyedhistogram.c_str()];
      const rj::Value& dgpu = dkhi[kgpu.c_str()];
      const rj::Value& dparent = dkhi[kparent.c_str()];

      extract(extract_histogram_fields(dkhi, left, ofs), left, found);
      extract(extract_histogram_fields(dgpu, left, ofs), left, found);
      extract(extract_histogram_fields(dparent, left, ofs), left, found);
    }
}


// Extract nested objects from Browsertime format.
void
extract_browsertime_nested_object(const rj::Value& v, strings& probes,
				  std::ofstream& ofs,
				  const histogram_view_t dview)
{
  if (v.IsObject())
    {
      for (vcmem_iterator i = v.MemberBegin(); i != v.MemberEnd(); ++i)
	{
	  string nname = i->name.GetString();
	  probes.push_back(nname);

	  const rj::Value& nv = i->value;
	  int iv = extract_pseudo_histogram_field(nv, dview);

	  ofs << nname << "," << iv << std::endl;
	}
    }
}


// Extract nested objects from Browsertime format.
void
extract_browsertime_object(const rj::Value& v, strings& probes,
			   std::ofstream& ofs, const histogram_view_t dview)
{
  std::clog << "object probes search starting..." << std::endl;

  // Walk top level timings.
  // firstPaint
  // fullyLoaded
  const char* kfp = "firstPaint";
  if (v.HasMember(kfp))
    {
      const rj::Value& dfp = v[kfp];
      int ifp = extract_pseudo_histogram_field(dfp, dview);
      ofs << kfp << "," << ifp << std::endl;
      probes.push_back(kfp);
    }

  const char* kfl = "fullyLoaded";
  if (v.HasMember(kfl))
    {
      const rj::Value& dfl = v[kfl];
      int ifl = extract_pseudo_histogram_field(dfl, dview);
      ofs << kfl << "," << ifl << std::endl;
      probes.push_back(kfl);
    }

  // Walk navigation timing.
  const char* knavt = "navigationTiming";
  if (v.HasMember(knavt))
    {
      const rj::Value& dnavt = v[knavt];
      extract_browsertime_nested_object(dnavt, probes, ofs, dview);
    }

  // Walk page timings.
  const char* kpaget = "pageTimings";
  if (v.HasMember(kpaget))
    {
      const rj::Value& dpaget = v[kpaget];
      extract_browsertime_nested_object(dpaget, probes, ofs, dview);
    }

  std::clog << "object probes found: " << probes.size() << std::endl;
}



/*
  Takes a text file with probe names to extract from a browsertime JSON file.

  Top-level fields:

  info
  files
  timestamps
  browserScripts
  visualMetrics
  cpu
  fullyLoaded
  errors
  statistics

  Output is a CSV file of probe names with extracted values

  Browsertime
  https://www.sitespeed.io/documentation/browsertime/

  See the following for more detail

  Navigation Timing
  https://w3c.github.io/navigation-timing/

  User Timing
  https://www.w3.org/TR/user-timing/

  HAR 1.2
  http://www.softwareishard.com/blog/har-12-spec/
 */
void
extract_browsertime_names(string inames, string ifile,
			  const histogram_view_t dview)
{
  // Read probe names from input file, and put into vector<string>
  strings probes;

  string ofname(file_path_to_stem(ifile) + "-x-" + file_path_to_stem(inames));
  std::ofstream ofs(make_extracted_data_file(ofname));

  // Load input JSON data file into DOM.
  rj::Document dom(deserialize_json_to_dom(ifile));

  // Data is either an array of objects or just one object. IF it is
  // an array, just use the first one.

  std::clog << "start dom extract" << std::endl << std::endl;

  if (dom.IsObject())
    {
      std::clog << "dom object " << std::endl;
      list_dom_object_fields(dom, false, true);

      //statistics/timing
      //statistics/timing/navigationTiming
      //statistics/timing/pageTimings
      rj::Value* pv = rj::Pointer("/statistics/timings").Get(dom);
      if (pv)
	{
	  const rj::Value& v = *pv;
	  extract_browsertime_object(v, probes, ofs, dview);

	  // Extract and serialize environmental metadata.
	  environment env = extract_environment_browsertime(dom);
	  serialize_environment(env, ofname);
	}
      else
	std::cerr << "dom object statistics/timings not found" << std::endl;
    }

  if (dom.IsArray())
    {
      std::clog << "dom array size " << dom.Size() << std::endl;

      for (uint i = 0; i < dom.Size(); ++i)
	{
	  const rj::Value& v = dom[i];
	  if (v.IsObject() && list_object_fields("", v, false, true) > 0)
	    {
	      const char* kstat("statistics");
	      if (v.HasMember(kstat))
		{
		  const char* ktime("timings");
		  const rj::Value& vs = v[kstat];
		  const rj::Value& vst = vs[ktime];
		  extract_browsertime_object(vst, probes, ofs, dview);


		  // Extract and serialize environmental metadata.
		  std::clog << "extracing environment metadata: ";
		  environment env = extract_environment_browsertime(v);
		  serialize_environment(env, ofname);
		  std::clog << "done" << std::endl;
		  break;
		}
	      else
		std::cerr << "dom object statistics not found" << std::endl;
	    }
	}
    }

  std::clog << std::endl << "end dom extract" << std::endl;
}


void
extract_identifiers(string inames, string idata)
{
  if constexpr (djson_t == json_t::browsertime)
    extract_browsertime_names(inames, idata, histogram_view_t::median);
  if constexpr (djson_t == json_t::mozilla)
    extract_mozilla_names(inames, idata);
  if constexpr (djson_t == json_t::mozilla_android)
    extract_mozilla_android_names(inames, idata);
}
} // namespace moz


int main(int argc, char* argv[])
{
  using namespace rapidjson;
  using namespace moz;

  // Sanity check.
  if (argc != 3)
    {
      std::cerr << usage() << std::endl;
      return 1;
    }

  // Input match names file, input JSON data file

  std::string inames = argv[1];
  std::string idata = argv[2];
  std::clog << "input files: " << inames << " , " << idata << std::endl;
  std::clog << std::endl;

  // Extract data/values from json.
  // This is useful for generating a list of Histograms and Scalar probe names.
  // list_json_fields(idata, true);
  //list_json_fields(idata, false);

  extract_identifiers(inames, idata);

  return 0;
}
