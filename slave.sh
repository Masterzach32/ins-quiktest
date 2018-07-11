#!/bin/bash

# definitions for console colors
red=$'\e[1;31m'
yellow=$'\e[33m'
end=$'\e[0m'

error_flag=0;

# the following blocks check for the existence of .project and .timestamp;
# the .project file is found in the project directory */ins-quiktest/, and
# all scripts depend on being run from this directory. therefore the slave
# script will not run if .project is not present.
# furthermore, .timestamp MUST be present, because this indicates that
# the master device has initiated the slave script. once the slave is
# started, the .running file is created, so as to prevent another slave
# instance from beginning.
if [ ! -f .project ] # working dir is not in project
then
    echo "$0: must be run from within ins-quiktest project directory"
    exit 1
fi
if [ ! -f .timestamp ] # file created by master
then
    echo "$0: can only be executed by master device"
    exit 1
fi
if [ -f .running ] # file created by slave
then
    echo "$0: already running on this machine; only one instance allowed"
    exit 1
fi

# dotfiles, source variables from global and local configurations
touch .running
source global.conf
source local.defaults

# load the local configuration machine-specific file, if it exists
if [ -f local.conf ]; then
    source local.conf
else
    # throw a soft warning about lack of local config file
    printf "$yellow%-${SP}s%s$end\n" "[${COLORS[$1]}]" \
        "Warning: no local config provided, using local.defaults"
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
if [ ${BPS_COM1[$1]} -gt 0 ]
then
    portname=$COM1 # $COM1 is the local variable defined in local.conf
    baudrate=${BPS_COM1[$1]}
    stty -F /dev/$portname $baudrate 2>/dev/null
    if [[ ! -e /dev/$portname ]]
    then
        printf "$red%-${SP}s%s$end\n" "[${COLORS[$1]}]" \
            "/dev/$portname does not exist or is inaccessible"
        rm -rf data/ .running
        exit 1
    fi

    # app/ldprm is guaranteed to only print the device serial number to stdout
    # when the --name flag is provided; the rest of the parameters are loaded
    # silently. --baud sets the COM1 baudrate; --rate sets the update frequency;
    # --init sets the initial alignment time, in seconds; --angles sets the
    # alignment angles; --lever sets the IMU-to-antenna offset.
    #
    # the source file for this app can be found in src/ldprm.c;
    # type 'app/ldprm --usage' for usage information.
    serialno="$(app/ldprm /dev/$portname --name \
              --baud ${BPS_COM1[$1]} --rate 200 --init 5 \
              --angles 0 0 0 --lever ${LX[$1]} ${LY[$1]} ${LZ[$1]} 2>/dev/null)"

    # if the serial number is empty, this indicates a failure to connect to
    # the INS; an errror will be thrown, and a file copied to the master error
    # directory, .error.d/
    if [ -z "$serialno" ]; then
        printf "$red%-${SP}s%s$end\n" "[${COLORS[$1]}]" \
            "Failed to establish INS connection"
        rm -rf data/ .running
        exit 1
    fi
    # serial number is stored in dotfile, .serial
    echo $serialno > $folder/.serial
    printf "%-${SP}s%s\n" "[${COLORS[$1]}]" "Connected to device S/N $serialno"
    printf "%-${SP}s%s\n" "[${COLORS[$1]}]" \
        "Loaded parameters: IMU-antenna offset [${LX[$1]}, ${LY[$1]}, ${LZ[$1]}]"
    filename=$serialno-$TIMESTAMP\.bin
    sleep 2
    # start data stream over COM1
    app/str2str -in serial://$portname:$baudrate \
        -out file://./$folder/$filename -c cmd/INS_OPVT2AHR.cmd 2>/dev/null &
    sleep 1
fi

# similarly for COM2; port is defined in local.conf, baudrate in global.conf;
# data stream established using str2str
if [ ${BPS_COM2[$1]} -gt 0 ]; then
    portname=$COM2
    if [[ ! -e /dev/$portname ]]
    then
        printf "$red%-${SP}s%s$end\n" "[${COLORS[$1]}]" \
            "/dev/$portname does not exist or is inaccessible"
        rm -rf data/ .running
        exit 1
    fi
    baudrate=${BPS_COM2[$1]}
    filename=$serialno-$TIMESTAMP\.gps
    stty -F /dev/$portname $baudrate 2>/dev/null
    ./app/str2str -in serial://$portname:$baudrate \
                  -out file://./$folder/$filename 2>/dev/null &
fi

# all data streams are detached from the current shell with the &
# operator, so when this script exits they will still run, until
# the master device runs 'killall str2str'
if [ ${BPS_COM3[$1]} -gt 0 ]; then
    portname=$COM3
    if [[ ! -e /dev/$portname ]]
    then
        ((error_flag++))
        printf "$red%-${SP}s%s$end\n" "[${COLORS[$1]}]" \
            "/dev/$portname does not exist or is inaccessible"
        rm -rf data/ .running
        exit 1
    fi
    baudrate=${BPS_COM3[$1]}
    stty -F /dev/$portname $baudrate 2>/dev/null
    ./app/str2str \
        -in ntrip://inertial:sensor22@us.inertiallabs.com:33101/roof \
        -out file://./$folder/NTRIP-dump.bin \
        -out serial://$portname:$baudrate 2>/dev/null &
fi

sleep 2
