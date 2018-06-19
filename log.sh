#/bin/sh!

echo "Press ENTER to quit."
timestamp=$(date -u +%Y-%m-%d-%H-%M-%S)
folder=data/LOG-$timestamp
mkdir -p $folder
make all >/dev/null 2>/dev/null

################################################################################

# GREEN COM1 - A904D6DE
# portname=serial/by-id/usb-FTDI_FT232R_USB_UART_A904D6DE-if00-port0
# GREEN COM2 - A904D6DR
# portname=serial/by-id/usb-FTDI_FT232R_USB_UART_A904D6DR-if00-port0
# GREEN COM3 - A904D6DK
# portname=serial/by-id/usb-FTDI_FT232R_USB_UART_A904D6DK-if00-port0

portname=ttyUSB0
baudrate=460800
stty -F /dev/$portname $baudrate 2>/dev/null
serialno="$(./app/ldprm /dev/$portname --name \
          --rate 200 --lever -0.02 -1.47 1.23 --angles 0 0 0 --init 7)"
if [ -z "$serialno" ]; then
    serialno="INS"
fi
filename=$serialno-$timestamp\.bin
sleep 3
./app/str2str -in serial://$portname:$baudrate \
              -out file://./$folder/$filename \
              -c cmd/INS_OPVT2AHR.cmd &

portname=ttyUSB1
baudrate=460800
filename=$serialno-GNSS-%Y-$timestamp\.gps
stty -F /dev/$portname $baudrate 2>/dev/null
./app/str2str -in serial://$portname:$baudrate \
          -out file://./$folder/$filename &

portname=ttyUSB2
baudrate=460800
filename=SPAN-$timestamp\.bin
stty -F /dev/$portname $baudrate 2>/dev/null
./app/str2str -in serial://$portname:$baudrate \
              -out file://./$folder/$filename &

portname=ttyUSB3
baudrate=460800
stty -F /dev/$portname $baudrate 2>/dev/null
./app/str2str -in ntrip://inertial:sensor22@us.inertiallabs.com:33101/roof \
              -out serial:://$portname:$baudrate &

read
killall str2str
sleep 1

./app/ilconv $folder/$serialno-$timestamp\.bin
./app/nconv $folder/SPAN-$timestamp\.bin

echo "Done."
