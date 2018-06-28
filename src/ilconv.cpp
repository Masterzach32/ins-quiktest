#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>

#include <Eigen/Geometry>

struct short_align_block
{
    // only kept for backwards compatibility

    float gyro_bias[3], avg_accel[3], avg_mag[3],
          init_hdg, init_roll, init_pitch;
    unsigned short USW;
};

struct ext_align_block
{
    float gyro_bias[3], avg_accel[3], avg_mag[3],
          init_hdg, init_roll, init_pitch;
    unsigned short USW;
    signed long UT_sr, UP_sr;
    signed short t_gyro[3], t_acc[3], t_mag[3];
    double latitude, longitude, altitude;
    float v_east, v_north, v_up;
    double g_true;
    float reserved1, reserved2;
};

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

int payload2header(struct short_align_block *frame, unsigned char *payload)
{
    // returns 0 if everything goes ok

    if (!frame || !payload) return 1;

    if ((payload[0] != 0xAA) || (payload[1] != 0x55) || (payload[2] != 0x01))
    {
        return 1;
    }
    if ((payload[4] | (payload[5] << 8)) != 0x38)
    {
        return 1;
    }

    memcpy(&frame->gyro_bias, payload+6, 12);
    memcpy(&frame->avg_accel, payload + 12+6, 12);
    memcpy(&frame->avg_mag, payload + 24+6, 12);
    memcpy(&frame->init_hdg, payload + 36+6, 4);
    memcpy(&frame->init_roll, payload + 40+6, 4);
    memcpy(&frame->init_pitch, payload + 44+6, 4);
    frame->USW = payload[48+6] | (payload[49+6] << 8);

    unsigned short checksum = 0;
    for (unsigned long i = 2; i < 56; ++i)
    {
        checksum += payload[i];
    }
    if (checksum != (payload[56] | (payload[57] << 8)))
    {
        return 1;
    }

    return 0;
}

int payload2extheader(struct ext_align_block *frame,
                       unsigned char *payload)
{
    // returns 0 if everything goes ok

    if (!frame || !payload) return 1;

    if ((payload[0] != 0xAA) || (payload[1] != 0x55) || (payload[2] != 0x01))
    {
        return 1;
    }
    if ((payload[4] | (payload[5] << 8)) != 0x86)
    {
        return 1;
    }

    memcpy(&frame->gyro_bias, payload + 6, 12);
    memcpy(&frame->avg_accel, payload + 12+6, 12);
    memcpy(&frame->avg_mag, payload + 24+6, 12);
    memcpy(&frame->init_hdg, payload + 36+6, 4);
    memcpy(&frame->init_roll, payload + 40+6, 4);
    memcpy(&frame->init_pitch, payload + 44+6, 4);
    frame->USW = payload[48+6] | (payload[49+6] << 8);

    frame->UT_sr = payload[50+6] | (payload[51+6] << 8) |
                   (payload[52+6] << 16) | (payload[53+6] << 24);
    frame->UP_sr = payload[54+6] | (payload[55+6] << 8) |
                   (payload[56+6] << 16) | (payload[57+6] << 24);

    frame->t_gyro[0] = payload[58+6] | (payload[59+6] << 8);
    frame->t_gyro[1] = payload[60+6] | (payload[61+6] << 8);
    frame->t_gyro[2] = payload[62+6] | (payload[63+6] << 8);

    frame->t_acc[0] = payload[64+6] | (payload[65+6] << 8);
    frame->t_acc[1] = payload[66+6] | (payload[67+6] << 8);
    frame->t_acc[2] = payload[68+6] | (payload[69+6] << 8);

    frame->t_mag[0] = payload[70+6] | (payload[71+6] << 8);
    frame->t_mag[1] = payload[72+6] | (payload[73+6] << 8);
    frame->t_mag[2] = payload[74+6] | (payload[75+6] << 8);

    memcpy(&frame->latitude, payload + 76+6, 8);
    memcpy(&frame->longitude, payload + 84+6, 8);
    memcpy(&frame->altitude, payload + 92+6, 8);
    memcpy(&frame->v_east, payload + 100+6, 4);
    memcpy(&frame->v_north, payload + 104+6, 4);
    memcpy(&frame->v_up, payload + 108+6, 4);
    memcpy(&frame->g_true, payload + 112+6, 8);
    memcpy(&frame->reserved1, payload + 120+6, 4);
    memcpy(&frame->reserved2, payload + 124+6, 4);

    unsigned short checksum = 0;
    for (unsigned long i = 2; i < 134; ++i)
    {
        checksum += payload[i];
    }
    if (checksum != (payload[134] | (payload[135] << 8)))
    {
        return 1;
    }

    return 0;
}

