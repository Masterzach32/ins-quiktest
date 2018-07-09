#!/bin/bash

# define escape characters for fancy console colors
red=$'\e[1;31m'
yellow=$'\e[33m'
gray=$'\e[90m'
end=$'\e[0m' # clears formatting

error_flag=0; # error flag, 0 if 100% success, 1 if otherwise

# the following blocks check for the existence of .project and .timestamp;
# the .project file is found in the project directory */ins-quiktest/, and
# all scripts depend on being run from this directory. therefore the master
# script will not run if .project is not present.
# furthermore, .timestamp must NOT be present, because this indicates that
# the master or slave script is already being run on this machine.
if [ ! -f .project ] # working dir is not in project
then
    echo "$0: must be run from within ins-quiktest project directory"
    exit
fi
if [ -f .timestamp ] # test is already running
then
    echo "$0: already running on this machine; only one instance allowed"
    exit
fi

touch .running # prevents slave script from running
mkdir .error.d >/dev/null 2>/dev/null # error handling directory

# this disables quitting the program using ctrl-C, because it must
# be allowed to clean up after itself; to quit the program normally,
# press Q/q when prompted.
trap '' 2

# the following block loads variables from global.conf and local.defaults.
# additionally, if a local.conf file exists, it will load variables from it
# as well; local.conf can be used to overwrite local.defaults with machine-
# specific values, like COM port device names. if local.conf is not found,
# a soft warning will be thrown, but the program can continue as normal.
source global.conf
source local.defaults
if [ -f local.conf ]; then
    source local.conf
else
    printf "$yellow%-10s%s$end\n" "[${COLORS[0]}]" \
        "Warning: no local config provided, using local.defaults"
fi

# PROJECT_DIR stores the absolute path to the project directory for
# convenience. the rest of this block creates the master data folder,
# and records the timestamp to be used for the current test. the
# timestamp is stored in $PROJECT_DIR/.timestamp.
PROJECT_DIR=$(pwd)
TIMESTAMP=$(date -u +%Y-%m-%d-%H-%M-%S)
folder=data/${COLORS[0]}-"$TIMESTAMP"
mkdir -p $folder >/dev/null 2>/dev/null
make all >/dev/null 2>/dev/null
echo "$TIMESTAMP" > .timestamp

# if the SPAN is enabled (if ENABLE[0] is greater than 1 in global.conf),
# the master device will begin to take data from the SPAN as described
# in this IF block. the baudrates are described in BPS_COM1[0],
# BPS_COM2[0], and BPS_COM3[0]. if any baudrate is 0, the program
# considers this as a disable of that port.

if [ ${ENABLE[0]} -gt 0 ]
then
    printf "%-10s%s %s %s %s\n" "[${COLORS[0]}]" \
        "Starting SPAN data w/ baudrates" \
        "[${BPS_COM1[0]}, ${BPS_COM2[0]}, ${BPS_COM3[0]}]" \
        "and lever arm" "[${LX[0]}, ${LY[0]}, ${LZ[0]}]"

    if [ ${BPS_COM2[0]} -gt 0 ]
    then
        portname=$COM2
        baudrate=${BPS_COM2[0]}
        filename=SPAN-$TIMESTAMP\.bin
        stty -F /dev/$portname $baudrate 2>/dev/null
        app/str2str -in serial://$portname:$baudrate \
                    -out file://./$folder/$filename \
                    -c cmd/SPAN-start.cmd 2>/dev/null &
    fi
    if [ ${BPS_COM3[0]} -gt 0 ]
    then
        portname=$COM3
        baudrate=${BPS_COM3[0]}
        stty -F /dev/$portname $baudrate 2>/dev/null
        app/str2str \
            -in ntrip://inertial:sensor22@us.inertiallabs.com:33101/roof \
            -out serial:://$portname:$baudrate 2>/dev/null &
    fi
else
    printf "$gray%-10s%s$end\n" "[${COLORS[0]}]" \
        "SPAN reference data disabled"
fi

# this FOR loop iterates through all of the slave devices, which are
# configured using the 2nd through Nth elements of the arrays in
# global.conf. this loop assumes the ENABLE switch array is the same
# length as the rest of the arrays.
# 
# for each device, two conditions must be met in order for the slave node
# to be started:
#
#   1. any of the three COM ports must be specified 
#      with a positive nonzero baudrate
#   2. the master enable switch must be greater than 0
#
# if these conditions are met, the master copies a few files from its own
# repository to each of the slave devices, and then initiates the
# slave script.

