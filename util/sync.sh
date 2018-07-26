#!/bin/bash

# sync.sh

if [[ ! -f .project ]] # working dir is not in project
then
    echo "$0: must be run from within ins-quiktest project directory"
    exit
fi

source config/global.conf

PROJECT_DIR=$(pwd)

for (( i=1; i<$NUMBER_OF_NODES; ++i ))
do
    printf "%-${SP}s%s\n" "[${COLORS[$i]}]" \
        "Syncing repository at ${LOGIN[$i]}:$PROJECT_DIR"
    # ssh $UNAME@${LOGIN[$i]} "cd $PROJECT_DIR; git fetch --all; git reset --hard originmake"
    scp -r $(paste -s -d ' ' config/manifest.txt) \
        $UNAME@${LOGIN[$i]}:$PROJECT_DIR
done