int payload2opvt2ahr(struct opvt2ahr *frame, unsigned char *payload)
{
    if (!frame || !payload) return 1;

    if ((payload[0] != 0xAA) || (payload[1] != 0x55) ||
        (payload[2] != 0x01) || (payload[3] != 0x58))
    {
        return 1;
    }

    frame->heading = payload[6+0] | (payload[6+1] << 8);
    frame->pitch = payload[6+2] | (payload[6+3] << 8);
    frame->roll = payload[6+4] | (payload[6+5] << 8);

    frame->gyro_x = payload[6+6] | (payload[6+7] << 8) |
        (payload[6+8] << 16) | (payload[6+9] << 24);
    frame->gyro_y = payload[6+10] | (payload[6+11] << 8) |
        (payload[6+12] << 16) | (payload[6+13] << 24);
    frame->gyro_z = payload[6+14] | (payload[6+15] << 8) |
        (payload[6+16] << 16) | (payload[6+17] << 24);

    frame->acc_x = payload[6+18] | (payload[6+19] << 8) |
        (payload[6+20] << 16) | (payload[6+21] << 24);
    frame->acc_y = payload[6+22] | payload[6+23] << 8 |
        (payload[6+24] << 16) | (payload[6+25] << 24);
    frame->acc_z = payload[6+26] | (payload[6+27] << 8) |
        (payload[6+28] << 16) | (payload[6+29] << 24);

    frame->mag_x = payload[6+30] | (payload[6+31] << 8);
    frame->mag_y = payload[6+32] | (payload[6+33] << 8);
    frame->mag_z = payload[6+34] | (payload[6+35] << 8);

    frame->USW = payload[6+36] | (payload[6+37] << 8);
    frame->vinp = payload[6+38] | (payload[6+39] << 8);
    frame->temp = payload[6+40] | (payload[6+41] << 8);

    frame->latitude = (long long) payload[6+42] |
        ((long long) payload[6+43] << 8) |
        ((long long) payload[6+44] << 16) |
        ((long long) payload[6+45] << 24) |
        ((long long) payload[6+46] << 32) |
        ((long long) payload[6+47] << 40) |
        ((long long) payload[6+48] << 48) |
        ((long long) payload[6+49] << 56);
    frame->longitude = (long long) payload[6+50] |
        ((long long) payload[6+51] << 8) |
        ((long long) payload[6+52] << 16) |
        ((long long) payload[6+53] << 24) |
        ((long long) payload[6+54] << 32) |
        ((long long) payload[6+55] << 40) |
        ((long long) payload[6+56] << 48) |
        ((long long) payload[6+57] << 56);
    frame->altitude = payload[6+58] | (payload[6+59] << 8) |
        (payload[6+60] << 16) | (payload[6+61]) << 24;

    frame->v_east = payload[6+62] | (payload[6+63] << 8) |
        (payload[6+64] << 16) | (payload[6+65] << 24);
    frame->v_north = payload[6+66] | (payload[6+67] << 8) |
        (payload[6+68] << 16) | (payload[6+69] << 24);
    frame->v_up = payload[6+70] | (payload[6+71] << 8) |
        (payload[6+72] << 16) | (payload[6+73] << 24);

    frame->lat_GNSS = (long long) payload[6+74] |
        ((long long) payload[6+75] << 8) |
        ((long long) payload[6+76] << 16) |
        ((long long) payload[6+77] << 24) |
        ((long long) payload[6+78] << 32) |
        ((long long) payload[6+79] << 40) |
        ((long long) payload[6+80] << 48) |
        ((long long) payload[6+81] << 56);
    frame->lon_GNSS = (long long) payload[82] |
        ((long long) payload[6+83] << 8) |
        ((long long) payload[6+84] << 16) |
        ((long long) payload[6+85] << 24) |
        ((long long) payload[6+86] << 32) |
        ((long long) payload[6+87] << 40) |
        ((long long) payload[6+88] << 48) |
        ((long long) payload[6+89] << 56);
    frame->alt_GNSS = payload[6+90] | (payload[6+91] << 8) |
        (payload[6+92] << 16) | (payload[6+93] << 24);

    frame->vh_GNSS = payload[6+94] | (payload[6+95] << 8) |
        (payload[6+96] << 16) | (payload[6+97] << 24);
    frame->track_grnd = payload[6+98] | (payload[6+99] << 8);
    frame->vup_GNSS = payload[6+100] | (payload[6+101] << 8) |
        (payload[6+102] << 16) | (payload[6+103] << 24);

    frame->ms_gps = payload[6+104] | (payload[6+105] << 8) |
        (payload[6+106] << 16) | (payload[6+107] << 24);
    frame->GNSS_info1 = payload[6+108];
    frame->GNSS_info2 = payload[6+109];
    frame->solnSVs = payload[6+110];
    frame->v_latency = payload[6+111] | (payload[6+112] << 8);
    frame->angle_pos_type = payload[6+113];
    frame->hdg_GNSS = payload[6+114] | (payload[6+115] << 8);

    frame->latency_ms_hdg = payload[6+116] | (payload[6+117] << 8);
    frame->latency_ms_pos = payload[6+118] | (payload[6+119] << 8);
    frame->latency_ms_vel = payload[6+120] | (payload[6+121] << 8);

    frame->p_bar = payload[6+122] | (payload[6+123] << 8);
    frame->h_bar = payload[6+124] | (payload[6+125] << 8) |
        (payload[6+126] << 16) | (payload[6+127] << 24);
    frame->new_gps = payload[6+128];

    unsigned short checksum = 0;
    for (unsigned long i = 2; i < 135; ++i)
    {
        checksum += payload[i];
    }
    if (checksum != (payload[135] | (payload[136] << 8)))
    {
        return 1;
    }

    return 0;
}

