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
};

void payload2opvt2ahr(struct opvt2ahr *frame, unsigned char payload[129])
{
    if (!frame) return;

    frame->heading = payload[0] | (payload[1] << 8);
    frame->pitch = payload[2] | (payload[3] << 8);
    frame->roll = payload[4] | (payload[5] << 8);

    frame->gyro_x = payload[6] | (payload[7] << 8) |
        (payload[8] << 16) | (payload[9] << 24);
    frame->gyro_y = payload[10] | (payload[11] << 8) |
        (payload[12] << 16) | (payload[13] << 24);
    frame->gyro_z = payload[14] | (payload[15] << 8) |
        (payload[16] << 16) | (payload[17] << 24);

    frame->acc_x = payload[18] | (payload[19] << 8) |
        (payload[20] << 16) | (payload[21] << 24);
    frame->acc_y = payload[22] | payload[23] << 8 |
        (payload[24] << 16) | (payload[25] << 24);
    frame->acc_z = payload[26] | (payload[27] << 8) |
        (payload[28] << 16) | (payload[29] << 24);

    frame->mag_x = payload[30] | (payload[31] << 8);
    frame->mag_y = payload[32] | (payload[33] << 8);
    frame->mag_z = payload[34] | (payload[35] << 8);

    frame->USW = payload[36] | (payload[37] << 8);
    frame->vinp = payload[38] | (payload[39] << 8);
    frame->temp = payload[40] | (payload[41] << 8);

    frame->latitude = payload[42] | (payload[43] << 8) |
        (payload[44] << 16) | (payload[45] << 24) |
        ((long long) payload[46] << 32) |
        ((long long) payload[47] << 40) |
        ((long long) payload[48] << 48) |
        ((long long) payload[49] << 56);
    frame->longitude = payload[50] | (payload[51] << 8) |
        (payload[52] << 16) | (payload[53] << 24) |
        ((long long) payload[54] << 32) |
        ((long long) payload[55] << 40) |
        ((long long) payload[56] << 48) |
        ((long long) payload[57] << 56);
    frame->altitude = payload[58] | (payload[59] << 8) |
        (payload[60] << 16) | (payload[61]) << 24;

    frame->v_east = payload[62] | (payload[63] << 8) |
        (payload[64] << 16) | (payload[65] << 24);
    frame->v_north = payload[66] | (payload[67] << 8) |
        (payload[68] << 16) | (payload[69] << 24);
    frame->v_up = payload[70] | (payload[71] << 8) |
        (payload[72] << 16) | (payload[73] << 24);

    frame->lat_GNSS = payload[74] | (payload[75] << 8) |
        (payload[76] << 16) | (payload[77] << 24) |
        ((long long) payload[78] << 32) |
        ((long long) payload[79] << 40) |
        ((long long) payload[80] << 48) |
        ((long long) payload[81] << 56);
    frame->lon_GNSS = payload[82] | (payload[83] << 8) |
        (payload[84] << 16) | (payload[85] << 24) |
        ((long long) payload[86] << 32) |
        ((long long) payload[87] << 40) |
        ((long long) payload[88] << 48) |
        ((long long) payload[89] << 56);
    frame->alt_GNSS = payload[90] | (payload[91] << 8) |
        (payload[92] << 16) | (payload[93] << 24);

    frame->vh_GNSS = payload[94] | (payload[95] << 8) |
        (payload[96] << 16) | (payload[97] << 24);
    frame->track_grnd = payload[98] | (payload[99] << 8);
    frame->vup_GNSS = payload[100] | (payload[101] << 8) |
        (payload[102] << 16) | (payload[103] << 24);

    frame->ms_gps = payload[104] | (payload[105] << 8) |
        (payload[106] << 16) | (payload[107] << 24);
    frame->GNSS_info1 = payload[108];
    frame->GNSS_info2 = payload[109];
    frame->solnSVs = payload[110];
    frame->v_latency = payload[111] | (payload[112] << 8);
    frame->angle_pos_type = payload[113];
    frame->hdg_GNSS = payload[114] | (payload[115] << 8);

    frame->latency_ms_hdg = payload[116] | (payload[117] << 8);
    frame->latency_ms_pos = payload[118] | (payload[119] << 8);
    frame->latency_ms_vel = payload[120] | (payload[121] << 8);

    frame->p_bar = payload[122] | (payload[123] << 8);
    frame->h_bar = payload[124] | (payload[125] << 8) |
        (payload[126] << 16) | (payload[127] << 24);
    frame->new_gps = payload[129];
}

