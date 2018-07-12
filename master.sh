#!/bin/bash

error_flag=0 # counts the number of errors thrown

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
    printf "$yellow%-${SP}s%s$end\n" "[${COLORS[0]}]" \
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

# disable the master enable switch if all of the COM ports
# for a particular device are disabled
for (( i=0; i<$NUMBER_OF_NODES; ++i ))
do
    if [[ (${BPS_COM1[$i]} -eq 0 && ${BPS_COM2[$i]} -eq 0 && \
           ${BPS_COM3[$i]} -eq 0) ]]
    then
        ENABLE[0]=0;
    fi

    # boolean array encodes whether data should be grabbed in
    # second node-wise for loop
    success[$i]=0
done

# if the SPAN is enabled (if ENABLE[0] is greater than 1 in global.conf),
# the master device will begin to take data from the SPAN as described
# in this IF block. the baudrates are described in BPS_COM1[0],
# BPS_COM2[0], and BPS_COM3[0]. if any baudrate is 0, the program
# considers this as a disable of that port.

if [[ ${ENABLE[0]} -gt 0 ]]
then
    printf "%-${SP}s%s %s %s %s\n" "[${COLORS[0]}]" \
        "Starting SPAN data w/ baudrates" \
        "[${BPS_COM1[0]}, ${BPS_COM2[0]}, ${BPS_COM3[0]}]"
    printf "%-${SP}s%s %s\n" "[${COLORS[0]}]" \
        "Verify antenna offset in cmd/SPAN-start.cmd:" \
        "[${LX[0]}, ${LY[0]}, ${LZ[0]}]"

    success[0]=1

    if [ ${BPS_COM2[0]} -gt 0 ]
    then
        portname=$COM2
        baudrate=${BPS_COM2[0]}
        filename=SPAN-$TIMESTAMP\.bin
        if [[ ! -e /dev/$portname ]]
        then
            ((error_flag++))
            success[0]=0
            printf "$red%-${SP}s%s$end\n" "[${COLORS[0]}]" \
                "/dev/$portname does not exist or is inaccessible"
        else
            stty -F /dev/$portname $baudrate 2>/dev/null
            app/str2str -in serial://$portname:$baudrate \
                -out file://./$folder/$filename \
                -c cmd/SPAN-start.cmd 2>/dev/null &
        fi
    fi
    if [ ${BPS_COM3[0]} -gt 0 ]
    then
        portname=$COM3
        baudrate=${BPS_COM3[0]}
        if [[ ! -e /dev/$portname ]]
        then
            ((error_flag++))
            success[0]=0
            printf "$red%-${SP}s%s$end\n" "[${COLORS[0]}]" \
                "/dev/$portname does not exist or is inaccessible"
        else
            stty -F /dev/$portname $baudrate 2>/dev/null
            app/str2str \
                -in ntrip://inertial:sensor22@us.inertiallabs.com:33101/roof \
                -out serial:://$portname:$baudrate 2>/dev/null \
                -out file:://./$folder/RTCM3.bin 2>/dev/null &
        fi
    fi
else
    printf "$gray%-${SP}s%s$end\n" "[${COLORS[0]}]" \
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