void print_header(FILE* out, struct short_align_block *frame)
{
    if (!out || !frame) return;

    fprintf(out, "gyroscope bias: %.5f %.5f %.5f\n",
        frame->gyro_bias[0], frame->gyro_bias[1], frame->gyro_bias[2]);
    fprintf(out, "mean acceleration: %.5f %.5f %.5f\n",
        frame->avg_accel[0], frame->avg_accel[1], frame->avg_accel[2]);
    fprintf(out, "mean magnetic field: %.5f %.5f %.5f\n",
        frame->avg_mag[0], frame->avg_mag[1], frame->avg_mag[2]);
    fprintf(out, "initial orientation: %.3f %.3f %.3f\n",
        frame->init_hdg, frame->init_pitch, frame->init_roll);
    fprintf(out, "unit status word: 0x%04x\n", frame->USW);
}

/*
struct ext_align_block
{
    float gyro_bias[3], avg_accel[3], avg_mag[3],
          init_hdg, init_roll, init_pitch;
    unsigned short USW;
    signed long UT_sr, UP_sr;
    signed short t_gyro[3], t_acc[3], t_mag[3];
    double latitude, longitude, altitude;
    float v_east, v_north, v_up;
    double g_true;
    float reserved1, reserved2;
};
*/

void print_extheader(FILE *out, struct ext_align_block *frame)
{
    if (!out || !frame) return;

    fprintf(out, "gyroscope bias: %.5f %.5f %.5f\n",
        frame->gyro_bias[0], frame->gyro_bias[1], frame->gyro_bias[2]);
    fprintf(out, "mean acceleration: %.5f %.5f %.5f\n",
        frame->avg_accel[0], frame->avg_accel[1], frame->avg_accel[2]);
    fprintf(out, "mean magnetic field: %.5f %.5f %.5f\n",
        frame->avg_mag[0], frame->avg_mag[1], frame->avg_mag[2]);
    fprintf(out, "initial orientation: %.3f %.3f %.3f\n",
        frame->init_hdg, frame->init_pitch, frame->init_roll);
    fprintf(out, "unit status word: 0x%04x\n", frame->USW);
    fprintf(out, "UT_sr: %ld; UP_sr: %ld\n",
        frame->UT_sr, frame->UP_sr);
    fprintf(out, "temp in gyro: %hd %hd %hd; acc: %hd %hd %hd; "
                 "mag: %hd %hd %hd\n",
        frame->t_gyro[0], frame->t_gyro[1], frame->t_gyro[2],
        frame->t_acc[0], frame->t_acc[1], frame->t_acc[2],
        frame->t_mag[0], frame->t_mag[1], frame->t_mag[2]);
    fprintf(out, "coordinates: %.9f %.9f %.3f\n",
        frame->latitude, frame->longitude, frame->altitude);
    fprintf(out, "velocity: %.3f %.3f %.3f\n",
        frame->v_east, frame->v_north, frame->v_up);
    fprintf(out, "gravity: %.5f\n", frame->g_true);
    fprintf(out, "reserved: %.6f %.6f\n",
        frame->reserved1, frame->reserved2);
}

