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

Currently the application sequesters all INSPVAA ASCII logs to their own file (*.ins), which all the
POSA logs going to another file (*.pos). At the time of writing this is an undesirable feature, but
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

The user should do the vast majority of configuring the test tool <b>only</b> by editing this file.

## manifest.txt

This file lists all files which will be copied from the master device to each of the slaves
before every test; files and directories can be delimited by newlines or spaces, and directories
should be followed by a forward slash, as in `cmd/` or `src/`.

## master.sh

This script is how the sausage is made.

## passfail.m

This script does math.

## ping.sh

This pings ur friends.

## slave.sh

This isn't as bad as it sounds, trust me
