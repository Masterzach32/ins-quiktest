#ifndef LDPRM_H
#define LDPRM_H

struct short_prm
{
    unsigned short data_rate;
    unsigned short align_time;
    signed long mag_dec;
    signed long latitude, longitude, altitude;
    unsigned char year, month, day;
    signed short align_angles[3];
    signed short mount[3];
    signed short lever[3];
    unsigned char altitude_byte;
    char device_name[9];
    unsigned char baro_altimeter;
};

void payload2struct(struct short_prm *prm, const unsigned char payload[60]);

void struct2payload(const struct short_prm *prm, unsigned char payload[60]);

#endif // LDPRM_H
