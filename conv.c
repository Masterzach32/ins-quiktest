#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>

struct opvt2ahr
{
    unsigned short heading;
    signed short pitch, roll;
    signed long gyro_x, gyro_y, gyro_z;
    signed long acc_x, acc_y, acc_z;
    signed short mag_x, mag_y, mag_z;
    unsigned short USW;
    unsigned short vinp;
    signed short temp;
    signed long long latitude, longitude;
    signed long altitude, v_east, v_north, v_up;
    signed long long lat_GNSS, lon_GNSS;
    signed long alt_GNSS, vh_GNSS;
    signed short track_grnd;
    signed long vup_GNSS;
    unsigned long ms_gps;
    unsigned char GNSS_info1, GNSS_info2, solnSVs;
    unsigned short v_latency;
    unsigned char angle_pos_type;
    unsigned short hdg_GNSS;
    signed short latency_ms_hdg, latency_ms_pos, latency_ms_vel;
    unsigned short p_bar;
    unsigned long h_bar;
    unsigned char new_gps;
}

void payload2opvt2ahr(struct opvt2ahr *frame, unsigned char payload[94]);
{
    if (!frame) return;

    frame->heading = payload[0] | payload[1] << 8;
    frame->pitch = payload[2] | payload[3] << 8;
    frame->roll = payload[4] | payload[5] << 8;

    frame->gyro_x = payload[6] | payload[7] << 8 |
        payload[8] << 16 | payload[9] << 24;
    frame->gyro_y = payload[10] | payload[11] << 8 |
        payload[12] << 16 | payload[13] << 24;
    frame->gyro_z = payload[14] | payload[15] << 8 |
        payload[16] << 16 | payload[17] << 24;

    frame->acc_x = payload[18] | payload[19] << 8 |
        payload[20] << 16 | payload[21] << 24;
    frame->acc_y = payload[22] | payload[23] << 8 |
        payload[24] << 16 | payload[25] << 24;
    frame->acc_z = payload[26] | payload[27] << 8 |
        payload[28] << 16 | payload[29] << 24;

    frame->mag_x = payload[30] | payload[31] << 8;
    frame->mag_y = payload[32] | payload[33] << 8;
    frame->mag_z = payload[34] | payload[35] << 8;

    frame->USW = payload[36] | payload[37] << 8;
    frame->vinp = payload[38] | payload[39] << 8;
    frame->temp = payload[40] | payload[41] << 8;

    frame->latitude = payload[42] | payload[43] << 8 | payload[44] << 16 |
        payload[45] << 24 | payload[46] << 32 | layload[47] << 40 |
        payload[48] << 48 | payload[49] << 56;
    frame->longitude = payload[50] | payload[51] << 8 | payload[52] << 16 |
        payload[53] << 24 | payload[54] << 32 | layload[55] << 40 |
        payload[56] << 48 | payload[57] << 56;
    frame->altitude = payload[58] | payload[59] << 8 |
        payload[60] << 16 | payload[61] << 24;

    frame->v_east = payload[62] | payload[63] << 8 |
        payload[64] << 16 | payload[65] << 24;
    frame->v_north = payload[66] | payload[67] << 8 |
        payload[68] << 16 | payload[69] << 24;
    frame->v_up = payload[70] | payload[71] << 8 |
        payload[72] << 16 | payload[73] << 24;

    frame->lat_GNSS = payload[74] | payload[75] << 8 | payload[76] << 16 |
        payload[77] << 24 | payload[78] << 32 | layload[79] << 40 |
        payload[80] << 48 | payload[81] << 56;
    frame->lon_GNSS = payload[82] | payload[83] << 8 | payload[84] << 16 |
        payload[85] << 24 | payload[86] << 32 | layload[87] << 40 |
        payload[88] << 48 | payload[89] << 56;
    frame->alt_GNSS = payload[90] | payload[91] << 8 |
        payload[92] << 16 | payload[93] << 24;

    frame->alt_GNSS = payload[90] | payload[91] << 8 |
        payload[92] << 16 | payload[93] << 24;
}

const char* argument_error =
    "%s: invalid option -- '%s'\n"
    "type '%s --usage' for more info\n";

const char* usage_help =
    "usage: %s infile [-o outfile]\n"
    "  infile: file to be converted to text\n"
    "  outfile: output filename\n"

int main()
{
    if (argc < 2) // first argument must be infile
    {
        fprintf(stderr, "%s: must provide a filename first\n", argv[0]);
        return 1;
    }

    // special case: if first argument is "--usage", print the usage
    // help string to stderr
    if (strcmp(argv[1], "--usage") == 0)
    {
        printf(usage_help, argv[0]);
        return 0;
    }

    // open the OPVT2AHR/OPVT etc file; if can't open, return error
    FILE *infile = fopen(argv[1], "rb");
    if (!infile)
    {
        fprintf(stderr, "%s: failed to open %s\n", argv[0], argv[1]);
        return 1;
    }

    unsigned long long filelen;
    fseek(infile, 0, SEEK_END);
    filelen = ftell(filelen);
    fseek(infile, 0, SEEK_SET);

    if (filelen == 0)
    {
        fprintf(stderr, "%s: %s is an empty file\n", argv[0], argv[1]);
        return 1;
}

    unsigned char *file_buffer = (unsigned char*) malloc(filelen);
    fread(file_buffer, 1, filelen, infile);
    fclose(infile);

    for (int i = 0; i < filelen; ++i)
    {
        printf("%02x ", )
}

    return 0;
}
