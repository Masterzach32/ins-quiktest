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

# BLUE COM2 - A904D6DI
portname=$COM2
baudrate=${BPS_COM2[5]}
echo "$COM2"
filename=SPAN-$TIMESTAMP\.bin
stty -F /dev/$portname $baudrate 2>/dev/null
app/str2str -in serial://$portname:$baudrate \
          -out file://./$folder/$filename \
          -c cmd/SPAN-start.cmd 2>/dev/null &

# BLUE COM3 - A904D6DL
portname=$COM3
baudrate=${BPS_COM3[5]}
echo "$COM3"
stty -F /dev/$portname $baudrate 2>/dev/null
app/str2str -in ntrip://inertial:sensor22@us.inertiallabs.com:33101/roof \
          -out serial:://$portname:$baudrate 2>/dev/null &

# iterate through all raspberry pi slave devices; only initiate
# the slave script if the master switch is enabled, and one of
# the COM ports is listed with a non-zero baudrate

for (( i=0; i<5; ++i ))
do
    if [ ${BPS_COM1[$i]} -gt 0 ] || [ ${BPS_COM2[$i]} -gt 0 ] || [ ${BPS_COM3[$i]} -gt 0 ]; then
        echo "Syncing ${COLORS[$i]} repository"
        scp global.conf local.defaults slave.sh master.sh .timestamp \
            ${SLAVE_LOGIN[$i]}:$PROJECT_DIR
        echo "Starting data on ${COLORS[$i]}"
        ssh ${SLAVE_LOGIN[$i]} -t "cd $PROJECT_DIR && bash slave.sh $i"
    fi
done

echo "Press ENTER to end test..."
read
echo "Ending test."

for (( i=0; i<5; ++i ))
do
    if [ ${BPS_COM1[$i]} -gt 0 ] || [ ${BPS_COM2[$i]} -gt 0 ] || [ ${BPS_COM3[$i]} -gt 0 ]; then
        echo "Grabbing data from ${COLORS[$i]}"
        scp -rp ${SLAVE_LOGIN[$i]}:$PROJECT_DIR/data/ .
        ssh ${SLAVE_LOGIN[$i]} -t "killall str2str && rm -rf $PROJECT_DIR/data"
        app/ilconv data/${COLORS[$i]}-$TIMESTAMP/F*.bin
    fi
done

app/nconv data/${COLORS[5]}-$TIMESTAMP/SPAN*.bin

rm .timestamp

killall str2str

sleep 1
echo "Done."
