unlogall true
setimuorientation 5
vehiclebodyrotation 0 180 0
setimutoantoffset 0.07 -0.08 0.44 0.02 0.02 0.02
undulation user 0.0
interfacemode com3 auto novatel

log interfacemodeb once
log serialconfigb once
log bestgnssposb ontime .2
log bestleverarmb onchanged
log vehiclebodyrotationb onchanged
log rangecmpb ontime 0.2
log gpsephemb onnew
log gloephemerisb onnew
log rawimusxb onnew
log inspvaxb ontime 0.05
log rtkposb ontime 0.05

@
unlogall
