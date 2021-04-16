// telemetry sunburst / RAIL forms -*- mode: C++ -*-

// Copyright (c) 2018-2021, Mozilla
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

#include "moz-perf-x-radial.h"


namespace moz {

std::string
usage()
{
  std::string s("usage: moz-telemetry-x-extract.exe data.json (names.txt)");
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

  constexpr const char* browserscripts = "browserScripts";
  constexpr const char* statistics = "statistics";
  constexpr const char* timings = "timings";
  constexpr const char* vendor = "vendor";
  constexpr const char* pscalars = "privileged_telemetry_scalars";
  constexpr const char* phistograms = "privileged_telemetry_histograms";
  constexpr const char* penvironment = "privileged_telemetry_environment";
}


strings
remove_matches(const strings& total, const strings& found)
{
  // Update accounting of found, to-find.
  strings remaining;

  if (found.empty())
    remaining = total;
  else
    {
      std::set_difference(total.begin(), total.end(),
			  found.begin(), found.end(),
			  std::inserter(remaining, remaining.begin()));

      std::clog << "probes total: " << total.size() << std::endl;
      std::clog << "probes found: " << found.size() << std::endl;
      std::clog << "probes remaining: " << remaining.size() << std::endl;
      std::clog << std::endl;
    }

  return remaining;
}


void
update_matches(const strings& nfound, strings& tremain, strings& tfound)
{
  std::copy(nfound.begin(), nfound.end(), std::back_inserter(tfound));
  tremain = remove_matches(tremain, nfound);
}


void
extract_histogram_nodes(const rj::Value& dnode, strings& found, strings& remain,
		       ostream& ofs, histogram_view_t hvw)
{
  const strings& dfound = extract_histogram_fields(dnode, remain, ofs, hvw);
  update_matches(dfound, remain, found);
}


void
extract_scalar_nodes(const rj::Value& dnode, strings& found, strings& remain,
		       ostream& ofs)
{
  const strings& dfound = extract_scalar_fields(dnode, remain, ofs);
  update_matches(dfound, remain, found);
}



// Histogram node and sub-nodes.
void
extract_histograms_mozilla(const rj::Value& dhisto,
			   strings& found, strings& remain,
			   ostream& ofs, histogram_view_t hvw)
{
  // Extract histogram values.
  extract_histogram_nodes(dhisto, found, remain, ofs, hvw);

  if (dhisto.HasMember(k::content))
    {
      std::clog << k::content << std::endl;
      const rj::Value& dcontent = dhisto[k::content];
      extract_histogram_nodes(dcontent, found, remain, ofs, hvw);
    }

  if (dhisto.HasMember(k::parent))
    {
      std::clog << k::parent << std::endl;
      const rj::Value& dparent = dhisto[k::parent];
      extract_histogram_nodes(dparent, found, remain, ofs, hvw);
    }

  if (dhisto.HasMember(k::extension))
    {
      std::clog << k::extension << std::endl;
      const rj::Value& dext = dhisto[k::extension];
      extract_histogram_nodes(dext, found, remain, ofs, hvw);
    }

  if (dhisto.HasMember(k::dynamic))
    {
      std::clog << k::dynamic << std::endl;
      const rj::Value& ddyn = dhisto[k::dynamic];
      extract_histogram_nodes(ddyn, found, remain, ofs, hvw);
    }

  if (dhisto.HasMember(k::gpu))
    {
      std::clog << k::gpu << std::endl;
      const rj::Value& dgpu = dhisto[k::gpu];
      extract_histogram_nodes(dgpu, found, remain, ofs, hvw);
    }

  if (dhisto.HasMember(k::socket))
    {
      std::clog << k::socket << std::endl;
      const rj::Value& dsocket = dhisto[k::socket];
      extract_histogram_nodes(dsocket, found, remain, ofs, hvw);
    }
}


// Scalar node and sub-nodes.
void
extract_scalars_mozilla(const rj::Value& dscal,
			strings& found, strings& remain, ostream& ofs)
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
			  strings& remain, ostream& ofs, auto fn)
{
  const bool is_array(vnode.IsArray());
  const bool is_object(vnode.IsObject());
  const bool is_string(vnode.IsString());

  if (is_object)
    {
      fn(vnode, found, remain, ofs);
    }

  if (is_string)
    {
      std::string stringified = vnode.GetString();
      rj::Document d = parse_stringified_json_to_dom(stringified);

      if (d.IsObject())
	fn(d, found, remain, ofs);
    }

  if (!is_object && !is_string)
    {
      std::clog << "snapshot format failure: input isn't object, string."
		<< "Is it an array? " << is_array << std::endl;
      exit(123);
    }
}


