#/bin/sh!

# GREEN COM1 - A904D6DE
portname=serial/by-id/usb-FTDI_FT232R_USB_UART_A904D6DE-if00-port0
baudrate=460800
filename=INS-%Y-%m-%d-%h-%M-%S.bin
stty -F /dev/$portname $baudrate 2>/dev/null
./str2str -in serial://$portname:$baudrate \
          -out file://$filename \
          -c readprm.cmd \
	  1>/dev/null 2>/dev/null &

sleep 1
killall str2str

echo "Done."
