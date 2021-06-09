#!/usr/bin/env bash

# NB: Assume MOZPERFAX set in environment

DEVICEID=$1
PRODUCTID=$2
RDIR=$3
METRICLIST=$4

#DATE=`date --iso`
DATE=2021-03-31
# Second precision writes for the database are more than sufficient
# for something that is monitored on a daily basis...
if [ ! -n "$DATE" ]; then
    DATEST=`date +%s`
else
    DATEST=`date --date "${DATE} 12:00:01" +%s`
fi

# Where to find necessary prequisites.
MOZXBDIR="${MOZPERFAX}/bin"
MOZXBROWSERTIME=$MOZXBDIR/moz-perf-x-extract.browsertime.exe
MOZXDOMAIN=$MOZXBDIR/moz-perf-x-extract.browsertime_url.exe

SCRIPTSDIR="${MOZPERFAX}/scripts"
SCRIPTCSVTON=$SCRIPTSDIR/csv-index-by-line-and-field.sh
SCRIPTDEVICE=$SCRIPTSDIR/common-devices.sh

# Convert DEVICEID into something rational and consistent for influx.
. ${SCRIPTDEVICE} $DEVICEID
#DEVICE=$fpm_dev_name
DEVICE=`echo ${fpm_dev_name} | sed 's/_/-/g'`

# Influx DB info
DBI=performance
DBHOST="hilldale-b40313e5.influxcloud.net"
DB="https://${DBHOST}:8086/api/v2/write?bucket=${DBI}&precision=s"
AUTH="Authorization: Token performance_wo:7169686c5ad9119c2557cae47919aff0"

# Prep: enter working directory, prep result data files.
cd $RDIR
$SCRIPTSDIR/copy-json-files-to-one-dir.sh

# Required for insertion.
# DEVICEID
# PRODUCTID
# DOMAIN
# METRIC
# VALUE
# VSTDDEV
# VMDEV
# DATE/TIMESTAMP
for file in json/browsertime*.json
do
    # Extract minimum domain information
    DOMAIN=`${MOZXDOMAIN} $file`

    # Make extracted csv file with only the specified metrics...
    $MOZXBROWSERTIME $file $METRICLIST

    # Find the generated file...
    csvfilestem=${file%.*}
    csvstem=${csvfilestem##*/}
    csvfile="./${csvstem}.?.csv"

    # Get total line count of the file...
    LC=`cat -n ${csvfile} | tail -n 1 | cut -f1`

    # Extract (LINEN, FIELDN) and insert
    for ((i=1;i<=${LC}; i++))
    do
	METRIC=`${SCRIPTCSVTON} "$csvfile" "$i" 1`
	VALUE=`${SCRIPTCSVTON} "$csvfile" "$i" 2`
	VSTDDEV=`${SCRIPTCSVTON} "$csvfile" "$i" 3`
	VMDEV=`${SCRIPTCSVTON} "$csvfile" "$i" 4`

	DATA="${METRIC},device=${DEVICE},product=${PRODUCTID},url=${DOMAIN} value=${VALUE},stddev=${VSTDDEV},mdev=${VMDEV} ${DATEST}"

	# v2.0
	echo "${DATA}"
	curl -i -XPOST "${DB}" --header "${AUTH}" --data-raw "${DATA}"
    done
done