void
extract_maybe_stringified(const rj::Value& vnode, strings& found,
			  strings& remain, ostream& ofs,
			  const histogram_view_t hvw, auto fn)
{
  const bool is_array(vnode.IsArray());
  const bool is_object(vnode.IsObject());
  const bool is_string(vnode.IsString());

  if (is_object)
    {
      fn(vnode, found, remain, ofs, hvw);
    }

  if (is_string)
    {
      std::string stringified = vnode.GetString();
      rj::Document d = parse_stringified_json_to_dom(stringified);

      if (d.IsObject())
	fn(d, found, remain, ofs, hvw);
    }

  if (!is_object && !is_string)
    {
      std::clog << "snapshot format failure: input isn't object, string."
		<< "Is it an array? " << is_array << std::endl;
      exit(124);
    }
}


/// Extract histograms, scalars, and environment info from glean-geckoview.
void
extract_mozilla_glean(string ifile)
{
  string ofname(file_path_to_stem(ifile) + "-x-" + "glean-telemetry");
  ofstream ofs(make_data_file(ofname, k::csv_ext));

  // Load input JSON data file into DOM.
  rj::Document dom(deserialize_json_to_dom(ifile));

  // Given the known details of the Mozilla android telemetry data schema,
  // save off the most used data nodes for speedier manipulation for
  // specific probes.
  const string kmetrics("metrics");
  const string ktiming("timing_distribution");

  // Get Histograms.
  if (dom.HasMember(kmetrics))
    {
      const rj::Value& dmetrics = dom[kmetrics.c_str()];
      const rj::Value& dtiming = dmetrics[ktiming.c_str()];

      // auto hwv = histogram_view_t::sum;
      list_object_fields(dtiming, ktiming, false);
      extract_histogram_fields(dtiming, ofs);
    }

  // Get environment
  const string kclient("client_info");
  const string kping("ping_info");
  if (dom.HasMember(kclient))
    {
      const rj::Value& dclient = dom[kclient.c_str()];

      environment env = { };
      env.os_name = field_value_to_string(dclient["os"]);
      env.os_version = field_value_to_string(dclient["os_version"]);

      string manu = field_value_to_string(dclient["device_manufacturer"]);
      string model = field_value_to_string(dclient["device_model"]);
      env.hw_name = manu + " " + model;

      env.sw_name = "Firefox Preview (Fenix)";
      env.sw_arch = field_value_to_string(dclient["architecture"]);
      env.sw_version = field_value_to_string(dclient["app_display_version"]);

      const rj::Value& dping = dom[kping.c_str()];
      env.date_time_stamp = field_value_to_string(dping["start_time"]);

      serialize_environment(env, ofname);
    }
}


