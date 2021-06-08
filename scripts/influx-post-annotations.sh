#!/usr/bin/env bash

# Date argument is date in iso format, aka 2021-06-06.
# Second precision writes for the database are more than sufficient
# for something that is monitored on a daily basis...
#DATE=$1
DATE="2021-06-08"
if [ ! -n "$DATE" ]; then
    DATEST=`date +%s`
else
    # Assuming DATE as YYY.MM.DD
    DATEST=`date --date "${DATE} 12:00:01" +%s`
fi
DATEEND=`expr $DATEST + 1`

EVENT1="restart"
EVENT2="power"
EVENT3="device-administration"
EVENT4="update"
EVENT5="configuration-change"
ETITLE=$EVENT1

# Reminder about device names, product names
# samsung-galaxy-s10
# samsung-galaxy-s7
# pixel-4-xl
# pixel-2
# quint
# all-devices
# fenix-nightly
# chrome
EDETAIL="all-devices"


DATA="events title=\"${ETITLE}\",text=\"${EDETAIL}\",tags=\"\" ${DATEST}"
echo "$DATE ${DATA}"

# v1.8
#DB="http://localhost:8086/write?db=${DBI}&precision=s"
#curl -i -XPOST "${DB}"  --data-binary "${DATA}"

# v2.0
DBHOST="hilldale-b40313e5.influxcloud.net"
DBI=performance
DB="https://${DBHOST}:8086/api/v2/write?bucket=${DBI}&precision=s"
AUTH="Authorization: Token performance_wo:7169686c5ad9119c2557cae47919aff0"
ACCEPT="Accept: application/json"
CTYPE="Content-Type: application/json"
curl -i -XPOST "${DB}" --header "${AUTH}" --data-raw "${DATA}"