void println_opvt2ahr(struct opvt2ahr *frame)
{
    if (!frame)
    {
        printf("        Heading"
               "          Pitch"
               "           Roll"
               "         Gyro_X"
               "         Gyro_Y"
               "         Gyro_Z"
               "          Acc_X"
               "          Acc_Y"
               "          Acc_Z"
               "         Magn_X"
               "         Magn_Y"
               "         Magn_Z"
               "    Temperature"
               "            Vdd"
               "      USW (L/H)"
               "              Latitude"
               "      Longitude"
               "       Altitude"
               "         V_East"
               "        V_North"
               "           V_Up"
               "       Lat_GNSS"
               "      Long_GNSS"
               "    Height_GNSS"
               "        Hor_spd"
               "        Trk_gnd"
               "        Ver_spd"
               "         ms_gps"
               "    GNSS_info_1"
               "    GNSS_info_2"
               "       #solnSVs"
               "        latency"
               "  anglesPosType"
               "   Heading_GNSS"
               "    Latency_ms_head"
               "     Latency_ms_pos"
               "     Latency_ms_vel"
               "          P_Bar"
               "          H_Bar"
               "        New_GPS\n");
        return;
    }

    printf("%15.2f%15.2f%15.2f",
        frame->heading/100.0, frame->pitch/100.0, frame->roll/100.0);
    printf("%15.5f%15.5f%15.5f",
        frame->gyro_x/1E5f, frame->gyro_y/1E5f, frame->gyro_z/1E5f);
    printf("%15.6f%15.6f%15.6f",
        frame->acc_x/1E6f, frame->acc_y/1E6f, frame->acc_z/1E6f);
    printf("%15.1f%15.1f%15.1f",
        frame->mag_x/10.0, frame->mag_y/10.0, frame->mag_z/10.0);
    printf("%15.1f%15.2f%23hu",
        frame->temp/10.0, frame->vinp/100.0, frame->USW);

    printf("%15.9f%15.9f%15.9f",
        frame->latitude/1E9f, frame->longitude/1E9f, frame->altitude/1E3f);
    printf("%0.2f %0.2f %0.2f ",
        frame->v_east/100.0, frame->v_north/100.0, frame->v_up/100.0);
    printf("%0.8f %0.8f %0.3f ",
        frame->lat_GNSS/1E9f, frame->lon_GNSS/1E9f, frame->alt_GNSS/1E3f);
    printf("%0.2f %0.2f %0.2f ",
        frame->vh_GNSS/100.0, frame->track_grnd/100.0, frame->vup_GNSS/100.0);
    printf("%ld ", frame->ms_gps);
    printf("0x%02x 0x%02x ", frame->GNSS_info1, frame->GNSS_info2);
    printf("%hhu %0.2f %hhu ",
        frame->solnSVs, frame->v_latency/1E3f, frame->angle_pos_type);
    printf("%0.2f %hhu %hhu %hhu ",
        frame->hdg_GNSS/100.0, frame->latency_ms_hdg,
        frame->latency_ms_pos, frame->latency_ms_vel);
    printf("%hu %0.2f %02x\n",
        frame->p_bar*2, frame->h_bar/100.0, frame->new_gps);
}