for (( i=1; i<$NUMBER_OF_NODES; ++i ))
do
    if [[ ${ENABLE[$i]} -eq 0 ]]
    then
        printf "$gray%-${SP}s%s$end\n" "[${COLORS[$i]}]" "Node disabled"
        continue
    fi

    printf "%-${SP}s%s\n" "[${COLORS[$i]}]" "Pinging LAN node at ${LOGIN[$i]}"
    ping -c 1 ${LOGIN[$i]} >/dev/null 2>/dev/null
    if [[ $? -gt 0 ]]
    then
        printf "$red%-${SP}s%s$end\n" "[${COLORS[$i]}]" \
            "Network error: cannot connect to LAN node at ${LOGIN[$i]}"
        ((error_flag++))
        continue
    fi

    ssh $UNAME@${LOGIN[$i]} "[ -f $PROJECT_DIR/.project ]"
    if [[ ! $? -eq 0 ]]
    then
        printf "$red%-${SP}s%s$end\n" "[${COLORS[$i]}]" \
            "Project repository not found on ${LOGIN[$i]}"
        ((error_flag++))
    fi

    printf "%-${SP}s%s\n" "[${COLORS[$i]}]" \
        "Syncing repository at ${LOGIN[$i]}:$PROJECT_DIR"
    scp -r global.conf local.defaults slave.sh master.sh .timestamp src/ \
        $UNAME@${LOGIN[$i]}:$PROJECT_DIR >/dev/null 2>/dev/null
    printf "%-${SP}s%s\n" "[${COLORS[$i]}]" "Starting INS data"
    ssh $UNAME@${LOGIN[$i]} -t "cd $PROJECT_DIR; bash slave.sh $i" 2>/dev/null

    x=$?
    if [[ $x -gt 0 ]]
    then
        printf "%-${SP}s%s\n" "[${COLORS[$i]}]" \
            "$x error(s) were detected during setup"
        ((error_flag+=x));
        continue
    fi

    success[$i]=1
done

if [[ $error_flag -eq 0 ]]
then

    # this WHILE loop is literally just an extremely fancy exit user prompt;
    # it waits for the user to press Q to stop the test, and prints the
    # test duration until that happens.
    BEGIN=$(date +%s)
    warn_flag=1

    while true
    do
        NOW=$(date +%s)
        let DIFF=$(($NOW - $BEGIN))
        let MINS=$(($DIFF / 60))
        let SECS=$(($DIFF % 60))
        let HOURS=$(($DIFF / 3600))
        printf "\r%-${SP}sPress [Q] to exit. Test duration: %02d:%02d:%02d " \
            "[${COLORS[0]}]" $HOURS $MINS $SECS

        # [-s] disables local echo
        # [-t 0.25] sets 0.25 second timeout
        # [-N 1] triggers exit after only one character
        read -s -t 0.25 -N 1 input

        if [[ $input = "q" ]] || [[ $input = "Q" ]]; then
            echo
            break
        fi

        if [[ $warn_flag -eq 0 || $DIFF -lt 5 ]]; then continue; fi

        if [[ ${ENABLE[0]} -gt 0 ]]
        then
            nf=$(find $folder/ -not -type d| wc -l)
            ef=$(find $folder/ -empty -not -type d| wc -l)
            if [[ $nf -eq 0 ]]
            then
                printf "\r$yellow%-${SP}s%s$end\n" "[${COLORS[0]}]" \
                    "Warning: SPAN directory contains no files"
            elif [[ $ef -gt 0 ]]
            then
                printf "\r$yellow%-${SP}s%s$end\n" "[${COLORS[0]}]" \
                    "Warning: $ef of $nf files in SPAN directory are empty"
            fi
        fi

        for (( i=1; i<$NUMBER_OF_NODES; ++i ))
        do
            if [[ ${ENABLE[$i]} -eq 0 ]]; then continue; fi
            nf=$(ssh $UNAME@${LOGIN[$i]} find \
                $PROJECT_DIR/data/${COLORS[$i]}-$TIMESTAMP/ \
                -not -type d | wc -l)
            ef=$(ssh $UNAME@${LOGIN[$i]} find \
                $PROJECT_DIR/data/${COLORS[$i]}-$TIMESTAMP/ \
                -empty -not -type d | wc -l)
            if [[ $nf -eq 0 ]]
            then
                printf "\r$yellow%-${SP}s%s$end\n" "[${COLORS[0]}]" \
                    "Warning: ${COLORS[$i]} directory contains no files"
            elif [[ $ef -gt 0 ]]
            then
                printf "\r$yellow%-${SP}s%s$end\n" "[${COLORS[0]}]" \
                    "Warning: $ef of $nf files in ${COLORS[$i]} directory are empty"
            fi
        done

        warn_flag=0
    done
else
    printf "%-${SP}s%s\n" "[${COLORS[0]}]" \
        "$error_flag error(s) were detected during setup"
    for (( i=0; i<$NUMBER_OF_NODES; ++i ))
    do
        success[$i]=0
    done
