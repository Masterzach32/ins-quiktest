#!/bin/bash

source global.conf

echo "Press ENTER to quit."

for i in 0 1 2 3 4; do
    if [ ${BPS_COM1[i]} -gt 0 ] ||
       [ ${BPS_COM2[i]} -gt 0 ] ||
       [ ${BPS_COM3[i]} -gt 0 ]; then

        scp global.conf ${SLAVE_LOGIN[i]}:$PROJECT_DIR
        ssh ${SLAVE_LOGIN[i]} cd $PROJECT_DIR && ./slave.sh $i
        exit
    fi
done

read

for i in 0 1 2 3 4; do
    if [ ${BPS_COM1[i]} -gt 0 ] ||
       [ ${BPS_COM2[i]} -gt 0 ] ||
       [ ${BPS_COM3[i]} -gt 0 ]; then
        mkdir -p data
        echo "$1"
        ssh ${SLAVE_LOGIN[i]} killall str2str
        exit
    fi
done

killall str2str
