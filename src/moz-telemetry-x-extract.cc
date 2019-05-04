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


namespace constants {

  constexpr const char* content = "content";
  constexpr const char* parent = "parent";

  constexpr const char* process = "processes";
  constexpr const char* dynamic = "dynamic";
  constexpr const char* extension = "extension";
  constexpr const char* gpu = "gpu";
  constexpr const char* socket = "socket";

  constexpr const char* vendor = "vendor";
  constexpr const char* pscalars = "privileged_telemetry_scalars";
  constexpr const char* phistograms = "privileged_telemetry_histograms";
  constexpr const char* penvironment = "privileged_telemetry_environment";
}


std::ofstream
make_extracted_data_file(string fstem)
{
  // Prepare output file.
  const string ofile(fstem + k::csv_ext);
  std::ofstream ofs(ofile);
  if (!ofs.good())
    std::cerr << k::errorprefix << "cannot open output file "
	      << ofile << std::endl;
  return ofs;
}


strings
remove_matches(const strings& total, const strings& found)
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
update_matches(const strings& nfound, strings& tremain, strings& tfound)
{
  std::copy(nfound.begin(), nfound.end(), std::back_inserter(tfound));
  tremain = remove_matches(tremain, nfound);
}


void
extract_histogram_nodes(const rj::Value& dnode, strings& found, strings& remain,
		       std::ofstream& ofs, histogram_view_t hvw)
{
  const strings& dfound = extract_histogram_fields(dnode, remain, ofs, hvw);
  update_matches(dfound, remain, found);
}


void
extract_scalar_nodes(const rj::Value& dnode, strings& found, strings& remain,
		       std::ofstream& ofs)
{
  const strings& dfound = extract_scalar_fields(dnode, remain, ofs);
  update_matches(dfound, remain, found);
}



// Histogram node and sub-nodes.
void
extract_histograms_mozilla(const rj::Value& dhisto,
			   strings& found, strings& remain,
			   std::ofstream& ofs, histogram_view_t hvw)
{
  // Extract histogram values.
  extract_histogram_nodes(dhisto, found, remain, ofs, hvw);

  if (dhisto.HasMember(k::content))
    {
      const rj::Value& dcontent = dhisto[k::content];
      extract_histogram_nodes(dcontent, found, remain, ofs, hvw);
    }

  if (dhisto.HasMember(k::parent))
    {
      const rj::Value& dparent = dhisto[k::parent];
      extract_histogram_nodes(dparent, found, remain, ofs, hvw);
    }

  if (dhisto.HasMember(k::extension))
    {
      const rj::Value& dext = dhisto[k::extension];
      extract_histogram_nodes(dext, found, remain, ofs, hvw);
    }

  if (dhisto.HasMember(k::dynamic))
    {
      const rj::Value& ddyn = dhisto[k::dynamic];
      extract_histogram_nodes(ddyn, found, remain, ofs, hvw);
    }

  if (dhisto.HasMember(k::gpu))
    {
      const rj::Value& dgpu = dhisto[k::gpu];
      extract_histogram_nodes(dgpu, found, remain, ofs, hvw);
    }

  if (dhisto.HasMember(k::socket))
    {
      const rj::Value& dsocket = dhisto[k::socket];
      extract_histogram_nodes(dsocket, found, remain, ofs, hvw);
    }
}


// Scalar node and sub-nodes.
void
extract_scalars_mozilla(const rj::Value& dscal,
			strings& found, strings& remain, std::ofstream& ofs)
{
  // Extract scalar values.
  extract_scalar_nodes(dscal, found, remain, ofs);

  if (dscal.HasMember(k::content))
    {
      const rj::Value& dcontent = dscal[k::content];
      extract_scalar_nodes(dcontent, found, remain, ofs);
    }

  if (dscal.HasMember(k::parent))
    {
      const rj::Value& dparent = dscal[k::parent];
      extract_scalar_nodes(dparent, found, remain, ofs);
    }
}


void
extract_maybe_stringified(const rj::Value& vnode, strings& found,
			  strings& remain, std::ofstream& ofs, auto fn)
{
  const bool is_array(vnode.IsArray());
  const bool is_object(vnode.IsObject());
  const bool is_string(vnode.IsString());

  if (is_object)
    {
      std::clog << "object:" << std::endl;
      fn(vnode, found, remain, ofs);
    }

  if (is_string)
    {
      std::clog << "string-ified: " << std::endl;
      std::string stringified = vnode.GetString();
      rj::Document d = parse_stringified_json_to_dom(stringified);

      if (d.IsObject())
	fn(d, found, remain, ofs);
    }

  if (!is_object && !is_string)
    {
      std::clog << "snapshot format failure: input isn't object, string."
		<< "Is it an array? " << is_array << std::endl;
      exit (123);
    }
}


