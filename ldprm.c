#include <stdio.h>
#include <inttypes.h>
#include <malloc.h>
#include <string.h>

#include "ldprm.h"

void payload2struct(struct short_prm *prm, const unsigned char payload[60])
{
    if (!prm) return;

    memcpy(prm->device_name, payload + 50, 8);

    // remember: LSB is first, later bytes must be shifted up
    prm->data_rate = payload[0] | (payload[1] << 8);
    prm->align_time = payload[2] | (payload[3] << 8);
    prm->mag_dec = payload[4] | (payload[5] << 8) |
        (payload[6] << 16) | (payload[7] << 24);

    prm->latitude  = payload[8] | (payload[9] << 8) |
        (payload[10] << 16) | (payload[11] << 24);
    prm->longitude = payload[12] | (payload[13] << 8) |
        (payload[14] << 16) | (payload[15] << 24);
    prm->altitude  = payload[16] | (payload[17] << 8) |
        (payload[18] << 16) | (payload[19] << 24);

    prm->year = payload[20], prm->month = payload[21], prm->day = payload[22];

    prm->align_angles[0] = payload[23] | (payload[24] << 8);
    prm->align_angles[1] = payload[25] | (payload[26] << 8);
    prm->align_angles[2] = payload[27] | (payload[28] << 8);

    prm->mount[0] = payload[29] | (payload[30] << 8);
    prm->mount[1] = payload[31] | (payload[32] << 8);
    prm->mount[2] = payload[33] | (payload[34] << 8);
    
    prm->lever[0] = payload[35] | (payload[36] << 8);
    prm->lever[1] = payload[37] | (payload[38] << 8);
    prm->lever[2] = payload[39] | (payload[40] << 8);

    prm->altitude_byte = payload[41];
    prm->baro_altimeter = payload[58];
}

void struct2payload(const struct short_prm *prm, unsigned char payload[60])
{
    if (!prm) return;

    memcpy(payload + 50, prm->device_name, 8);

    // remember: LSB is first, later bytes must be shifted up
    payload[0] = prm->data_rate & 0xFF;
    payload[1] = prm->data_rate & (0xFF << 8);
    
    payload[2] = prm->align_time & 0xFF;
    payload[3] = prm->align_time & (0xFF << 8);

    payload[4] = prm->mag_dec & 0xFF;
    payload[5] = prm->mag_dec & (0xFF << 8);
    payload[6] = prm->mag_dec & (0xFF << 16);
    payload[7] = prm->mag_dec & (0xFF << 24);

    payload[8] = prm->latitude & 0xFF;
    payload[9] = prm->latitude & (0xFF << 8);
    payload[10] = prm->latitude & (0xFF << 16);
    payload[11] = prm->latitude & (0xFF << 24);

    payload[12] = prm->longitude & 0xFF;
    payload[13] = prm->longitude & (0xFF << 8);
    payload[14] = prm->longitude & (0xFF << 16);
    payload[15] = prm->longitude & (0xFF << 24);

    payload[16] = prm->altitude & 0xFF;
    payload[17] = prm->altitude & (0xFF << 8);
    payload[18] = prm->altitude & (0xFF << 16);
    payload[19] = prm->altitude & (0xFF << 24);

    payload[20] = prm->year;
    payload[21] = prm->month;
    payload[22] = prm->day;

    payload[23] = prm->align_angles[0] & 0xFF;
    payload[24] = prm->align_angles[0] & (0xFF << 8);

    payload[25] = prm->align_angles[1] & 0xFF;
    payload[26] = prm->align_angles[1] & (0xFF << 8);

    payload[27] = prm->align_angles[2] & 0xFF;
    payload[28] = prm->align_angles[2] & (0xFF << 8);

    payload[29] = prm->mount[0] & 0xFF;
    payload[30] = prm->mount[0] & (0xFF << 8);

    payload[31] = prm->mount[1] & 0xFF;
    payload[32] = prm->mount[1] & (0xFF << 8);

    payload[33] = prm->mount[2] & 0xFF;
    payload[34] = prm->mount[2] & (0xFF << 8);

    payload[35] = prm->lever[0] & 0xFF;
    payload[36] = prm->lever[0] & (0xFF << 8);

    payload[37] = prm->lever[1] & 0xFF;
    payload[38] = prm->lever[1] & (0xFF << 8);

    payload[39] = prm->lever[2] & 0xFF;
    payload[40] = prm->lever[2] & (0xFF << 8);

    payload[41] = prm->altitude_byte;
    payload[58] = prm->baro_altimeter;
}

const unsigned char paylen = 60, numel = 27, wordlens[] =
    {2, 2, 4, 4, 4, 4, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2,
     1, 1, 1, 2, 2, 2, 8, 1, 1};

int main(int argc, char** argv)
{
    unsigned long filelen;

    if (argc < 2)
    {
        fprintf(stderr, "usage: must provide a file to read\n");
        return 1;
    }

    FILE *fileptr = fopen(argv[1], "rb");
    if (!fileptr)
    {
        fprintf(stderr, "error: invalid filename\n");
        return 2;
    }

    fseek(fileptr, 0, SEEK_END);
    filelen = ftell(fileptr);
    rewind(fileptr);

    if (filelen != 68)
    {
        fprintf(stderr, "error: file is not 68 bytes\n");
        return 3;
    }

    unsigned char payload[60];
    fseek(fileptr, 6, SEEK_SET); // skip the first 6 bytes
    fread(payload, 1, paylen, fileptr);
    fclose(fileptr);

    /*
    unsigned char rptr = 0, limit = 0;
    for (unsigned char el = 0; el < numel; ++el)
    {
        unsigned char size = wordlens[el];
        limit += size;
        while (rptr < limit)
        {
            printf("%02x ", payload[rptr]);
            ++rptr;
        }
        printf("\n");
    }
    */

    struct short_prm dat;
    payload2struct(&dat, payload);
    dat.lever[0] = 1001;
    struct2payload(&dat, payload);
    payload2struct(&dat, payload);

    printf("device name: %s\n", dat.device_name); 
    printf("data rate: %hu\n", dat.data_rate);
    printf("initial alignment time: %hu\n", dat.align_time);
    printf("magnetic declination: %d\n", dat.mag_dec);
    printf("position: %d, %d, %d\n", dat.latitude, dat.longitude, dat.altitude);
    printf("date: %hhu/%hhu/%hhu\n", dat.year, dat.month, dat.day);
    printf("alignment angles: <%hi, %hi, %hi>\n",
        dat.align_angles[0], dat.align_angles[1], dat.align_angles[2]);
    printf("mounting lever: <%hi, %hi, %hi>\n",
        dat.mount[0], dat.mount[1], dat.mount[2]);
    printf("lever arm: <%hi, %hi, %hi>\n",
        dat.lever[0], dat.lever[1], dat.lever[2]);
    printf("altitude output: %hhu\n", dat.altitude_byte);
    printf("baro enabled: %hhu\n", dat.baro_altimeter);

    FILE *outfile = fopen("outfile.bin", "wb");
    if (!outfile)
    {
        fprintf(stderr, "error: failed to open output file\n");
        return 5;
    }  

    const unsigned char command[] = {0xAA, 0x55, 0, 0, 7, 0, 0x40, 0x47, 0};
    fwrite(command, 1, sizeof(command), outfile);
    fwrite(payload, 1, sizeof(payload), outfile);
    fflush(outfile);
    fclose(outfile);

    return 0;
}

// READ:  AA 55 00 00 07 00 41 48 00
// WRITE: AA 55 00 00 07 00 40 47 00 
