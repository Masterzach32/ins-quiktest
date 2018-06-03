#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

struct short_prm
{
    unsigned short data_rate;
    unsigned short align_time;
    long mag_dec;
    long latitude, longitude, altitude;
    unsigned char year, month, day;
    short align_angles[3];
    short mount[3];
    short lever[3];
    unsigned char altitude_byte;
    char device_name[9];
    unsigned char baro_altimeter;
};

void payload2struct(struct short_prm *prm, const unsigned char payload[60])
{
    if (!prm) return;

    memset(prm->device_name, 0, sizeof(prm->device_name));
    memcpy(prm->device_name, payload + 50, 8);

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
    payload[1] = prm->data_rate >> 8;
    
    payload[2] = prm->align_time & 0xFF;
    payload[3] = prm->align_time >> 8;

    payload[4] = prm->mag_dec & 0xFF;
    payload[5] = prm->mag_dec >> 8;
    payload[6] = prm->mag_dec >> 16;
    payload[7] = prm->mag_dec >> 24;

    payload[8] = prm->latitude & 0xFF;
    payload[9] = prm->latitude >> 8;
    payload[10] = prm->latitude >> 16;
    payload[11] = prm->latitude >> 24;

    payload[12] = prm->longitude & 0xFF;
    payload[13] = prm->longitude >> 8;
    payload[14] = prm->longitude >> 16;
    payload[15] = prm->longitude >> 24;

    payload[16] = prm->altitude & 0xFF;
    payload[17] = prm->altitude >> 8;
    payload[18] = prm->altitude >> 16;
    payload[19] = prm->altitude >> 24;

    payload[20] = prm->year;
    payload[21] = prm->month;
    payload[22] = prm->day;

    payload[23] = prm->align_angles[0] & 0xFF;
    payload[24] = prm->align_angles[0] >> 8;

    payload[25] = prm->align_angles[1] & 0xFF;
    payload[26] = prm->align_angles[1] >> 8;

    payload[27] = prm->align_angles[2] & 0xFF;
    payload[28] = prm->align_angles[2] >> 8;

    payload[29] = prm->mount[0] & 0xFF;
    payload[30] = prm->mount[0] >> 8;

    payload[31] = prm->mount[1] & 0xFF;
    payload[32] = prm->mount[1] >> 8;

    payload[33] = prm->mount[2] & 0xFF;
    payload[34] = prm->mount[2] >> 8;

    payload[35] = prm->lever[0] & 0xFF;
    payload[36] = prm->lever[0] >> 8;

    payload[37] = prm->lever[1] & 0xFF;
    payload[38] = prm->lever[1] >> 8;

    payload[39] = prm->lever[2] & 0xFF;
    payload[40] = prm->lever[2] >> 8;

    payload[41] = prm->altitude_byte;
    payload[58] = prm->baro_altimeter;
}

void print_struct(struct short_prm prm)
{
    printf("device name: %s\n", prm.device_name); 
    printf("data rate: %hu Hz\n", prm.data_rate);
    printf("initial alignment time: %hu seconds\n", prm.align_time);
    printf("magnetic declination: %0.2f degrees\n", prm.mag_dec/100.0);
    printf("position: %0.5f, %0.5f, %0.2f\n",
            prm.latitude/10000000.0,
            prm.longitude/10000000.0,
            prm.altitude/100.0);
    printf("date: %hu/%hhu/%hhu\n", 2000 + prm.year, prm.month, prm.day);
    printf("alignment angles: <%0.2f, %0.2f, %0.2f> degrees\n",
        prm.align_angles[0]/100.0,
        prm.align_angles[1]/100.0,
        prm.align_angles[2]/100.0);
    printf("mounting lever: <%0.2f, %0.2f, %0.2f> meters\n",
        prm.mount[0]/100.0, prm.mount[1]/100.0, prm.mount[2]/100.0);
    printf("lever arm: <%0.2f, %0.2f, %0.2f> meters\n",
        prm.lever[0]/100.0, prm.lever[1]/100.0, prm.lever[2]/100.0);
    printf("altitude output: %hhu\n", prm.altitude_byte);
    printf("baro enabled: %hhu\n", prm.baro_altimeter);
}

const char* usage_help = 
    "usage: %s infile [-o outfile] [-x swapfile] [-p] [-r dr] [-i s] [-l lx ly lz] [-a h p r]\n"
    "  infile: name of file containing ReadINSPar data\n"
    "  outfile: optional name of output file to contain LoadINSPar command\n"
    "  swapfile: optional name of imitation ReadINSPar file\n"
    "  [-p]: print INS params in plaintext\n"
    "  dr: data rate of INS output, in Hz; must be multiple of 200 Hz\n"
    "  s: INS initial alignment time in seconds\n"
    "  lx ly lz: offset from imu to antenna, in meters\n"
    "  h p r: angle offset from vehicle orientation, in degrees\n";

