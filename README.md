# ins-quiktest

This repository is a testing tool for the Inertial Labs, Inc. Inertial Navigation System.
It is intended to run on several Raspberry Pi linux computers: one master device, and arbitrarily many slaves.
The master device takes data from a ground truth reference solution (currently a NovAtel SPAN),
while the slaves configure and collect data from IL INS units.

This README provides information about the project structure and the functionality of its many modules.

## app/str2str

The app/ directory stores binary executables. The only file in this directory tracked by git is app/str2str,
a precombiled binary for which the source code is not included. str2str is part of RTKLIB, and is used to move
binary traffic from point A to point B, whether the points be serial devices, NTRIP servers, files, etc.

## cmd/

All of the tool's ASCII and hex commands are kept in the cmd/ directory. These files describe discrete commands
to be sent to the INS, SPAN, or other device, and are usually used in tandem with str2str.

## config/

Bash variable configuration files are saved here. Files in config/ describe hardware
serial port configurations for a specific machine, and override certain default
variables described further down. These files allow COM ports to be properly mapped
onto their device names, found in /dev/serial/by-id/.

## sample/

If this directory exists, it contains past test data saved for later use.

## src/ilconv.cpp

This C++ source file describes a binary to text converter for the INS OPVT2AHR format.
It is used following a driving test to convert raw binary traffic to a report of experiment,
the format of which is comparable to that produced by the official INS Demo software.
However, this program cannot parse INS Demo logs, as the demo strips the data frames of
their sync bytes and headers; ilconv can only parse raw traffic. Furthermore, it expects
the binary traffic to contain, in order: the 10 byte ACK, a short OR extended initial
alignment data frame, and then arbitrarily many OPVT2AHR packets. It is somewhat tolerant
of skips and checksum mismatches.

This converter is also capable of applying a PV offset retroactively. For example, one might
use issue `app/ilconv data/log.bin --pvoff 1 2 3` to apply a PV offset of <1, 2, 3> meters
to an OPVT2AHR binary log and convert it to text.

This application uses the Eigen linear algebra library.

See also `app/ilconv --usage`.

## src/ldprm.c

src/ldprm.c is a C source file designed to read and write to an Inertial Navigation
System's parameters which are accessible using the LoadINSPar command described in the INS ICD.
Additionally the application can detect and change the COM1 baudrate.
Example usage is described below. (Assume for a moment the INS COM1 device is `/dev/ttyUSB0`.)

To change the baudrate to B115200:
`app/ldprm /dev/ttyUSB0 --baud 115200`

To set the lever arm to <1.1, 3, -2.4>:
`app/ldprm /dev/ttyUSB0 --lever 1.1 3 -2.4`

To set the initial alignment time to 6 seconds, and update rate to 100 Hz:
`app/ldprm /dev/ttyUSB0 --init 6 --rate 100`

See also `app/ldprm --usage`.

## src/nconv.c

This is the NovAtel OEM7 format binary to text converter. Its usage is identical to that of ilconv,
save that it does not include a PV offset feature. It is capable of parsing the following logs:
INSPVAB, BESTPOSB, BESTGNSSPOSB, and RTKPOSB.

Currently the application sequesters all INSPVAA ASCII logs to their own file (.ins), which all the
POSA logs going to another file (.pos). At the time of writing this is an undesirable feature, but
is an artifact of poor extensibility of passfail.m, which can only read INSPVAA logs.

See also `app/nconv --usage`.

## src/opvt.c

This file is an experimental OPVT binary to text converter for INS binary logs, though it is not
currently used and no guarantees are made as to its proper functionality. Usage syntax is
identical to ilconv, though again without the PV offset capability.

## .project

Used to signal to scripts and applications the location of the root project directory,
since here is no guarantee as to the name of the repository parent directory.
This enables the safe use of relative filepaths.

## Makefile

Enumerates make recipes for the binary executables which will be placed into app/,
and for which sources files can be found in src/.

## global.conf