# boolean array encodes whether data should be grabbed in
# second node-wise for loop
success=(0 0 0 0 0 0);

for (( i=1; i<${#ENABLE[@]}; ++i ))
do
    if [[ ${ENABLE[$i]} -eq 0 ]]
    then
        printf "$gray%-10s%s$end\n" "[${COLORS[$i]}]" "Node disabled"
        continue
    fi
    if [[ (${BPS_COM1[$i]} -eq 0 && ${BPS_COM2[$i]} -eq 0 && \
           ${BPS_COM3[$i]} -eq 0) ]]
    then
        printf "$gray%-10s%s$end\n" "[${COLORS[$i]}]" "Node disabled"
        continue
    fi

    printf "%-10s%s\n" "[${COLORS[$i]}]" "Pinging LAN node at ${LOGIN[$i]}"
    ping -c 1 ${LOGIN[$i]} >/dev/null 2>/dev/null
    if [[ $? -gt 0 ]]
    then
        printf "$red%-10s%s$end\n" "[${COLORS[$i]}]" \
            "Cannot connect to LAN node at ${LOGIN[$i]}"
        error_flag=1
        continue
    fi
    ssh $UNAME@${LOGIN[$i]} "[ -f $PROJECT_DIR/.project ]"
    if [[ $? -eq 0 ]]
    then
        printf "%-10s%s\n" "[${COLORS[$i]}]" \
            "Syncing repository at ${LOGIN[$i]}:$PROJECT_DIR"
        scp global.conf local.defaults slave.sh master.sh .timestamp \
            $UNAME@${LOGIN[$i]}:$PROJECT_DIR >/dev/null 2>/dev/null
        printf "%-10s%s\n" "[${COLORS[$i]}]" "Starting INS data"
        ssh $UNAME@${LOGIN[$i]} -t "cd $PROJECT_DIR && bash slave.sh $i" 2>/dev/null
        success[$i]=1
    else
        printf "$red%-10s%s$end\n" "[${COLORS[$i]}]" \
            "Project repository not found on ${LOGIN[$i]}"
        error_flag=1
    fi
done

# this WHILE loop is literally just an extremely fancy exit user prompt;
# it waits for the user to press Q to stop the test, and prints the
# test duration until that happens.
BEGIN=$(date +%s)
printf "%-10s%s\n" "[${COLORS[0]}]" "Press [Q] to exit."
while true
do
    NOW=$(date +%s)
    let DIFF=$(($NOW - $BEGIN))
    let MINS=$(($DIFF / 60))
    let SECS=$(($DIFF % 60))
    let HOURS=$(($DIFF / 3600))
    printf "\r%-10sTest duration: %02d:%02d:%02d " "[${COLORS[0]}]" $HOURS $MINS $SECS
    
    # [-s] disables local echo
    # [-t 0.25] sets 0.25 second timeout
    # [-N 1] triggers exit after only one character
    read -s -t 0.25 -N 1 input
    
    if [[ $input = "q" ]] || [[ $input = "Q" ]]; then
        echo
        break
    fi
done

printf "%-10s%s\n" "[${COLORS[0]}]" "Ending test..."

INS_TEXT_FILES=()

# this block has the incredibly complicated job of stopping all
# the slave devices, grabbing their data, offsetting it to the SPAN
# reference position, and renaming/reorganizing
for (( i=1; i<${#ENABLE[@]}; ++i ))
do
    # double check node success
    if [[ -f ".error.d/${COLORS[$i]}" ]]
    then
        success[$i]=0
        error_flag=1
        continue
    fi
    if [[ ${ENABLE[$i]} -eq 0 ]]
    then
        continue
    fi
    if [[ (${BPS_COM1[$i]} -eq 0 && ${BPS_COM2[$i]} -eq 0 && \
           ${BPS_COM3[$i]} -eq 0) ]]
    then
        continue
    fi
    if [[ ${success[$i]} -eq 0 ]]
    then
        continue
    fi

    printf "%-10s%s\n" "[${COLORS[$i]}]" "Grabbing INS data"
    ssh $UNAME@${LOGIN[$i]} -t "killall str2str" >/dev/null 2>/dev/null
    scp -rp $UNAME@${LOGIN[$i]}:$PROJECT_DIR/data/${COLORS[$i]}-$TIMESTAMP \
        data/ >/dev/null 2>/dev/null
    if [ $? -ne 0 ]
    then
        printf "$red%-10s%s\n$end" "[${COLORS[$i]}]" \
            "Failed to collect INS data"
        error_flag=1
    else
        PVX=$(echo "${LX[$i]} - ${LX[0]}" | bc)
        PVY=$(echo "${LY[$i]} - ${LY[0]}" | bc)
        PVZ=$(echo "${LZ[$i]} - ${LZ[0]}" | bc)
        printf "%-10s%s\n" "[${COLORS[$i]}]" \
            "Converting INS data w/ PV offset [$PVX, $PVY, $PVZ]"
        app/ilconv data/${COLORS[$i]}-$TIMESTAMP/*.bin \
            --pvoff $PVX $PVY $PVZ >/dev/null 2>/dev/null
        if [ $? -ne 0 ]
        then
            printf "$red%-10s%s\n$end" "[${COLORS[$i]}]" \
                "Error: failed to convert INS log file to txt"
            error_flag=1
        fi
        serialno=$(cat data/${COLORS[$i]}-$TIMESTAMP/.serial)
        mv data/${COLORS[$i]}-$TIMESTAMP data/$serialno-$TIMESTAMP
        # rm data/$serialno-$TIMESTAMP/.serial
        INS_TEXT_FILES+=("$serialno-$TIMESTAMP")
    fi
    scp -rp $UNAME@${LOGIN[$i]}:$PROJECT_DIR/data/LOG-* data/ >/dev/null 2>/dev/null
    ssh $UNAME@${LOGIN[$i]} -t "cd $PROJECT_DIR &&\
        rm -rf data .running" >/dev/null 2>/dev/null
done

if [[ ${ENABLE[0]} -gt 0 ]]
then
    printf "%-10s%s\n" "[${COLORS[0]}]" "Converting SPAN data"
    app/nconv data/${COLORS[0]}-$TIMESTAMP/SPAN*.bin >/dev/null 2>/dev/null
    if [[ $? -ne 0 ]]
    then
        printf "$red%-10s%s$end\n" "[${COLORS[0]}]" \
            "Error: failed to convert SPAN log file to txt"
        error_flag=1
    fi
fi

mv data/${COLORS[0]}-$TIMESTAMP data/SPAN-$TIMESTAMP
mkdir data/LOG
mv data/*-$TIMESTAMP data/LOG
mv data/LOG data/LOG-$TIMESTAMP
killall str2str >/dev/null 2>/dev/null
rm -rf .running .timestamp .error.d
printf "%-10s%s\n" "[${COLORS[0]}]" "Done."

if [[ error_flag -gt 0 ]]
then
    printf "%-10s%s" "[${COLORS[0]}]" \
        "Errors were reported. Delete entire test log? [yes/no] "
    read input
    while [[ "$input" != "Yes" && "$input" != "yes" && \
        "$input" != "No" && "$input" != "no" ]]
    do
        printf "%-10s%s" "[${COLORS[0]}]" "Please type 'yes' or 'no'. "
        read input
    done
    if [[ "$input" == "Yes" || "$input" == "yes" ]]
    then
        printf "%-10s%s\n" "[${COLORS[0]}]" "Deleting data/LOG-$TIMESTAMP"
        rm -rf data/LOG-$TIMESTAMP
        exit
    fi
fi

if [[ ${ENABLE[0]} -eq 0 ]]
then
    exit
fi
if [[ ${BPS_COM1[0]} -eq 0 && ${BPS_COM2[0]} -eq 0 && ${BPS_COM3[0]} -eq 0 ]]
then
    exit
fi

printf "%-10s%s" "[${COLORS[0]}]" "Generate report? [y/n] "
read -N 1 input
if [[ $input = "y" ]] || [[ $input = "Y" ]]; then
    echo
else
    echo
    exit
fi

printf "%-10s%s\n" "[${COLORS[0]}]" "Generating reports..."
for fn in "${INS_TEXT_FILES[@]}"
do
    printf "%-10s%s\n" "[${COLORS[0]}]" \
        "Writing to data/LOG-$TIMESTAMP/$fn/Accuracy Report.dingleberry"
    octave-cli passfail.m \
        data/LOG-$TIMESTAMP/$fn/$fn.txt \
        data/LOG-$TIMESTAMP/SPAN-$TIMESTAMP/SPAN-$TIMESTAMP.txt
    touch "data/LOG-$TIMESTAMP/$fn/Accuracy Report.dingleberry"
done