void
extract_maybe_stringified(const rj::Value& vnode, strings& found,
			  strings& remain, std::ofstream& ofs,
			  const histogram_view_t hvw, auto fn)
{
  const bool is_array(vnode.IsArray());
  const bool is_object(vnode.IsObject());
  const bool is_string(vnode.IsString());

  if (is_object)
    {
      std::clog << "object:" << std::endl;
      fn(vnode, found, remain, ofs, hvw);
    }

  if (is_string)
    {
      std::clog << "string-ified: " << std::endl;
      std::string stringified = vnode.GetString();
      rj::Document d = parse_stringified_json_to_dom(stringified);

      if (d.IsObject())
	fn(d, found, remain, ofs, hvw);
    }

  if (!is_object && !is_string)
    {
      std::clog << "snapshot format failure: input isn't object, string."
		<< "Is it an array? " << is_array << std::endl;
      exit (123);
    }
}


/// Extract histograms, scalars, and environment info from snapshot node.
void
extract_mozilla_snapshot(const rj::Value& dvendor, string inames, string ifile)
{
  // Read probe names from input file, and put into vector<string>
  strings probes = deserialize_text_to_strings(inames);

  string ofname(file_path_to_stem(ifile) + "-x-" + "telemetry");
  std::ofstream ofs(make_extracted_data_file(ofname));

  strings found;
  strings remain(probes);

  if (dvendor.HasMember(k::phistograms))
    {
      std::clog << "histogram snapshot start" << std::endl;
      const rj::Value& dhisto = dvendor[k::phistograms];
      auto fn = extract_histograms_mozilla;
      const histogram_view_t hwv = histogram_view_t::sum;
      extract_maybe_stringified(dhisto, found, remain, ofs, hwv, fn);
      std::clog << "histogram snapshot end" << std::endl;
    }

  if (dvendor.HasMember(k::pscalars))
    {
      std::clog << "scalar snapshot start" << std::endl;
      const rj::Value& dscal = dvendor[k::pscalars];
      auto fn = extract_scalars_mozilla;
      extract_maybe_stringified(dscal, found, remain, ofs, fn);
      std::clog << "scalar snapshot end" << std::endl;
    }

  if (dvendor.HasMember(k::penvironment))
    {
      std::clog << "environment snapshot start" << std::endl;
      const rj::Value& denv = dvendor[k::penvironment];

      environment env = { };
      if (denv.IsString())
	{
	  std::clog << "string-ified: " << std::endl;
	  std::string stringified = denv.GetString();
	  rj::Document d = parse_stringified_json_to_dom(stringified);
	  env = extract_environment_mozilla(d, true);
	}
      if (denv.IsObject())
	env = extract_environment_mozilla(denv, true);

      serialize_environment(env, ofname);
      std::clog << "environment snapshot end" << std::endl;
    }
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
extract_mozilla_android(string inames, string ifile)
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

  strings found;
  strings remain(probes);
  if (dom.HasMember(kscalar.c_str()))
    {
      const rj::Value& ds = dom[kscalar.c_str()];
      extract_scalars_mozilla(ds, found, remain, ofs);
    }
  std::clog << "done scalar" << std::endl;

  if (dom.HasMember(kkeyedscalar.c_str()))
    {
      const rj::Value& dks = dom[kkeyedscalar.c_str()];
      extract_scalars_mozilla(dks, found, remain, ofs);
    }
  std::clog << "done keyed scalar" << std::endl;

  auto hwv = histogram_view_t::sum;
  if (dom.HasMember(khistogram.c_str()))
    {
      const rj::Value& dhisto = dom[khistogram.c_str()];
      extract_histograms_mozilla(dhisto, found, remain, ofs, hwv);
    }
  std::clog << "done histogram" << std::endl;

  if (dom.HasMember(kkeyedhistogram.c_str()))
    {
      const rj::Value& dkhisto = dom[kkeyedhistogram.c_str()];
      extract_histograms_mozilla(dkhisto, found, remain, ofs, hwv);
    }
  std::clog << "done keyed histogram" << std::endl;
}