/// Extract histograms, scalars, and environment info from snapshot node.
void
extract_mozilla_snapshot(const rj::Value& dvendor, string inames, string ifile)
{
  // Read probe names from input file, and put into vector<string>
  strings probes = deserialize_file_to_strings(inames);

  string ofname(file_path_to_stem(ifile) + "-x-" + "telemetry");
  std::ofstream ofs(make_data_file(ofname, k::csv_ext));

  strings found;
  strings remain(probes);

  if (dvendor.HasMember(k::phistograms))
    {
      std::clog << k::phistograms << " snapshot start" << std::endl;
      const rj::Value& dhisto = dvendor[k::phistograms];
      auto fn = extract_histograms_mozilla;
      const histogram_view_t hwv = histogram_view_t::median;
      extract_maybe_stringified(dhisto, found, remain, ofs, hwv, fn);
      std::clog << "histogram snapshot end" << std::endl << std::endl;
    }

  if (dvendor.HasMember(k::pscalars))
    {
      std::clog << k::pscalars << " snapshot start" << std::endl;
      const rj::Value& dscal = dvendor[k::pscalars];
      auto fn = extract_scalars_mozilla;
      extract_maybe_stringified(dscal, found, remain, ofs, fn);
      std::clog << "scalar snapshot end" << std::endl << std::endl;
    }

  if (dvendor.HasMember(k::penvironment))
    {
      std::clog << k::penvironment << " snapshot start" << std::endl;
      const rj::Value& denv = dvendor[k::penvironment];

      environment env = { };
      if (denv.IsString())
	{
	  std::string stringified = denv.GetString();
	  rj::Document d = parse_stringified_json_to_dom(stringified);
	  env = extract_environment_mozilla(d, true);
	}
      if (denv.IsObject())
	env = extract_environment_mozilla(denv, true);

      serialize_environment(env, ofname);
      std::clog << "environment snapshot end" << std::endl << std::endl;
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
extract_mozilla_android(const string ifile, const string inames)
{
  // Read probe names from input file, and put into vector<string>
  strings probes = deserialize_file_to_strings(inames);

  string ofname(file_path_to_stem(ifile) + "-x-" + file_path_to_stem(inames));
  std::ofstream ofs(make_data_file(ofname, k::csv_ext));

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
extract_mozilla_desktop(const string ifile, const string inames)
{
  // Read probe names from input file, and put into vector<string>
  strings probes = deserialize_file_to_strings(inames);

  string ofname(file_path_to_stem(ifile) + "-x-" + file_path_to_stem(inames));
  std::ofstream ofs(make_data_file(ofname, k::csv_ext));

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
				  ostream& ofs,
				  const histogram_view_t dview)
{
  if (v.IsObject())
    {
      for (vcmem_iterator i = v.MemberBegin(); i != v.MemberEnd(); ++i)
	{
	  string nname = i->name.GetString();
	  probes.push_back(nname);

	  const rj::Value& nv = i->value;
	  auto iv = extract_pseudo_histogram_field(nv, dview);

	  ofs << nname << "," << iv << std::endl;
	}
    }
}


// Extract metrics (as object nodes in JSON) from Browsertime format.
// Assuming the statistics/timing and statistics/visualMetrics nodes.
void
extract_browsertime_statistics(const rj::Value& v,
			       ostream& ofs, const histogram_view_t dview)
{
  strings found;

  const rj::Value& vt = v[k::timings];

  // Walk top level timings.

  // firstPaint
  // fullyLoaded
  const char* kfp = "firstPaint";
  if (vt.HasMember(kfp))
    {
      const rj::Value& dfp = vt[kfp];
      auto ifp = extract_pseudo_histogram_field(dfp, dview);
      ofs << kfp << "," << ifp << std::endl;
      found.push_back(kfp);
    }

  const char* kfl = "fullyLoaded";
  if (vt.HasMember(kfl))
    {
      const rj::Value& dfl = vt[kfl];
      auto ifl = extract_pseudo_histogram_field(dfl, dview);
      ofs << kfl << "," << ifl << std::endl;
      found.push_back(kfl);
    }

  const char* kttcp = "timeToContentfulPaint";
  if (vt.HasMember(kttcp))
    {
      const rj::Value& dttcp = vt[kttcp];
      auto ittcp = extract_pseudo_histogram_field(dttcp, dview);
      ofs << kttcp << "," << ittcp << std::endl;
      found.push_back(kttcp);
    }

  const char* kttdcf = "timeToDomContentFlushed";
  if (vt.HasMember(kttdcf))
    {
      const rj::Value& dttdcf = vt[kttdcf];
      auto ittdcf = extract_pseudo_histogram_field(dttdcf, dview);
      ofs << kttdcf << "," << ittdcf << std::endl;
      found.push_back(kttdcf);
    }

  const char* kttfi = "timeToFirstInteractive";
  if (vt.HasMember(kttfi))
    {
      const rj::Value& dttfi = vt[kttfi];
      auto ittfi = extract_pseudo_histogram_field(dttfi, dview);
      ofs << kttfi << "," << ittfi << std::endl;
      found.push_back(kttfi);
    }

  const char* krum = "rumSpeedIndex";
  if (vt.HasMember(krum))
    {
      const rj::Value& drum = vt[krum];
      auto irum = extract_pseudo_histogram_field(drum, dview);
      ofs << krum << "," << irum << std::endl;
      found.push_back(krum);
    }

  // Walk nested navigation timing.
  const char* knavt = "navigationTiming";
  if (vt.HasMember(knavt))
    {
      const rj::Value& dnavt = vt[knavt];
      extract_browsertime_nested_object(dnavt, found, ofs, dview);
    }

  // Walk nested page timings.
  const char* kpaget = "pageTimings";
  if (vt.HasMember(kpaget))
    {
      const rj::Value& dpaget = vt[kpaget];
      extract_browsertime_nested_object(dpaget, found, ofs, dview);
    }

  // Walk nested paint timings.
  const char* kpaint = "paintTimings";
  if (vt.HasMember(kpaint))
    {
      const rj::Value& dpaint = vt[kpaint];
      extract_browsertime_nested_object(dpaint, found, ofs, dview);
    }

  // Walk nested visual metrics.
  const char* kvizmet = "visualMetrics";
  if (v.HasMember(kvizmet))
    {
      const rj::Value& dvismet = v[kvizmet];
      extract_browsertime_nested_object(dvismet, found, ofs, dview);
    }

  std::clog << found.size() << " probes found: " << std::endl;
  for (const string& s : found)
    std::clog << s << std::endl;
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
extract_browsertime(string ifile, string inames, const histogram_view_t dview)
{
  // Setup output.
  string ofname(file_path_to_stem(ifile));
  if (!inames.empty())
    {
      ofname += "-x-";
      string ifname(file_path_to_stem(inames));
      ofname += ifname;
    }
  ofstream ofs(make_data_file(ofname, k::csv_ext));
  ostringstream oss;

  // Load input JSON data file into DOM.
  rj::Document dom(deserialize_json_to_dom(ifile));
  if (dom.HasParseError())
    {
      std::clog << "error: failed to parse JSON in " << ifile << std::endl;
      exit(12);
    }

  // Depending on the browsertime version, extraction varies.
  // Data is either an array of objects or just one object. If it is
  // an array, just use the first one.
  string browsertimev;
  rj::Value* bv = rj::Pointer("/info/browsertime/version").Get(dom);
  if (bv)
    {
      const rj::Value& v = *bv;
      browsertimev = field_value_to_string(v);
      std::clog << "start dom extract of browsertime version: "
		<< browsertimev << std::endl << std::endl;
    }

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
	  extract_browsertime_statistics(v, oss, dview);

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
      bool statisticsp = false;
      bool vendorp = false;
      bool browserscriptsp = false;

      std::clog << "dom array size " << dom.Size() << std::endl << std::endl;
      for (uint i = 0; i < dom.Size(); ++i)
	{
	  const rj::Value& v = dom[i];
	  auto nfields = list_object_fields(v, "", false, true);

	  if (v.IsObject() && nfields > 0)
	    {
	      if (v.HasMember(k::statistics))
		{
		  // Browsertime metrics.
		  statisticsp = true;
		  const rj::Value& vs = v[k::statistics];
		  extract_browsertime_statistics(vs, oss, dview);

		  // Extract and serialize environmental metadata, then stop.
		  environment env = extract_environment_browsertime(v);
		  serialize_environment(env, ofname);
		}

	      if (v.HasMember(k::browserscripts))
		{
		  browserscriptsp = true;
		  const rj::Value& vscripts = v[k::browserscripts];
		  if (vscripts.IsArray())
		    {
		      for (uint j = 0; j < vscripts.Size(); ++j)
			{
			  const rj::Value& vssub = vscripts[j];
			  if (vssub.HasMember(k::vendor))
			    {
			      vendorp = true;
			      const rj::Value& vendor = vssub[k::vendor];
			      if (list_object_fields(vendor, "", false) > 0)
				extract_mozilla_snapshot(vendor, inames, ifile);
			    }
			}
		    }
		}
	    }
	}

      auto not_found_alert = [](const string& s, const bool b)
      {
	if (b)
	  std::cerr << "dom node: " << s << " not found" << std::endl;
      };

      std::clog << std::endl;
      not_found_alert(k::statistics, statisticsp);
      not_found_alert(k::browserscripts, browserscriptsp);
      not_found_alert(k::vendor, vendorp);
    }

  std::clog << std::endl << "end dom extract" << std::endl;


  svg::value_type v(0);
  istringstream iss(oss.str());
  id_value_umap actual = deserialize_id_value_map(iss, v);

  // Read probe/metric names from input file, and put into vector<string>
  strings ids = deserialize_file_to_strings(inames);
  if (!ids.empty())
    {
      // Do edit list only.
      for (const string& metric : ids)
	{
	  if (actual.count(metric) == 1)
	    {
	      auto itr = actual.find(metric);
	      auto [ id, v] = *itr;
	      ofs << id << "," << v << std::endl;
	    }
	}
    }
  else
    {
      // Extract all.
      for (const auto& idv : actual)
	{
	  auto [ id, v] = idv;
	  ofs << id << "," << v << std::endl;
	}
    }
}


/**

Parse the log bits that look like this:

Fenix
[2020-07-21 22:11:47] INFO: [browsertime] https://cnn.com/ampstories/us/why-hurricane-michael-is-a-monster-unlike-any-other TTFB: 419ms (±37.43ms), firstPaint: 1.12s (±70.13ms), firstVisualChange: 2.27s (±53.92ms), DOMContentLoaded: 529ms (±39.98ms), Load: 1.47s (±64.08ms), speedIndex: 2.29s (±52.76ms), perceptualSpeedIndex: 2.29s (±52.78ms), contentfulSpeedIndex: 2.27s (±53.91ms), visualComplete85: 2.29s (±52.93ms), lastVisualChange: 2.75s (±52.84ms) (10 runs)

Chrome
[2020-07-21 21:15:13] INFO: [browsertime] https://cnn.com/ampstories/us/why-hurricane-michael-is-a-monster-unlike-any-other 28 requests, TTFB: 357ms (±103.34ms), firstPaint: 1.84s (±95.03ms), firstVisualChange: 2.14s (±91.56ms), FCP: 2.16s (±92.83ms), DOMContentLoaded: 510ms (±104.30ms), LCP: 1.78s (±108.67ms), CLS: 0 (±0.00), Load: 1.42s (±211.46ms), speedIndex: 2.25s (±89.90ms), perceptualSpeedIndex: 2.24s (±89.95ms), contentfulSpeedIndex: 2.14s (±93.25ms), visualComplete85: 2.26s (±91.85ms), lastVisualChange: 2.26s (±91.85ms) (10 runs)

Pass these logs to 3-field CSV of form (metric,time in ms, variance in ms) as:
TTFB,357,103.34

logfile = input browsertime log file
inames = input file of probe names to find in log file, if none extract all
iterations = number of browsertime interations in log file
 */
void
extract_browsertime_log(const string logfile, const string inames,
			const uint iterations = 10)
{
  // Do edit list.
  // Read probe names from input file, and put into vector<string>
  strings probes = deserialize_file_to_strings(inames);
  const auto probends = probes.end();

  std::ostringstream ostrs;
  std::ifstream ifs(logfile);
  if (ifs.good())
    {
      ostrs << ifs.rdbuf();
      string raw = ostrs.str();

      // Find summary results block, save it off for detailed parsing.
      string resultsblock;
      auto lastttfbpos = raw.rfind("TTFB: ");
      if (lastttfbpos != string::npos)
	{
	  const string runsstr(" (" + to_string(iterations) + " runs)");
	  auto lastiterpos = raw.rfind(runsstr);
	  if (lastiterpos != string::npos)
	    {
	      resultsblock = raw.substr(lastttfbpos, lastiterpos - lastttfbpos);
	      std::cout << "results block raw: " << std::endl;
	      std::cout << resultsblock << std::endl << std::endl;
	    }
	}
      else
	{
	  string m("extract_browsertime_log::error cannot find results block");
	  throw std::runtime_error(m);
	}

      // Parse summary results block.
      ostrs.str(string());
      std::istringstream istrs(resultsblock);
      do
	{
	  string pname;
	  getline(istrs, pname, ':');
	  std::cout << pname << std::endl;

	  const bool foundp = find(probes.begin(), probends, pname) != probends;
	  if (istrs.good() && (foundp || probes.empty()))
	    {
	      double pvalue(0);
	      istrs >> pvalue;

	      if (pvalue != 0)
		{
		  string timeunit;
		  istrs >> timeunit;
		  if (timeunit == "s")
		    pvalue *= 1000;
		}

	      // Extract variance part as one blob: (±103.34ms),
	      string varpre;
	      istrs >> varpre;

	      double variance(0);
	      if (pvalue != 0)
		{
		  std::istringstream ivar(varpre);
		  if (ivar.good())
		    {
		      char c1;
		      ivar >> c1;
		      char c2;
		      ivar >> c2;
		      char c3;
		      ivar >> c3;
		      ivar >> variance;

		      string timeunitv;
		      ivar >> timeunitv;
		      if (timeunitv == "s")
			variance *= 1000;
		    }
		}

	      // Output as csv.
	      ostrs << pname << k::comma << pvalue << k::comma << variance
		    << std::endl;
	    }
	  else
	    {
	      istrs.ignore(20, k::comma);
	    }

	  // Extract space after comma and before next probe name.
	  istrs.ignore(1);
	}
      while (istrs.good());


      // Results as csv.
      const string processed(ostrs.str());
      if (!processed.empty())
	{
	  string oname = logfile.substr(0, logfile.size() - 4) + k::csv_ext;
	  std::cout << "output filename: " << oname << std::endl << std::endl;
	  std::cout << "summary block processed as: " << std::endl;
	  std::cout << processed << std::endl
		    << std::endl;

	  std::ofstream ofs(oname);
	  ofs << ostrs.str();
	}
      else
	std::cout << "no probes found" << std::endl;

    }
 }


void
extract_identifiers(string idata, string inames, const json_t schema)
{
  if (schema == json_t::browsertime)
    extract_browsertime(idata, inames, histogram_view_t::median);
  if (schema == json_t::browsertime_log)
    extract_browsertime_log(idata, inames);
  if (schema == json_t::mozilla_desktop)
    extract_mozilla_desktop(idata, inames);
  if (schema == json_t::mozilla_android)
    extract_mozilla_android(idata, inames);
  if (schema == json_t::mozilla_glean)
    extract_mozilla_glean(idata);
}
} // namespace moz


int main(int argc, char* argv[])
{
  using namespace rapidjson;
  using namespace moz;

  // Sanity check.
  if (argc < 2 || argc > 3)
    {
      std::cerr << usage() << std::endl;
      return 1;
    }

  // Input match names file, input JSON data file
  std::string idata = argv[1];

  std::string inames;
  if (argc == 3)
    inames = argv[2];

  std::clog << "starting with " << argc - 1 << " input file(s): " << std::endl
	    << idata << std::endl << inames << std::endl;
  std::clog << std::endl;

  // Extract data/values from json.
  // This is useful for generating a list of Histograms and Scalar probe names.
  //list_json_fields(idata, 0);
  //list_json_fields(idata, 1);

  //extract_identifiers(idata, inames, json_t::browsertime_log);
  extract_identifiers(idata, inames, json_t::browsertime);
  //  extract_identifiers(idata, inames, json_t::har);

  return 0;
}
