#!/bin/bash

# ping.sh

# this program is a utility which pings all the hostnames
# defined in global.conf on the local area network. it can
# be used as a diagnostic tool, which can verify whether
# any or all nodes are present on the network.

if [ ! -f .project ] # working dir is not in project
then
    echo "$0: must be run from within ins-quiktest project directory"
    exit
fi

source config/global.conf

for (( i=0; i<$NUMBER_OF_NODES; ++i ))
do
    timeout 2 ping -c 1 ${LOGIN[$i]} >/dev/null 2>/dev/null
    if [[ $? -eq 0 ]]
    then
        printf "%-${SP}s%s\n" "[${COLORS[$i]}]" \
            "Found ${LOGIN[$i]}"
    else
        printf "$red%-${SP}s%s$end\n" "[${COLORS[$i]}]" \
            "Did not find ${LOGIN[$i]}"
    fi
done