/*
  Takes two arguments

  1. text file with probe names to extract from Mozilla telemetry.
  2. input telemetry main ping JSON file

  Output is a CSV file of probe names with extracted values

  Top-level fields for main ping:

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
extract_mozilla_main(string inames, string ifile)
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
      strings remain(probes);

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
      auto hvw = histogram_view_t::median;
      extract_histogram_nodes(dhisto, found, remain, ofs, hvw);
      extract_histogram_nodes(dcont, found, remain, ofs, hvw);
      extract_histogram_nodes(dgpu, found, remain, ofs, hvw);

      // Extract scalar values.
      // list_object_fields(dsimple);
      extract_scalar_nodes(dsimple, found, remain, ofs);
      extract_scalar_nodes(dparent, found, remain, ofs);

      // List remain.
      std::clog << std::endl;
      std::clog << remain.size() << " remain probes: " << std::endl;
      for (const string& s : remain)
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


// Extract objects from Browsertime format.
void
extract_browsertime_object(const rj::Value& v,
			   std::ofstream& ofs, const histogram_view_t dview)
{
  std::clog << "object probes search starting..." << std::endl;

  // Walk top level timings.
  strings found;

  // firstPaint
  // fullyLoaded
  const char* kfp = "firstPaint";
  if (v.HasMember(kfp))
    {
      const rj::Value& dfp = v[kfp];
      int ifp = extract_pseudo_histogram_field(dfp, dview);
      ofs << kfp << "," << ifp << std::endl;
      found.push_back(kfp);
    }

  const char* kfl = "fullyLoaded";
  if (v.HasMember(kfl))
    {
      const rj::Value& dfl = v[kfl];
      int ifl = extract_pseudo_histogram_field(dfl, dview);
      ofs << kfl << "," << ifl << std::endl;
      found.push_back(kfl);
    }

  // Walk navigation timing.
  const char* knavt = "navigationTiming";
  if (v.HasMember(knavt))
    {
      const rj::Value& dnavt = v[knavt];
      extract_browsertime_nested_object(dnavt, found, ofs, dview);
    }

  // Walk page timings.
  const char* kpaget = "pageTimings";
  if (v.HasMember(kpaget))
    {
      const rj::Value& dpaget = v[kpaget];
      extract_browsertime_nested_object(dpaget, found, ofs, dview);
    }

  std::clog << "object probes search found: " << found.size() << std::endl;
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
extract_browsertime(string inames, string ifile, const histogram_view_t dview)
{
  string ofname(file_path_to_stem(ifile) + "-x-" + "browsertime");
  std::ofstream ofs(make_extracted_data_file(ofname));

  // Load input JSON data file into DOM.
  rj::Document dom(deserialize_json_to_dom(ifile));

  // Data is either an array of objects or just one object. IF it is
  // an array, just use the first one.

  std::clog << "start dom extract" << std::endl << std::endl;

  // Older browsertime versions...
  if (dom.IsObject())
    {
      std::clog << "dom object " << std::endl << std::endl;
      list_dom_object_fields(dom, false, true);

      //statistics/timing
      //statistics/timing/navigationTiming
      //statistics/timing/pageTimings
      rj::Value* pv = rj::Pointer("/statistics/timings").Get(dom);
      if (pv)
	{
	  const rj::Value& v = *pv;
	  extract_browsertime_object(v, ofs, dview);

	  // Extract and serialize environmental metadata.
	  environment env = extract_environment_browsertime(dom);
	  serialize_environment(env, ofname);
	}
      else
	std::cerr << "dom object statistics/timings not found" << std::endl;
    }

  // Newer browsertime versions...
  if (dom.IsArray())
    {
      std::clog << "dom array size " << dom.Size() << std::endl << std::endl;

      for (uint i = 0; i < dom.Size(); ++i)
	{
	  const rj::Value& v = dom[i];
	  if (v.IsObject() && list_object_fields(v, "", false, true) > 0)
	    {
	      constexpr const char* statistics = "statistics";
	      if (v.HasMember(statistics))
		{
		std::cerr << "dom node " << statistics << std::endl;

		  // Browsertime metrics.
		  constexpr const char* timings = "timings";
		  const rj::Value& vs = v[statistics];
		  const rj::Value& vst = vs[timings];
		  extract_browsertime_object(vst, ofs, dview);

		  // Extract and serialize environmental metadata, then stop.
		  environment env = extract_environment_browsertime(v);
		  serialize_environment(env, ofname);
		}
	      else
		std::cerr << "no dom node " << statistics << std::endl;

	      constexpr const char* browserscripts = "browserScripts";
	      if (v.HasMember(browserscripts))
		{
		  std::cerr << "dom node " << browserscripts << std::endl;

		  const rj::Value& vscripts = v[browserscripts];
		  if (vscripts.IsArray())
		    {
		      for (uint j = 0; j < vscripts.Size(); ++j)
			{
			  const rj::Value& vssub = vscripts[j];
			  if (vssub.HasMember(k::vendor))
			    {
			      std::cerr << "dom node " << k::vendor << std::endl;
			      const rj::Value& vendor = vssub[k::vendor];
			      if (list_object_fields(vendor, "", false) > 0)
				extract_mozilla_snapshot(vendor, inames, ifile);
			    }
			  else
			    std::cerr << "no dom node " << k::vendor << std::endl;
			}
		    }
		}
	      else
		std::cerr << "no dom node " << browserscripts << std::endl;
	    }
	}
    }

  std::clog << std::endl << "end dom extract" << std::endl;
}


void
extract_identifiers(string inames, string idata)
{
  if constexpr (djson_t == json_t::browsertime)
    extract_browsertime(inames, idata, histogram_view_t::median);
  if constexpr (djson_t == json_t::mozilla_android)
    extract_mozilla_android(inames, idata);
  if constexpr (djson_t == json_t::mozilla_main)
    extract_mozilla_main(inames, idata);
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
  std::clog << "input files: " << std::endl
	    << inames << std::endl << idata << std::endl;
  std::clog << std::endl;

  // Extract data/values from json.
  // This is useful for generating a list of Histograms and Scalar probe names.

  //list_json_fields(idata, false);
  //list_json_fields(idata, true);

  extract_identifiers(inames, idata);

  return 0;
}
