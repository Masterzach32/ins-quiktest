#!/bin/bash

# usbconf.sh

# this program is a utility which assists hardware setup
# for an arbitrary node in the ins-quiktest network

if [ ! -f .project ] # working dir is not in project
then
    echo "$0: must be run from within ins-quiktest project directory"
    exit
fi

printf "Enter the name of the config file: "
read filename

if [[ -f config/$filename ]]; then
    printf "Warning: config/$filename will be overwritten. "
    printf "Continue? [y/n] "
    read answer
    if [[ "$answer" != "y" && "$answer" != "Y" ]]
    then exit 1; fi
fi
printf "Writing to config/%s.\n" "$filename"

echo -e "#!/bin/bash\n" > config/$filename
echo -e "# master.local\n" >> config/$filename
echo -e "# dev serial device, excluding \"/dev/\"" >> config/$filename
echo -e "# -- for example, /dev/ttyUSB0 is expressed as ttyUSB0\n" \
    >> config/$filename

printf "Please connect COM1 (RS-232); then press ENTER. "
read
echo "COM1=serial/by-id/$(ls /dev/serial/by-id/)" >> config/$filename

printf "Disconnect COM1 (RS-232) and connect COM1 (RS-422); then press ENTER. "
read
echo "COM1_RS422=serial/by-id/$(ls /dev/serial/by-id/)" >> config/$filename

printf "Disconnect COM1 (RS-422) and connect COM2; then press ENTER. "
read
echo "COM2=serial/by-id/$(ls /dev/serial/by-id/)" >> config/$filename

printf "Disconnect COM2 and connect COM3; then press ENTER. "
read
echo "COM3=serial/by-id/$(ls /dev/serial/by-id/)" >> config/$filename

