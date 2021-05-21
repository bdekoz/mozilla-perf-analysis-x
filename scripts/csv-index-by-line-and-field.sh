#!/usr/bin/env bash

FILE=$1
LINE=$2;
FIELD=$3;

# Get total line count.
LC=`cat -n ${FILE} | tail -n 1 | cut -f1`


function get_field_at_line_and_index
{
    LINEN=$1;
    FIELDN=$2;
    VAL=`cat ${FILE} | sed -n ${LINEN}p | cut -d "," -f ${FIELDN}`
    echo "$VAL";
}

function get_all_fields
{
    for ((i=1;i<=${LC}; i++))
    do
	LINEN=$i
	FIELDS=(1 2 3 4)
	for FIELDN in "${FIELDS[@]}"
	do
	    RVAL=$(get_field_at_line_and_index $LINEN $FIELDN)
	    echo "$RVAL"
	done
    done
}

#echo $(get_all_fields)
echo $(get_field_at_line_and_index $LINE $FIELD)