void opvt2ahr2payload(struct opvt2ahr *frame, unsigned char payload[129])
{
    if (!frame) return;

    payload[0] = frame->heading & 0xFF;
    payload[1] = frame->heading >> 8;
    payload[2] = frame->pitch & 0xFF;
    payload[3] = frame->pitch >> 8;
    payload[4] = frame->roll & 0xFF;
    payload[5] = frame->roll >> 8;

    payload[6] = frame->gyro_x & 0xFF;
    payload[7] = frame->gyro_x >> 8;
    payload[8] = frame->gyro_x >> 16;
    payload[9] = frame->gyro_x >> 24;
    payload[10] = frame->gyro_y & 0xFF;
    payload[11] = frame->gyro_y >> 8;
    payload[12] = frame->gyro_y >> 16;
    payload[13] = frame->gyro_y >> 24;
    payload[14] = frame->gyro_z & 0xFF;
    payload[15] = frame->gyro_z >> 8;
    payload[16] = frame->gyro_z >> 16;
    payload[17] = frame->gyro_z >> 24;

    payload[18] = frame->acc_x & 0xFF;
    payload[19] = frame->acc_x >> 8;
    payload[20] = frame->acc_x >> 16;
    payload[21] = frame->acc_x >> 24;
    payload[22] = frame->acc_y & 0xFF;
    payload[23] = frame->acc_y >> 8;
    payload[24] = frame->acc_y >> 16;
    payload[25] = frame->acc_y >> 24;
    payload[26] = frame->acc_z & 0xFF;
    payload[27] = frame->acc_z >> 8;
    payload[28] = frame->acc_z >> 16;
    payload[29] = frame->acc_z >> 24;

    payload[30] = frame->mag_x & 0xFF;
    payload[31] = frame->mag_x >> 8;
    payload[32] = frame->mag_y & 0xFF;
    payload[33] = frame->mag_y >> 8;
    payload[34] = frame->mag_z & 0xFF;
    payload[35] = frame->mag_z >> 8;

    payload[36] = frame->USW & 0xFF;
    payload[37] = frame->USW >> 8;
    payload[38] = frame->vinp & 0xFF;
    payload[39] = frame->vinp >> 8;
    payload[40] = frame->temp & 0xFF;
    payload[41] = frame->temp >> 8;

    payload[42] = frame->latitude & 0xFF;
    payload[43] = frame->latitude >> 8;
    payload[44] = frame->latitude >> 16;
    payload[45] = frame->latitude >> 24;
    payload[46] = frame->latitude >> 32;
    payload[47] = frame->latitude >> 40;
    payload[48] = frame->latitude >> 48;
    payload[49] = frame->latitude >> 56;
    payload[50] = frame->longitude & 0xFF;
    payload[51] = frame->longitude >> 8;
    payload[52] = frame->longitude >> 16;
    payload[53] = frame->longitude >> 24;
    payload[54] = frame->longitude >> 32;
    payload[55] = frame->longitude >> 40;
    payload[56] = frame->longitude >> 48;
    payload[57] = frame->longitude >> 56;

    return;

    frame->acc_x = payload[18] | (payload[19] << 8) |
        (payload[20] << 16) | (payload[21] << 24);
    frame->acc_y = payload[22] | payload[23] << 8 |
        (payload[24] << 16) | (payload[25] << 24);
    frame->acc_z = payload[26] | (payload[27] << 8) |
        (payload[28] << 16) | (payload[29] << 24);

    frame->mag_x = payload[30] | (payload[31] << 8);
    frame->mag_y = payload[32] | (payload[33] << 8);
    frame->mag_z = payload[34] | (payload[35] << 8);

    frame->USW = payload[36] | (payload[37] << 8);
    frame->vinp = payload[38] | (payload[39] << 8);
    frame->temp = payload[40] | (payload[41] << 8);

    frame->latitude = payload[42] | (payload[43] << 8) |
        (payload[44] << 16) | (payload[45] << 24) |
        ((long long) payload[46] << 32) |
        ((long long) payload[47] << 40) |
        ((long long) payload[48] << 48) |
        ((long long) payload[49] << 56);
    frame->longitude = payload[50] | (payload[51] << 8) |
        (payload[52] << 16) | (payload[53] << 24) |
        ((long long) payload[54] << 32) |
        ((long long) payload[55] << 40) |
        ((long long) payload[56] << 48) |
        ((long long) payload[57] << 56);
    frame->altitude = payload[58] | (payload[59] << 8) |
        (payload[60] << 16) | (payload[61]) << 24;

    frame->v_east = payload[62] | (payload[63] << 8) |
        (payload[64] << 16) | (payload[65] << 24);
    frame->v_north = payload[66] | (payload[67] << 8) |
        (payload[68] << 16) | (payload[69] << 24);
    frame->v_up = payload[70] | (payload[71] << 8) |
        (payload[72] << 16) | (payload[73] << 24);

    frame->lat_GNSS = payload[74] | (payload[75] << 8) |
        (payload[76] << 16) | (payload[77] << 24) |
        ((long long) payload[78] << 32) |
        ((long long) payload[79] << 40) |
        ((long long) payload[80] << 48) |
        ((long long) payload[81] << 56);
    frame->lon_GNSS = payload[82] | (payload[83] << 8) |
        (payload[84] << 16) | (payload[85] << 24) |
        ((long long) payload[86] << 32) |
        ((long long) payload[87] << 40) |
        ((long long) payload[88] << 48) |
        ((long long) payload[89] << 56);
    frame->alt_GNSS = payload[90] | (payload[91] << 8) |
        (payload[92] << 16) | (payload[93] << 24);

    frame->vh_GNSS = payload[94] | (payload[95] << 8) |
        (payload[96] << 16) | (payload[97] << 24);
    frame->track_grnd = payload[98] | (payload[99] << 8);
    frame->vup_GNSS = payload[100] | (payload[101] << 8) |
        (payload[102] << 16) | (payload[103] << 24);

    frame->ms_gps = payload[104] | (payload[105] << 8) |
        (payload[106] << 16) | (payload[107] << 24);
    frame->GNSS_info1 = payload[108];
    frame->GNSS_info2 = payload[109];
    frame->solnSVs = payload[110];
    frame->v_latency = payload[111] | (payload[112] << 8);
    frame->angle_pos_type = payload[113];
    frame->hdg_GNSS = payload[114] | (payload[115] << 8);

    frame->latency_ms_hdg = payload[116] | (payload[117] << 8);
    frame->latency_ms_pos = payload[118] | (payload[119] << 8);
    frame->latency_ms_vel = payload[120] | (payload[121] << 8);

    frame->p_bar = payload[122] | (payload[123] << 8);
    frame->h_bar = payload[124] | (payload[125] << 8) |
        (payload[126] << 16) | (payload[127] << 24);
    frame->new_gps = payload[129];
    
}

