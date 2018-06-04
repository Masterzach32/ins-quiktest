#/bin/sh!

if [ -z "$1" ]; then
echo "usage: provide command file to load"
exit
fi

# GREEN COM1 - A904D6DE
portname=serial/by-id/usb-FTDI_FT232R_USB_UART_A904D6DE-if00-port0
baudrate=460800
stty -F /dev/$portname $baudrate 2>/dev/null
./str2str -in serial://$portname:$baudrate \
          -out file://./response.bin \
          -c $1 &

read

killall str2str
sleep 1

echo ""
echo "Done."
