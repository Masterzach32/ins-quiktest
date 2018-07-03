#!/bin/bash

# define escape characters for fancy console colors
red=$'\e[1;31m'
yellow=$'\e[33m'
end=$'\e[0m' # clears formatting

if [ ! -f .project ] # working dir is not in project
then
    echo "$0: must be run from within ins-quiktest project directory"
    exit
fi

source global.conf

for (( i=0; i<${#LOGIN[@]}; ++i ))
do
    timeout 2 ping -c 1 ${LOGIN[$i]} >/dev/null 2>/dev/null
    if [[ $? -eq 0 ]]
    then
        printf "%-10s%s\n" "[${COLORS[$i]}]" "Found ${LOGIN[$i]}"
    else
        printf "$red%-10s%s$end\n" "[${COLORS[$i]}]" "Did not find ${LOGIN[$i]}"
    fi
done