const char* argument_error =
    "%s: invalid option -- '%s'\n"
    "type '%s --usage' for more info\n";

const char* usage_help =
    "usage: %s infile [-o outfile]\n"
    "  infile: file to be converted to text\n"
    "  outfile: output filename\n";

int main(int argc, char** argv)
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
        fprintf(stderr, "%s: failed to open '%s'\n", argv[0], argv[1]);
        return 1;
    }

    unsigned long long filelen;
    fseek(infile, 0, SEEK_END);
    filelen = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    if (filelen == 0)
    {
        fprintf(stderr, "%s: %s is an empty file\n", argv[0], argv[1]);
        return 1;
    }

    unsigned char *file_buffer = (unsigned char*) malloc(filelen);
    if (!file_buffer)
    {
        fprintf(stderr, "%s: memory allocation error\n", argv[0]);
        return 1;
    }

    fread(file_buffer, 1, filelen, infile);
    fclose(infile);

    const unsigned long framelen = 129;
    signed long long rptr = filelen;
    while (rptr > 0) rptr -= framelen;
    rptr += framelen;
    println_opvt2ahr(0);
    while (rptr < filelen - framelen)
    {
        unsigned char payload[framelen];
        memcpy(payload, file_buffer + rptr, sizeof(payload));
        struct opvt2ahr frame;
        payload2opvt2ahr(&frame, payload);
        println_opvt2ahr(&frame);
        rptr += framelen;
        fprintf(stderr, "\r%02.1f", (100.0*rptr)/filelen);
    }
    fprintf(stderr, "       \rDone.\n");
    return 0;
}
