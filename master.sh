#!/bin/bash

source global.conf

echo "Press ENTER to quit."

for i in 0 1 2 3 4; do
    if [ ${BPS_COM1[i]} -gt 0 ] ||
       [ ${BPS_COM2[i]} -gt 0 ] ||
       [ ${BPS_COM3[i]} -gt 0 ]; then
        scp global.conf ${SLAVE_LOGIN[$i]}:$PROJECT_DIR
        ssh ${SLAVE_LOGIN[$i]} cd $PROJECT_DIR && ./slave.sh $i
    fi
done

read

for i in 0 1 2 3 4; do
    if [ ${BPS_COM1[i]} -gt 0 ] ||
       [ ${BPS_COM2[i]} -gt 0 ] ||
       [ ${BPS_COM3[i]} -gt 0 ]; then
        ssh ${SLAVE_LOGIN[$i]} killall str2str
    fi
done

sleep 1
echo "Done."
