#!/bin/bash

# slave.sh

# the following blocks check for the existence of .project and .timestamp;
# the .project file is found in the project directory */ins-quiktest/, and
# all scripts depend on being run from this directory. therefore the slave
# script will not run if .project is not present.
if [[ ! -f .project ]] # working dir is not in project
then
    echo "$0: must be run from within ins-quiktest project directory"
    exit 1
fi

# dotfiles, source variables from global and local configurations
source config/global.conf
if [[ -f config/${LOGIN[$1]} ]]; then
    source config/${LOGIN[$1]}
else
    printf "$red%-${SP}s%s$end\n" "[${COLORS[$1]}]" \
        "No local config provided"
    exit 1
fi

# get the project dir and timestamp
PROJECT_DIR=$(pwd)
TIMESTAMP=$(cat .timestamp)
rm .timestamp

# make data folder and compile executables if they're dated
folder=data/${COLORS[$1]}-$TIMESTAMP
mkdir -p $folder >/dev/null 2>/dev/null
make all >/dev/null 2>/dev/null

################################################################################

printf "%-${SP}s%s\n" "[${COLORS[$1]}]" \
    "COM port baudrates: [${BPS_COM1[$1]}, ${BPS_COM2[$1]}, ${BPS_COM3[$1]}]"

serialno="INS"

# INS parameters are read and written over COM1
if [[ ${BPS_COM1[$1]} -gt 0 ]]
then
    portname=$COM1 # $COM1 is the local variable defined in local.conf
    if [[ ${RS422[$1]} -gt 0 ]]; then portname=$COM1_RS422; fi
    baudrate=${BPS_COM1[$1]}
    stty -F /dev/$portname $baudrate 2>/dev/null
    if [[ ! -e /dev/$portname ]]
    then
        printf "$red%-${SP}s%s$end\n" "[${COLORS[$1]}]" \
            "/dev/$portname does not exist or is inaccessible"
        rm -rf data/
        exit 1
    fi

    # app/ldprm is guaranteed to only print the device serial number to stdout
    # when the --name flag is provided; the rest of the parameters are loaded
    # silently. --rate sets the update frequency; --init sets the initial 
    # alignment time, in seconds; --angles sets the alignment angles; --lever
    # sets the IMU-to-antenna offset.
    #
    # the source file for this app can be found in src/ldprm.c;
    # type 'app/ldprm --usage' for usage information.
    serialno="$(app/ldprm /dev/$portname --name \
              --rate 200 --init 5 --angles 0 0 0 \
              --lever ${LX[$1]} ${LY[$1]} ${LZ[$1]} \
              --baud ${BPS_COM1[$1]} 2>/dev/null)"

    # if the serial number is empty, this indicates a failure to connect to
    # the INS; the script exits and returns an error
    if [[ -z "$serialno" ]]; then
        printf "$red%-${SP}s%s$end\n" "[${COLORS[$1]}]" \
            "Failed to establish INS connection"
        rm -rf data/
        exit 1
    fi
    # serial number is stored in dotfile, .serial
    echo $serialno > $folder/.serial
    printf "%-${SP}s%s\n" "[${COLORS[$1]}]" "Connected to device S/N $serialno"
    printf "%-${SP}s%s\n" "[${COLORS[$1]}]" \
        "Loaded parameters: IMU-antenna offset [${LX[$1]}, ${LY[$1]}, ${LZ[$1]}]"
    filename=$serialno-$TIMESTAMP\.bin
    sleep 2
    if [[ -n ${CMD_COM1[$1]} ]]
    then
        printf "%-${SP}s%s\n" "[${COLORS[$1]}]" \
            "Sending cmd/${CMD_COM1[$1]} over COM1"
    fi
    # start data stream over COM1
    app/str2str -in serial://$portname:$baudrate \
        -out file://./$folder/$filename \
        -c cmd/${CMD_COM1[$1]} 2>/dev/null &
    sleep 1
fi

# similarly for COM2; port is defined in config/*.local, baudrate in global.conf;
# data stream established using str2str
if [[ ${BPS_COM2[$1]} -gt 0 ]]; then
    portname=$COM2
    if [[ ! -e /dev/$portname ]]
    then
        printf "$red%-${SP}s%s$end\n" "[${COLORS[$1]}]" \
            "/dev/$portname does not exist or is inaccessible"
        rm -rf data/
        exit 1
    fi
    baudrate=${BPS_COM2[$1]}
    filename=$serialno-$TIMESTAMP\.gps
    if [[ -n ${CMD_COM2[$1]} ]]
    then
        printf "%-${SP}s%s\n" "[${COLORS[$1]}]" \
            "Sending cmd/${CMD_COM2[$1]} over COM2"
    fi
    stty -F /dev/$portname $baudrate 2>/dev/null
    ./app/str2str -in serial://$portname:$baudrate \
                  -out file://./$folder/$filename \
                  -c cmd/${CMD_COM2[$1]} 2>/dev/null &
fi

# all data streams are detached from the current shell with the &
# operator, so when this script exits they will still run, until
# the master device runs 'killall str2str'
if [[ ${BPS_COM3[$1]} -gt 0 ]]; then
    portname=$COM3
    if [[ ! -e /dev/$portname ]]
    then
        printf "$red%-${SP}s%s$end\n" "[${COLORS[$1]}]" \
            "/dev/$portname does not exist or is inaccessible"
        rm -rf data/
        exit 1
    fi
    baudrate=${BPS_COM3[$1]}
    stty -F /dev/$portname $baudrate 2>/dev/null
    ./app/str2str \
        -in ntrip://inertial:sensor22@us.inertiallabs.com:33101/roof \
        -out file://./$folder/RTCM3.bin \
        -out serial://$portname:$baudrate 2>/dev/null &
fi

sleep 2