void println_opvt2ahr(FILE *out, struct opvt2ahr *frame)
{
    if (!out) return;
    if (!frame)
    {
        fprintf(out,
               "        Heading"
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
               "            USW"
               "       Latitude"
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

    fprintf(out, "%15.2f%15.2f%15.2f",
        frame->heading/100.0, frame->pitch/100.0, frame->roll/100.0);
    fprintf(out, "%15.5f%15.5f%15.5f",
        frame->gyro_x/1.0E5, frame->gyro_y/1.0E5, frame->gyro_z/1.0E5);
    fprintf(out, "%15.6f%15.6f%15.6f",
        frame->acc_x/1.0E6, frame->acc_y/1.0E6, frame->acc_z/1.0E6);
    fprintf(out, "%15.1f%15.1f%15.1f",
        frame->mag_x*10.0, frame->mag_y*10.0, frame->mag_z*10.0);
    fprintf(out, "%15.1f%15.2f%15hu",
        frame->temp/10.0, frame->vinp/100.0, frame->USW);
    fprintf(out, "%15.9f%15.9f%15.7f",
        frame->latitude/1.0E9, frame->longitude/1.0E9, frame->altitude/1.0E3);
    fprintf(out, "%15.2f%15.2f%15.2f",
        frame->v_east/100.0, frame->v_north/100.0, frame->v_up/100.0);
    fprintf(out, "%15.9f%15.9f%15.7f",
        frame->lat_GNSS/1.0E9, frame->lon_GNSS/1.0E9, frame->alt_GNSS/1.0E3);
    fprintf(out, "%15.2f%15.2f%15.2f",
        frame->vh_GNSS/100.0, frame->track_grnd/100.0, frame->vup_GNSS/100.0);
    fprintf(out, "%15lu", frame->ms_gps);
    fprintf(out, "%15hhu%15hhu",
        frame->GNSS_info1, frame->GNSS_info2);
    fprintf(out, "%15hhu%15hu%15hhu",
        frame->solnSVs, frame->v_latency, frame->angle_pos_type);
    fprintf(out, "%15.2f%19hhd%19hhd%19hhd",
        frame->hdg_GNSS/100.0, frame->latency_ms_hdg,
        frame->latency_ms_pos, frame->latency_ms_vel);
    fprintf(out, "%15lu%15.2f%15hhu\n",
        ((unsigned long) frame->p_bar)*2, frame->h_bar/100.0, frame->new_gps);
}

const char* argument_error =
    "%s: invalid option -- '%s'\n"
    "type '%s --usage' for more info\n";

const char* usage_help =
    "usage: %s infile [-o outfile] [-pv x y z]\n"
    "  infile: file to be converted to text\n"
    "  outfile: output filename\n"
    "  x y z: position-velocity offset\n";

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
    const unsigned long framelen = 137;
    if (filelen < framelen)
    {
        fprintf(stderr, "%s: %s is not long enough\n", argv[0], argv[1]);
        return 1;
    }

    unsigned char out_index = 0;
    unsigned char pvoff_flag = 0;
    double pvoff_input[3] = {0};

    for (int i = 2; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-o") | !strcmp(argv[i], "--out"))
        {
            if (argc < i + 2)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 1;
            }
            out_index = ++i;
        }
        else if (!strcmp(argv[i], "-pv") | !strcmp(argv[i], "--pvoff"))
        {
            if (argc < i + 4)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 1;
            }
            pvoff_flag = 1;
            pvoff_input[0] = atof(argv[++i]);
            pvoff_input[1] = atof(argv[++i]);
            pvoff_input[2] = atof(argv[++i]);
        }
        else // if any argument is unexpected, throw argument error
        {
            fprintf(stderr, argument_error, argv[0], argv[i], argv[0]);
            return 1;
        }
    }

    unsigned char *file_buffer = (unsigned char*) malloc(filelen);
    if (!file_buffer)
    {
        fprintf(stderr, "%s: memory allocation error\n", argv[0]);
        return 1;
    }

    fread(file_buffer, 1, filelen, infile);
    fclose(infile);

    char *outfn = (char*) malloc(strlen(argv[1]) + 1);
    if (!outfn)
    {
        fprintf(stderr, "%s: memory allocation error\n", argv[0]);
        return 1;
    }
    strcpy(outfn, argv[1]);
    char *ext_ptr = strstr(outfn, ".bin");
    if (!ext_ptr) // file does not contain ".bin", so tack ".txt" on the end
    {
        free(outfn);
        outfn = (char*) malloc(strlen(argv[1]) + 5);
        if (!outfn)
        {
            fprintf(stderr, "%s: memory allocation error\n", argv[0]);
            return 1;
        }
        strcpy(outfn, argv[1]);
        strcpy(outfn + strlen(outfn), ".txt");
    }
    else // replace ".bin" with ".txt"
    {
        strcpy(ext_ptr, ".txt");
    }

    FILE *outfile;
    if (out_index)
    {
        outfn = argv[out_index];
    }
    outfile = fopen(outfn, "wb");
    if (!outfile)
    {
        fprintf(stderr, "%s: failed to open '%s'\n", argv[0], outfn);
        return 1;
    }

    FILE *debug;
    if (pvoff_flag)
    {
        debug = fopen("calculations.txt", "wb");
        fprintf(debug, "pv offset: %.2f %.2f %.2f\n\n",
            pvoff_input[0], pvoff_input[1], pvoff_input[2]);
        fprintf(debug, "%15s%15s%15s%15s%15s"
            "%15s%15s%15s%15s%15s%15s%15s"
            "%15s%15s%15s%15s%15s%15s\n",
            "ms_gps","compass","heading","pitch","roll",
            "qw","qx","qy","qz",
            "gyro_x","gyro_y","gyro_z",
            "poff_east","poff_north","poff_up",
            "voff_east","voff_north","voff_up");
    }

    unsigned long long rptr = 0;

    fprintf(outfile, "post-test applied PV offset: %.2f %.2f %.2f\n",
        pvoff_input[0], pvoff_input[1], pvoff_input[2]);

    // TODO: include checksum verification

    // verify ACK
    if ((file_buffer[0] != 0xAA) | (file_buffer[1] != 0x55) |
        (file_buffer[2] != 0x01) | (file_buffer[3] != 0x58) |
        (file_buffer[4] != 0x08))
    {
            fprintf(stderr, "%s: file ACK parse error at 0x%02llx\n",
                argv[0], rptr);
            return 1;
    }

    rptr += 10;
    // verify initial alignment block
    if ((file_buffer[rptr] != 0xAA) | (file_buffer[rptr + 1] != 0x55) |
        (file_buffer[rptr + 2] != 0x01))
    {
            fprintf(stderr, "%s: file align block parse error 0x%02llx\n",
                argv[0], rptr);
            return 1;
    }

    unsigned short msg_len = file_buffer[rptr+4] | (file_buffer[rptr+5] << 8);
    if (msg_len == 0x38) // short alignment block
    {
        struct short_align_block header;
        payload2header(&header, file_buffer + rptr);
        print_header(outfile, &header);
        rptr += 58;
    }
    else if (msg_len == 0x86) // extended block
    {
        struct ext_align_block header;
        payload2extheader(&header, file_buffer + rptr);
        print_extheader(outfile, &header);
        rptr += 136;
    }

    unsigned char progress, old_progress = 255;

    fprintf(outfile, "\n");
    println_opvt2ahr(outfile, 0);
    while (rptr < filelen)
    {
        // at the beginning of every iteration,
        // rptr will point at the AA in the beginning
        // of every packet
        struct opvt2ahr frame;
        int error = payload2opvt2ahr(&frame, file_buffer + rptr);
        if (error) ++rptr;
        else if (pvoff_flag)
        {
            // rotations to radians; heading sign convention
            // is inverted to follow the right-hand rule
            double heading = (M_PI/180)*(360 - frame.heading/100.0),
                   pitch = (M_PI/180)*(frame.pitch/100.0),
                   roll = (M_PI/180)*(frame.roll/100.0);

            // calculate rotation quaternion
            // rotation convention is Z-X'-Y''
            const Eigen::Vector3d const_offset =
                {pvoff_input[0], pvoff_input[1], pvoff_input[2]};
            auto qz = Eigen::AngleAxisd(heading, Eigen::Vector3d::UnitZ());
            auto Xp = qz * Eigen::Vector3d::UnitX(),
                 Yp = qz * Eigen::Vector3d::UnitY();
            auto qx = Eigen::AngleAxisd(pitch, Xp);
            auto Ypp = qx * Yp;
            auto qy = Eigen::AngleAxisd(roll, Ypp);
            Eigen::Quaterniond qw = qz * qx * qy;
            auto p_offset = qw * const_offset;
            const unsigned long long R_EARTH = 6371000;

            // add offset to opvt2ahr data frame before printing
            frame.latitude += (180E9*p_offset.y())/(R_EARTH*M_PI);
            double lat_rad = (M_PI/180)*(frame.latitude/1.0E9);
            frame.longitude += (180E9*p_offset.x())/(R_EARTH*M_PI*cos(lat_rad));
            frame.altitude += 1E3*p_offset.z();

            // turn rate in radians per second
            auto turn_rate = Eigen::Vector3d(
                frame.gyro_x/1.0E5, frame.gyro_y/1.0E5, frame.gyro_z/1.0E5);
            turn_rate = M_PI/180.0 * turn_rate;
            auto v_offset = (qw * turn_rate).cross(qw * const_offset);
            frame.v_east += v_offset.x();
            frame.v_north += v_offset.y();
            frame.v_up += v_offset.z();

            /*
            // print useful info to debug file
            fprintf(debug, "%15lu%15.2f"
                "%15.2f%15.2f%15.2f"
                "%15.2f%15.2f%15.2f%15.2f"
                "%15.5f%15.5f%15.5f"
                "%15.3f%15.3f%15.3f"
                "%15.3f%15.3f%15.3f\n",
                frame.ms_gps, frame.heading/100.0,
                heading*180/M_PI, pitch*180/M_PI, roll*180/M_PI,
                qw.w(), qw.x(), qw.y(), qw.z(),
                turn_rate.x(), turn_rate.y(), turn_rate.z(),
                p_offset.x(), p_offset.y(), p_offset.z(),
                v_offset.x(), v_offset.y(), v_offset.z());
            */
        }

        if (!error)
        {
            println_opvt2ahr(outfile, &frame);
            rptr += framelen;
        }
        fflush(outfile);

        progress = 100*rptr/filelen;
        if (progress != old_progress)
            fprintf(stderr, "\r%s: Writing to %s: %2hhu%%",
                argv[0], outfn, progress);
    }
    fprintf(stderr, "\r%s: Writing to %s: Done.\n", argv[0], outfn);
    fclose(outfile);
    if (pvoff_flag) fclose(debug);
    return 0;
}
