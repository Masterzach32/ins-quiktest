#!/bin/bash

red=$'\e[1;31m'
yellow=$'\e[33m'
end=$'\e[0m'

if [ ! -f .project ] # working dir is not in project
then
    echo "$0: must be run from within ins-quiktest project directory"
    exit
fi
if [ ! -f .timestamp ] # file created by master
then
    echo "$0: can only be executed by master device"
    exit
fi
if [ -f .running ] # file created by slave
then
    echo "$0: already running on this machine; only one instance allowed"
    exit
fi

touch .running
source global.conf
source local.defaults
if [ -f local.conf ]; then
    source local.conf
else
    printf "$yellow%-10s%s\n$end" "[${COLORS[$1]}]" \
        "Warning: no local config provided, using local.defaults"
fi

TIMESTAMP=$(cat .timestamp)
rm .timestamp

folder=data/${COLORS[$1]}-$TIMESTAMP
mkdir -p $folder >/dev/null 2>/dev/null
make all >/dev/null 2>/dev/null

################################################################################

printf "%-10s%s\n" "[${COLORS[$1]}]" \
    "COM port baudrates: [${BPS_COM1[$1]}, ${BPS_COM2[$1]}, ${BPS_COM3[$1]}]"

if [ ${BPS_COM1[$1]} -gt 0 ]; then
    portname=$COM1
    baudrate=${BPS_COM1[$1]}
    stty -F /dev/$portname $baudrate 2>/dev/null
    serialno="$(app/ldprm /dev/$portname --name \
              --baud ${BPS_COM1[$1]} --rate 200 --init 7 \
              --angles 0 0 0 --lever ${LX[$1]} ${LY[$1]} ${LZ[$1]} 2>/dev/null)"
    if [ -z "$serialno" ]; then
        printf "$red%-10s%s\n$end" "[${COLORS[$1]}]" "Error: failed to connect to INS"
        rm -rf data/ .running
        exit
    fi
    echo $serialno > $folder/.serial
    printf "%-10s%s\n" "[${COLORS[$1]}]" "Connected to device S/N $serialno"
    printf "%-10s%s\n" "[${COLORS[$1]}]" \
        "Loaded parameters: IMU-antenna offset [${LX[$1]}, ${LY[$1]}, ${LZ[$1]}]"
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