const char valid_rates[] = {5, 10, 20, 25, 40, 50, 100, 200};

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        fprintf(stderr, usage_help, argv[0]);
        return 1;
    }

    FILE *fileptr = fopen(argv[1], "rb");
    if (!fileptr)
    {
        fprintf(stderr, "%s: invalid filename \"%s\"\n", argv[0], argv[1]);
        return 2;
    }

    unsigned char rate_flag = 0;
    unsigned char init_flag = 0;
    unsigned char lever_flag = 0;
    unsigned char angle_flag = 0;
    unsigned char output_flag = 0;
    unsigned char swap_flag = 0;
    unsigned char print_flag = 0;

    unsigned char rate_input;
    unsigned char init_input;
    double lever_input[3];
    double angle_input[3];
    char* outfn;
    char* swapfn;

    for (int i = 2; i < argc; ++i)
    {
        if (strcmp(argv[i], "-o") == 0)
        {
            if (argc < i + 2)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 3;
            }
            outfn = (char*) calloc(strlen(argv[i+1]) + 1, 1);
            strcpy(outfn, argv[i+1]);
            output_flag = 1;
        }
        else if (strcmp(argv[i], "-x") == 0)
        {
            if (argc < i + 2)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 3;
            }
            swapfn = (char*) calloc(strlen(argv[i+1]) + 1, 1);
            strcpy(swapfn, argv[i+1]);
            swap_flag = 1;
        }
        else if (strcmp(argv[i], "-r") == 0)
        {
            if (argc < i + 2)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 3;
            }
            rate_input = atoi(argv[i+1]);
            unsigned char num_of_rates = sizeof(valid_rates)/sizeof(valid_rates[0]);
            for (int i = 0; i < num_of_rates; ++i)
            {
                if (rate_input == valid_rates[i])
                {
                    rate_flag = 1;
                }
            }
            if (!rate_flag)
            {
                fprintf(stderr, "%s: valid data rates are: ", argv[0]);
                for (int i = 0; i < num_of_rates; ++i)
                {
                    fprintf(stderr, "%hhu", valid_rates[i]);
                    if (i < 7) fprintf(stderr, ", ");
                }
                fprintf(stderr, "\n");
                return 3;
            }
        }
        else if (strcmp(argv[i], "-i") == 0)
        {
            if (argc < i + 2)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 3;
            }
            init_flag = 1;
            init_input = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "-l") == 0)
        {
            if (argc < i + 4)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 3;
            }
            lever_flag = 1;
            lever_input[0] = atof(argv[i+1]);
            lever_input[1] = atof(argv[i+2]);
            lever_input[2] = atof(argv[i+3]);
        }
        else if (strcmp(argv[i], "-a") == 0)
        {
            if (argc < i + 4)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 4;
            }
            angle_flag = 1;
            angle_input[0] = atof(argv[i+1]);
            angle_input[1] = atof(argv[i+2]);
            angle_input[2] = atof(argv[i+3]);
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            print_flag = 1;
        }
    }

    fseek(fileptr, 0, SEEK_END);
    unsigned long filelen = ftell(fileptr);
    fseek(fileptr, 0, SEEK_SET);

    if (filelen != 68)
    {
        fprintf(stderr, "%s: file is not a ReadINSPar response\n", argv[0]);
        return 4;
    }

    unsigned char payload[60];
    fseek(fileptr, 6, SEEK_SET);
    fread(payload, 60, 1, fileptr);
    fclose(fileptr);

    struct short_prm dat;
    payload2struct(&dat, payload);

    if (rate_flag) dat.data_rate = rate_input;
    if (init_flag) dat.align_time = init_input;
    if (lever_flag)
    {
        dat.lever[0] = lever_input[0]*100;
        dat.lever[1] = lever_input[1]*100;
        dat.lever[2] = lever_input[2]*100;
    }
    if (angle_flag)
    {
        dat.align_angles[0] = angle_input[0]*100;
        dat.align_angles[1] = angle_input[1]*100;
        dat.align_angles[2] = angle_input[2]*100;
    }
    if (print_flag) print_struct(dat);
    struct2payload(&dat, payload);

    const unsigned char command[] = {0xAA, 0x55, 0, 0, 7, 0, 0x40, 0x47, 0};
    const unsigned char header[] = {0xAA, 0x55, 1, 0x41, 0x42, 0};

    unsigned short sum = 0;
    for (int i = 2; i < 6; ++i)  sum += header[i];
    for (int i = 0; i < 60; ++i) sum += payload[i];

    unsigned char checksum[] = {sum & 0xFF, sum >> 8};

    if (output_flag)
    {
        FILE *outfile = fopen(outfn, "wb");
        if (!outfile)
        {
            fprintf(stderr, "%s: failed to open output file\n", argv[0]);
            return 5;
        }
        fwrite(command, sizeof(command), 1, outfile);
        fwrite(payload, sizeof(payload), 1, outfile);
        fflush(outfile);
        fclose(outfile);
    }
    if (swap_flag)
    {
        FILE *swapfile = fopen(swapfn, "wb");
        if (!swapfile)
        {
            fprintf(stderr, "%s: failed to open swap file\n", argv[0]);
            return 6;
        }
        fwrite(header, 1, sizeof(header), swapfile);
        fwrite(payload, 1, sizeof(payload), swapfile);
        fwrite(checksum, 1, sizeof(checksum), swapfile);
        fflush(swapfile);
        fclose(swapfile);
    }

    return 0;
}

// READ:  AA 55 00 00 07 00 41 48 00
// WRITE: AA 55 00 00 07 00 40 47 00 
