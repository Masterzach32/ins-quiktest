#/bin/sh!

echo "Press ENTER to quit."
folder=data/LOG-$(date -u +%Y-%m-%d-%H-%M-%S)
mkdir -p $folder

################################################################################

# GREEN COM1 - A904D6DE
portname=serial/by-id/usb-FTDI_FT232R_USB_UART_A904D6DE-if00-port0
baudrate=460800
serialno="$(./ldprm /dev/$portname -n -r 200 -l 1 2 3 -a 30 60 90)"
filename=$serialno-%Y-%m-%d-%h-%M-%S.bin
./str2str -in serial://$portname:$baudrate \
          -out file://./$folder/$filename \
          -c ../cmd/INS_OPVT2AHR.cmd &

# GREEN COM2 - A904D6DR
portname=serial/by-id/usb-FTDI_FT232R_USB_UART_A904D6DR-if00-port0
baudrate=460800
filename=$serialno-GNSS-%Y-%m-%d-%h-%M-%S.gps
stty -F /dev/$portname $baudrate 2>/dev/null
./str2str -in serial://$portname:$baudrate \
          -out file://./$folder/$filename &

# GREEN COM3 - A904D6DK
# portname=serial/by-id/usb-FTDI_FT232R_USB_UART_A904D6DK-if00-port0
# baudrate=115200
# stty -F /dev/$portname $baudrate 2>/dev/null
# ./str2str -in ntrip://inertial:sensor22@us.inertiallabs.com:33101/roof \
#           -out serial:://$portname:$baudrate &

read
sudo killall str2str
echo "Done."