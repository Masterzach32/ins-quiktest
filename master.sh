#!/bin/bash

red=$'\e[1;31m'
yellow=$'\e[33m'
end=$'\e[0m'

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
trap '' 2 # disable SIGINT
source global.conf
source local.defaults
if [ -f local.conf ]; then
    source local.conf
else
    printf "$yellow%-10s%s\n$end" "[${COLORS[0]}]" \
        "Warning: no local config provided, using local.defaults"
fi

TIMESTAMP=$(date -u +%Y-%m-%d-%H-%M-%S)
folder=data/${COLORS[0]}-"$TIMESTAMP"
mkdir -p $folder >/dev/null 2>/dev/null
make all >/dev/null 2>/dev/null
echo "$TIMESTAMP" > .timestamp

printf "%-10s%s %s %s %s\n" "[${COLORS[0]}]" "Starting SPAN data w/ baudrates" \
    "[${BPS_COM1[0]}, ${BPS_COM2[0]}, ${BPS_COM3[0]}]" \
    "and lever arm" "[${LX[0]}, ${LY[0]}, ${LZ[0]}]"

portname=$COM2
baudrate=${BPS_COM2[0]}
filename=SPAN-$TIMESTAMP\.bin
stty -F /dev/$portname $baudrate 2>/dev/null
app/str2str -in serial://$portname:$baudrate \
            -out file://./$folder/$filename \
            -c cmd/SPAN-start.cmd 2>/dev/null &

portname=$COM3
baudrate=${BPS_COM3[0]}
stty -F /dev/$portname $baudrate 2>/dev/null
app/str2str -in ntrip://inertial:sensor22@us.inertiallabs.com:33101/roof \
            -out serial:://$portname:$baudrate 2>/dev/null &

# iterate through all raspberry pi slave devices; only initiate
# the slave script if the master switch is enabled, and one of
# the COM ports is listed with a non-zero baudrate

for (( i=1; i<${#ENABLE[@]}; ++i ))
do
    if [ ${BPS_COM1[$i]} -gt 0 ] || [ ${BPS_COM2[$i]} -gt 0 ] || [ ${BPS_COM3[$i]} -gt 0 ]; then
        printf "%-10s%s\n" "[${COLORS[$i]}]" \
            "Syncing repository at ${LOGIN[$i]}:$PROJECT_DIR"
        scp global.conf local.defaults slave.sh master.sh .timestamp \
            ${LOGIN[$i]}:$PROJECT_DIR >/dev/null 2>/dev/null
        printf "%-10s%s\n" "[${COLORS[$i]}]" "Starting INS data"
        ssh ${LOGIN[$i]} -t "cd $PROJECT_DIR && bash slave.sh $i" 2>/dev/null
    fi
done

rm .timestamp
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

    read -s -t 0.25 -N 1 input
    if [[ $input = "q" ]] || [[ $input = "Q" ]]; then
        echo
        break
    fi
done

printf "%-10s%s\n" "[${COLORS[0]}]" "Ending test..."

INS_TEXT_FILES=()

for (( i=1; i<${#ENABLE[@]}; ++i ))
do
    if [ ${BPS_COM1[$i]} -gt 0 ] || [ ${BPS_COM2[$i]} -gt 0 ] || [ ${BPS_COM3[$i]} -gt 0 ]; then
        printf "%-10s%s\n" "[${COLORS[$i]}]" "Grabbing INS data"
        ssh ${LOGIN[$i]} -t "killall str2str" >/dev/null 2>/dev/null
        scp -rp ${LOGIN[$i]}:$PROJECT_DIR/data/${COLORS[$i]}-$TIMESTAMP \
            data/ >/dev/null 2>/dev/null
        if [ $? -ne 0 ]
        then
            printf "$red%-10s%s\n$end" "[${COLORS[$i]}]" \
                "Error: failed to collect INS data"
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
            fi
            serialno=$(cat data/${COLORS[$i]}-$TIMESTAMP/.serial)
            mv data/${COLORS[$i]}-$TIMESTAMP data/$serialno-$TIMESTAMP
            # rm data/$serialno-$TIMESTAMP/.serial
            INS_TEXT_FILES+=("$serialno-$TIMESTAMP")
        fi
        scp -rp ${LOGIN[$i]}:$PROJECT_DIR/data/LOG-* data/ >/dev/null 2>/dev/null
        ssh ${LOGIN[$i]} -t "cd $PROJECT_DIR &&\
            rm -rf data .running" >/dev/null 2>/dev/null
    fi
done

printf "%-10s%s\n" "[${COLORS[0]}]" "Converting SPAN data"
app/nconv data/${COLORS[0]}-$TIMESTAMP/SPAN*.bin >/dev/null 2>/dev/null
if [ $? -ne 0 ]
then
    printf "$red%-10s%s\n$end" "[${COLORS[0]}]" \
        "Error: failed to convert SPAN log file to txt"
fi

mv data/${COLORS[0]}-$TIMESTAMP data/SPAN-$TIMESTAMP
mkdir data/LOG
mv data/*-$TIMESTAMP data/LOG
mv data/LOG data/LOG-$TIMESTAMP
killall str2str >/dev/null
rm .running
printf "%-10s%s\n" "[${COLORS[0]}]" "Done."

printf "%-10s%s" "[${COLORS[0]}]" "Generate report? [y/n] "
read -s -N 1 input
if [[ $input = "y" ]] || [[ $input = "Y" ]]; then
    echo
else
    echo
    exit
fi

printf "%-10s%s\n" "[${COLORS[0]}]" "Generating reports..."
for fn in "${INS_TEXT_FILES[@]}"
do
    printf "%-10s%s\n" "[${COLORS[0]}]" "Writing to data/LOG-$TIMESTAMP/$fn/Accuracy Report.dingleberry"
    touch "data/LOG-$TIMESTAMP/$fn/Accuracy Report.dingleberry"
done
