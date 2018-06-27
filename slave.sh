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
        serialno="F???????"
        rm -rf $folder
        exit
    fi
    printf "%-10s%s\n" "[${COLORS[$1]}]" "Connected to device S/N $serialno"
    printf "%-10s%s\n" "[${COLORS[$1]}]" \
        "COM port baudrates: [${BPS_COM1[$1]}, ${BPS_COM2[$1]}, ${BPS_COM3[$1]}]"
    printf "%-10s%s\n" "[${COLORS[$1]}]" "Loaded parameters: IMU-ANT-OFF ${LX[$1]}, $LY, $LZ"
    filename=$serialno-$TIMESTAMP\.bin
    sleep 3
    ./app/str2str -in serial://$portname:$baudrate \
                  -out file://./$folder/$filename \
                  -c cmd/INS_OPVT2AHR.cmd 2>/dev/null &
fi

if [ ${BPS_COM2[$1]} -gt 0 ]; then
    portname=$COM2
    baudrate=${BPS_COM2[$1]}
    filename=$serialno-$TIMESTAMP\.gps
    stty -F /dev/$portname $baudrate 2>/dev/null
    ./app/str2str -in serial://$portname:$baudrate \
                  -out file://./$folder/$filename 2>/dev/null &
fi

if [ ${BPS_COM3[$1]} -gt 0 ]; then
    portname=$COM3
    baudrate=${BPS_COM3[$1]}
    stty -F /dev/$portname $baudrate 2>/dev/null
    ./app/str2str \
        -in ntrip://inertial:sensor22@us.inertiallabs.com:33101/roof \
        -out serial:://$portname:$baudrate 2>/dev/null &
fi
