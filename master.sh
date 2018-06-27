#!/bin/bash

trap '' 2 # disable SIGINT
source global.conf
source local.defaults
if [ -f local.conf ]; then
    source local.conf
fi

TIMESTAMP=$(date -u +%Y-%m-%d-%H-%M-%S)
folder=data/${COLORS[5]}-"$TIMESTAMP"
mkdir -p $folder >/dev/null 2>/dev/null
make all >/dev/null 2>/dev/null
echo "$TIMESTAMP" > .timestamp

printf "%-10s%s\n" "[${COLORS[5]}]" "Starting SPAN data with baudrates \
[${BPS_COM1[5]}, ${BPS_COM2[5]}, ${BPS_COM3[5]}]"

portname=$COM2
baudrate=${BPS_COM2[5]}
filename=SPAN-$TIMESTAMP\.bin
stty -F /dev/$portname $baudrate 2>/dev/null
app/str2str -in serial://$portname:$baudrate \
          -out file://./$folder/$filename \
          -c cmd/SPAN-start.cmd 2>/dev/null &

portname=$COM3
baudrate=${BPS_COM3[5]}
stty -F /dev/$portname $baudrate 2>/dev/null
app/str2str -in ntrip://inertial:sensor22@us.inertiallabs.com:33101/roof \
          -out serial:://$portname:$baudrate 2>/dev/null &

# iterate through all raspberry pi slave devices; only initiate
# the slave script if the master switch is enabled, and one of
# the COM ports is listed with a non-zero baudrate

for (( i=0; i<5; ++i ))
do
    if [ ${BPS_COM1[$i]} -gt 0 ] || [ ${BPS_COM2[$i]} -gt 0 ] || [ ${BPS_COM3[$i]} -gt 0 ]; then
        printf "%-10s%s\n" "[${COLORS[$i]}]" \
            "Syncing repository at ${SLAVE_LOGIN[$i]}:$PROJECT_DIR"
        scp global.conf local.defaults slave.sh master.sh .timestamp \
            ${SLAVE_LOGIN[$i]}:$PROJECT_DIR > /dev/null
        printf "%-10s%s\n" "[${COLORS[$i]}]" "Starting INS data"
        ssh ${SLAVE_LOGIN[$i]} -t "cd $PROJECT_DIR && bash slave.sh $i"
    fi
done

BEGIN=$(date +%s)
printf "%-10s%s\n" "[${COLORS[5]}]" "Press [Q] to exit."

while true
do
    NOW=$(date +%s)
    let DIFF=$(($NOW - $BEGIN))
    let MINS=$(($DIFF / 60))
    let SECS=$(($DIFF % 60))
    let HOURS=$(($DIFF / 3600))

    printf "\r%-10sTest duration: %02d:%02d:%02d" "[${COLORS[5]}]" $HOURS $MINS $SECS

    read -s -t 0.25 -N 1 input
    if [[ $input = "q" ]] || [[ $input = "Q" ]]; then
        echo
        break
    fi
done

printf "%-10s%s\n" "[${COLORS[5]}]" "Ending test..."

for (( i=0; i<5; ++i ))
do
    if [ ${BPS_COM1[$i]} -gt 0 ] || [ ${BPS_COM2[$i]} -gt 0 ] || [ ${BPS_COM3[$i]} -gt 0 ]; then
        printf "%-10s%s\n" "[${COLORS[$i]}]" "Grabbing INS data"
        scp -rp ${SLAVE_LOGIN[$i]}:$PROJECT_DIR/data/ .
        ssh ${SLAVE_LOGIN[$i]} -t "killall str2str && rm -rf $PROJECT_DIR/data"
        printf "%-10s%s\n" "[${COLORS[$i]}]" "Converting INS data"
        app/ilconv data/${COLORS[$i]}-$TIMESTAMP/F*.bin
    fi
done

printf "%-10s%s\n" "[${COLORS[5]}]" "Converting SPAN data"
app/nconv data/${COLORS[5]}-$TIMESTAMP/SPAN*.bin

rm .timestamp

killall str2str >/dev/null

sleep 1
printf "%-10s%s\n" "[${COLORS[5]}]" "Done."