fi

printf "%-${SP}s%s\n" "[${COLORS[0]}]" "Ending test..."

INS_TEXT_FILES=() # array of successfully converted INS text files

# this block has the incredibly complicated job of stopping all
# the slave devices, grabbing their data, offsetting it to the SPAN
# reference position, and renaming/reorganizing
for (( i=1; i<$NUMBER_OF_NODES; ++i ))
do
    # if a node's name appears in .error.d/, it means the slave
    # device has thrown an error and data collection/cleanup is
    # unnecessary.
    # if [[ -f ".error.d/${COLORS[$i]}" ]]
    # then
    #     success[$i]=0 # set the success bit for this slave to false
    #     ((error_flag++)) # raise the error flag
    #     continue # skip all steps for this unit
    # fi

    # if the slave is disabled, either via the master enable switch
    # or with its COM port baudrates, the unit will be skipped.
    # the same is true for when its success bit is set to 0.
    if [[ ${ENABLE[$i]} -eq 0 ]]
    then
        continue
    fi
    if [[ ${success[$i]} -eq 0 ]]
    then
        continue
    fi

    # it is determined that the device collected data successfully, so the
    # master will now pull the INS data from the slave

    printf "%-${SP}s%s\n" "[${COLORS[$i]}]" "Grabbing INS data"

    # kill str2str and secure copy data from data folder
    ssh $UNAME@${LOGIN[$i]} "killall str2str" >/dev/null 2>/dev/null
    scp -rp $UNAME@${LOGIN[$i]}:$PROJECT_DIR/data/${COLORS[$i]}-$TIMESTAMP \
        data/ >/dev/null 2>/dev/null
    if [ $? -ne 0 ]
    then
        # throw an error if secure copy fails
        printf "$red%-${SP}s%s\n$end" "[${COLORS[$i]}]" \
            "Failed to collect INS data"
        ((error_flag++))
    elif [[ ${BPS_COM1[$i]} -gt 0 ]]
    then
        # iff data is collected successfully, the INS log needs to be
        # converted to text and offset to the SPAN position (this is
        # done with app/ilconv, the source for which is found in
        # src/ilconv.cpp)

        PVX=$(echo "${LX[$i]} - ${LX[0]}" | bc)
        PVY=$(echo "${LY[$i]} - ${LY[0]}" | bc)
        PVZ=$(echo "${LZ[$i]} - ${LZ[0]}" | bc)
        printf "%-${SP}s%s\n" "[${COLORS[$i]}]" \
            "Converting INS data w/ PV offset [$PVX, $PVY, $PVZ]"
        app/ilconv data/${COLORS[$i]}-$TIMESTAMP/*.bin \
            --pvoff $PVX $PVY $PVZ >/dev/null 2>/dev/null
        if [ $? -ne 0 ]
        then
            # throw an error if conversion fails
            printf "$red%-${SP}s%s\n$end" "[${COLORS[$i]}]" \
                "Error: failed to convert INS log file to text"
            ((error_flag++))
        fi

        # move data around, rename folders, add to array of files
        if [[ -f data/${COLORS[$i]}-$TIMESTAMP/.serial ]]
        then
            serialno=$(cat data/${COLORS[$i]}-$TIMESTAMP/.serial)
            mv data/${COLORS[$i]}-$TIMESTAMP data/$serialno-$TIMESTAMP
        fi
        # rm data/$serialno-$TIMESTAMP/.serial
        INS_TEXT_FILES+=("$serialno-$TIMESTAMP")
    fi
    # copy all other LOG folders from slave, and clean up dotfiles
    scp -rp $UNAME@${LOGIN[$i]}:$PROJECT_DIR/data/LOG-* data/ >/dev/null 2>/dev/null
    ssh $UNAME@${LOGIN[$i]} -t "cd $PROJECT_DIR &&\
        rm -rf data .running" >/dev/null 2>/dev/null
done

# if the SPAN is enabled, convert the data to INSPVAA log
if [[ ${ENABLE[0]} -gt 0 && ${success[0]} -gt 0 ]]
then
    printf "%-${SP}s%s\n" "[${COLORS[0]}]" "Converting SPAN data"
    app/nconv data/${COLORS[0]}-$TIMESTAMP/SPAN*.bin >/dev/null 2>/dev/null
    if [[ $? -ne 0 ]]
    then
        # throw an error if conversion fails
        printf "$red%-${SP}s%s$end\n" "[${COLORS[0]}]" \
            "Error: failed to convert SPAN log file to txt"
        success[0]=0
        ((error_flag++))
    fi

    # restructure LOG folder
    mv data/${COLORS[0]}-$TIMESTAMP data/SPAN-$TIMESTAMP
    mkdir data/LOG
    mv data/*-$TIMESTAMP data/LOG
    mv data/LOG data/LOG-$TIMESTAMP
else
    rm -rf data/${COLORS[0]}-$TIMESTAMP 2>/dev/null
fi

# kill str2str, remove dotfiles
killall str2str >/dev/null 2>/dev/null
rm -rf .running .timestamp .error.d
printf "%-${SP}s%s\n" "[${COLORS[0]}]" "Done."

# if the error flag is ever raised (it cannot be lowered, so
# error_flag > 0 indicates at least one error was thrown), the user
# is given the option to delete the entire test log.
#
# the user MUST enter exactly 'Yes' or 'yes' to delete the data,
# and must enter exactly 'No' or 'no' to dismiss the dialogue.
if [[ error_flag -gt 0 ]]
then
    printf "%-${SP}s%s" "[${COLORS[0]}]" \
        "$error_flag error(s) were reported. Delete entire test log? [yes/no] "
    read input
    message_count=0
    while [[ "$input" != "Yes" && "$input" != "yes" && \
        "$input" != "No" && "$input" != "no" ]]
    do
        messages=(  "Please type 'yes' or 'no'. "
                    "PLEASE type 'yes' or 'no'. "
                    "PLEASE DO IT. "
                    "Please? Just type yes or no. "
                    "omgplz. "
                    "I hate you. "
                    "... "
                    "... "
                    "Do you feel good about yourself? "
                    "You should be working right now. "
                    "... "
                    "Moo. ")
        if [[ $message_count -eq ${#messages[@]} ]]
        then
            input="Moo."
            break
        fi
        printf "%-${SP}s%s" "[${COLORS[0]}]" "${messages[$message_count]}"
        read input
        ((message_count++))
    done
    if [[ "$input" == "Yes" || "$input" == "yes" ]]
    then
        printf "%-${SP}s%s\n" "[${COLORS[0]}]" "Deleting data/LOG-$TIMESTAMP"
        rm -rf data/LOG-$TIMESTAMP
        exit
    fi
fi

# at this point the only thing left to do is to generate a report, so if
# the SPAN data was disabled, the script can exit
if [[ ${ENABLE[0]} -eq 0 || ${success[0]} -eq 0 ]]
then
    exit
fi

# user promt to generate reports for INS accuracy; user can enter
# 'Y' or 'y' to afirm, or any key to dismiss
printf "%-${SP}s%s" "[${COLORS[0]}]" "Generate report? [y/n] "
read -N 1 input
if [[ $input = "y" ]] || [[ $input = "Y" ]]; then
    echo
else
    echo
    exit
fi

# for every INS text file added during second node loop,
# run passfail.m with the test file and INSPVAA log as arguments
printf "%-${SP}s%s\n" "[${COLORS[0]}]" "Generating reports..."
for fn in "${INS_TEXT_FILES[@]}"
do
    printf "%-${SP}s%s\n" "[${COLORS[0]}]" \
        "Writing to data/LOG-$TIMESTAMP/$fn/Accuracy Report.dingleberry"
    octave-cli passfail.m \
        data/LOG-$TIMESTAMP/$fn/$fn.txt \
        data/LOG-$TIMESTAMP/SPAN-$TIMESTAMP/SPAN-$TIMESTAMP.txt
    touch "data/LOG-$TIMESTAMP/$fn/Accuracy Report.dingleberry"
done
