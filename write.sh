#/bin/sh!

if [ -z "$1" ]; then
echo "usage: provide an abreviated .prm file to load"
exit
fi

cp ../cmd/INS_LOADPAR.cmd ../cmd/INS_LOADPAR_BLOCK.cmd
hexdump -ve '1/1 "%.2x "' $1 >> ../cmd/INS_LOADPAR_BLOCK.cmd
echo -n $(tr -d '\n' < ../cmd/INS_LOADPAR_BLOCK.cmd) > ../cmd/INS_LOADPAR_BLOCK.cmd

exit

# GREEN COM1 - A904D6DE
portname=serial/by-id/usb-FTDI_FT232R_USB_UART_A904D6DE-if00-port0
baudrate=460800
stty -F /dev/$portname $baudrate 2>/dev/null
./str2str -in serial://$portname:$baudrate \
          -c ../cmd/INS_LOADPAR_BLOCK.cmd \
	  1>/dev/null 2>/dev/null &

sleep 1
killall str2str

echo "Done."
