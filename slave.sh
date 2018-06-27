#!/bin/bash

source global.conf
source local.defaults
if [ -f local.conf ]; then
    source local.conf
fi

TIMESTAMP="$(cat .timestamp)"
rm .timestamp

folder=data/${COLORS[$1]}-"$TIMESTAMP"
mkdir -p $folder >/dev/null 2>/dev/null
make all >/dev/null 2>/dev/null

################################################################################

if [ ${BPS_COM1[$1]} -gt 0 ]; then
    portname=$COM1
    baudrate=${BPS_COM1[$1]}
    stty -F /dev/$portname $baudrate 2>/dev/null
    serialno="$(app/ldprm /dev/$portname --name \
              --baud ${BPS_COM1[$1]} --rate 200 --init 7 \
              --angles 0 0 0 --lever ${LX[$1]} $LY $LZ)"
    if [ -z "$serialno" ]; then
        printf "%-10s%s\n" "[${COLORS[$1]}]" "Error: failed to load INS parameters"
        rm -rf $folder
        exit
    fi
    filename=$serialno-$TIMESTAMP\.bin
    sleep 3
    ./app/str2str -in serial://$portname:$baudrate \
                  -out file://./$folder/$filename \
                  -c cmd/INS_OPVT2AHR.cmd &
fi

if [ ${BPS_COM2[$1]} -gt 0 ]; then
    portname=$COM2
    baudrate=${BPS_COM2[$1]}
    filename=$serialno-$TIMESTAMP\.gps
    stty -F /dev/$portname $baudrate 2>/dev/null
    ./app/str2str -in serial://$portname:$baudrate \
                  -out file://./$folder/$filename &
fi

if [ ${BPS_COM3[$1]} -gt 0 ]; then
    portname=$COM3
    baudrate=${BPS_COM3[$1]}
    stty -F /dev/$portname $baudrate 2>/dev/null
    ./app/str2str \
        -in ntrip://inertial:sensor22@us.inertiallabs.com:33101/roof \
        -out serial:://$portname:$baudrate &
fi