This file declares all Bash variables which will be shared between the master and slave
devices; these include COM port baudrates, network hostnames, logins, terminal colors,
master enable/disable switches, etc.

The user should do the vast majority of configuring the test tool only by editing this file.
A short description of the contained variables follows.

A note: for all of these arrays, elements of the same index describe the same device.
Note that node #0 is always the master. For example, node #2's name can be found in `COLOR[2]`,
login address can be found in `LOGIN[2]`, its enable status in `ENABLE[2]`, and the baudrate of its
COM2 port in `BPS_COM2[2]`.

#### NUMBER_OF_NODES
This describes the number of devices involved in the network, including the master device.
Ensure `NUMBER_OF_NODES` is not greater than the number of elements in any of the arrays below.
#### COLORS
This formerly contained literal colors, but now is just a list of names for humans to call the
master and slave devices by.
#### UNAME
The login username for each of the Raspberry Pi devices.
#### LOGIN
The login address of each Pi. It is recommended to use the .local domain, as this is robust
to changes in IP address due to network alterations.
#### ENABLE
This is the master enable/disable switch for each device. 0 indicates OFF, greater than 0
indicates ON.
#### BPS_COM1, BPS_COM2, BPS_COM3
These arrays describe the baudrates for every COM port in the system; there is no error checking
for these arrays, and incorrect or misspelled baudrates will be used verbatim. Zeroes in these
arrays are semantically identical to a disable of that particular port.
#### RS_422
This is a switch similar to `ENABLE`; this enables the RS-422 interface for a given node. 0 indicates
RS-232, and any number greater than 0 indictates RS-422.
#### CMD_COM1, CMD_COM2
These arrays list the commands to be sent to specific COM ports to begin the test. The files
listed here will be preprended with "cmd/", as all command files are stored in the cmd/ directory.
As an example, if one wanted to send the command described in cmd/INS_OPVT.cmd to COM1 on the the 3rd node,
the assignment `CMD_COM1[3]="INS_OPVT.cmd"` will suffice.
#### LX, LY, LZ
The IMU to antenna offset of the SPAN and INS units are all described here, in meters. The unit vectors
are as follows: x points towards the right side of the vehicle, y towards the front, and z upwards,
forming a right handed coordinate system. These values are loaded into each INS before the test,
and to apply PV offsets to the INS logs; note that the SPAN's internal IMU offset is set by the command
cmd/SPAN-start.cmd, and may differ from the elements `LX[0]`, `LY[0]`, and `LZ[0]`.
#### SP, red, green, yellow, grey, end
The remaining variables are cosmetic, and enumerate colors and spacing for the console output.

## manifest.txt

This file lists all files which will be copied from the master device to each of the slaves
before every test; files and directories can be delimited by newlines or spaces, and directories
should be followed by a forward slash, as in cmd/ or src/.

## master.sh

This script is how the sausage is made.
It integrates all of the modules described in other sections into a cohesive program,
and is meant to be run only from the master device. A semi-accurate progression of steps
is as follows.

1. Load global.conf and the master local configuration file from config/.
1. Copy files described in manifest.txt from master directory to each slave.
1. Initiate slave script on each slave device.
1. Wait for user to terminate test; periodically assess the health of the slaves' data.
1. Terminate the test and collect data from slave data directories.
1. Reorganize, rename, convert, and analyze INS data compared to SPAN reference.

## passfail.m

An octave-cli script used to compile a report comparing the accuracy of the INS vs the
ground truth SPAN solution. The report considers position and orientation accuracy
in the regime of dynamic navigation.

## ping.sh

A utility application which looks for nodes on the LAN, using the hostnames provided in global.conf.

## slave.sh

(This isn't as bad as it sounds, trust me)

This script is run by each slave device, initiated by the master device via SSH.
The steps it takes are loosely described below.

1. Load global.conf and the device-specific local configuration file from config/.
1. If enabled, apply settings to the INS over COM1 using app/ldprm.
1. Initiate data collection over COM1, then COM2, then COM3.
1. Exit and return an error code to the master if any errors are encountered.
