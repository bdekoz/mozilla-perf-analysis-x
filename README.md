# mozilla-telemetry-x

These are C++ sources for creating experimental analyses of mozilla telemetry data. Together with input data in the form of Mozilla telemetry main ping JSON files, optional browsertime JSON files, and various highlight and marker/probe lists, these sources can render results in a variety of output views and formats.


**BUILDING**

Set the environmental variable MOZPERFAX to the location of the source directory.


Main executables are built from sources in the src directory, by running `scripts/compile.sh` like so
```
../scripts/compile-source.sh moz-telemetry-x-extract.cc
```

**EXECUTABLES**

```
moz-telemetry-x-extract.exe
moz-telemetry-x-analyze-radial.exe
moz-telemetry-x-analyze-ripple.exe
```


`moz-telemetry-x-extract.exe names.txt data.json`

Extract data from input JSON file into CSV file of *probe names* and timing *values*.


`moz-telemetry-x-analyze-radial.exe data.csv`

Extract data from input CSV file and render into visual form SVG


`moz-telemetry-x-analyze-radial.exe data1.csv data2.csv (edit.txt)`

Extract data from input CSV files and render into visual form SVG. The optional *edit.txt* file is used to hilight the probe names from the *data1.csv* file.


**SCRIPTS**

From a results directory and metric edit list to svg images for potential static site, radial visualizations
```
browsertime-to-uno-with-1-metric-list.sh (metric file)
  transform-1-metric-cosmology-to-uno.sh (toplevel product results for one day) (metric file)
    extract-metrics-from-log.sh
      copy-log-files-to-one-dir.sh (results dir)
      moz-perf-x-extract.browsertime_log.exe
    extract-metrics-from-json-to-csv.sh (results dir)
      copy-json-files-to-one-dir.sh
      moz-perf-x-extract.browsertime.exe
    moz-perf-x-analyze-radial-uno.exe (csv file) (metric cosmo)
    svg-dir-to-pngs.sh
```

From a results directory and metric edit list to grafana chart visualizations
```
browsertime-to-grafana-with-1-metric-list.sh DEVID PRODUCTID RDIR (metric file)
  transform-1-metric-cosmology-to-grafana.sh DEVID PRODUCTID RDIR (metric file)
    extract-metrics-from-json.sh (RD)
      copy-json-files-to-one-dir.sh
      moz-perf-x-extract.browsertime.exe (bt file) (metric file)
      moz-perf-x-extract.browsertime_url.exe (bt file)
      csv-index-by-line-and-field.sh FILE LINE FIELD
```
