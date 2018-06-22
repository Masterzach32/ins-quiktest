#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>

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

struct opvt
{
    unsigned short heading;
    signed short pitch, roll;
    signed short gyro_x, gyro_y, gyro_z;
    signed short acc_x, acc_y, acc_z;
    signed short mag_x, mag_y, mag_z;
    unsigned short USW;
    unsigned short vinp;
    signed short temp;
    signed long latitude, longitude, altitude;
    signed long v_east, v_north, v_up;
    signed long lat_GNSS, lon_GNSS, alt_GNSS, vh_GNSS;
    signed short track_grnd;
    signed long vup_GNSS;
    unsigned long ms_gps;
    unsigned char GNSS_info1, GNSS_info2, solnSVs;
    unsigned short v_latency;
    signed char latency_ms_pos, latency_ms_vel;
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

int payload2opvt(struct opvt *frame, unsigned char *payload)
{
    if (!frame || !payload) return 1;

    if ((payload[0] != 0xAA) || (payload[1] != 0x55) ||
        (payload[2] != 0x01) || (payload[3] != 0x52))
    {
        return 1;
    }

    frame->heading = payload[6+0] | (payload[6+1] << 8);
    frame->pitch = payload[6+2] | (payload[6+3] << 8);
    frame->roll = payload[6+4] | (payload[6+5] << 8);

    frame->gyro_x = payload[6+6] | (payload[6+7] << 8);
    frame->gyro_y = payload[6+8] | (payload[6+9] << 8);
    frame->gyro_z = payload[6+10] | (payload[6+11] << 8);

    frame->acc_x = payload[6+12] | (payload[6+13] << 8);
    frame->acc_y = payload[6+14] | (payload[6+15] << 8);
    frame->acc_z = payload[6+16] | (payload[6+17] << 8);

    frame->mag_x = payload[6+18] | (payload[6+19] << 8);
    frame->mag_y = payload[6+20] | (payload[6+21] << 8);
    frame->mag_z = payload[6+22] | (payload[6+23] << 8);

    frame->USW = payload[6+24] | (payload[6+25] << 8);
    frame->vinp = payload[6+26] | (payload[6+27] << 8);
    frame->temp = payload[6+28] | (payload[6+29] << 8);

    frame->latitude = payload[6+30] | (payload[6+31] << 8) |
        (payload[6+32] << 16) | (payload[6+33] << 24);
    frame->longitude = payload[6+34] | (payload[6+35] << 8) |
        (payload[6+36] << 16) | (payload[6+37] << 24);
    frame->altitude = payload[6+38] | (payload[6+39] << 8) |
        (payload[6+40] << 16) | (payload[6+41]) << 24;

    frame->v_east = payload[6+42] | (payload[6+43] << 8) |
        (payload[6+44] << 16) | (payload[6+45] << 24);
    frame->v_north = payload[6+46] | (payload[6+47] << 8) |
        (payload[6+48] << 16) | (payload[6+49] << 24);
    frame->v_up = payload[6+50] | (payload[6+51] << 8) |
        (payload[6+52] << 16) | (payload[6+53] << 24);

    frame->lat_GNSS = payload[6+54] | (payload[6+55] << 8) |
        (payload[6+56] << 16) | (payload[6+57] << 24);
    frame->lon_GNSS = payload[6+58] | (payload[6+59] << 8) |
        (payload[6+60] << 16) | (payload[6+61] << 24);
    frame->alt_GNSS = payload[6+62] | (payload[6+63] << 8) |
        (payload[6+64] << 16) | (payload[6+65] << 24);

    frame->vh_GNSS = payload[6+66] | (payload[6+67] << 8) |
        (payload[6+68] << 16) | (payload[6+69] << 24);
    frame->track_grnd = payload[6+70] | (payload[6+71] << 8);
    frame->vup_GNSS = payload[6+72] | (payload[6+73] << 8) |
        (payload[6+74] << 16) | (payload[6+75] << 24);

    frame->ms_gps = payload[6+76] | (payload[6+77] << 8) |
        (payload[6+78] << 16) | (payload[6+79] << 24);
    frame->GNSS_info1 = payload[6+80];
    frame->GNSS_info2 = payload[6+81];
    frame->solnSVs = payload[6+82];

    frame->latency_ms_pos = payload[6+83];
    frame->latency_ms_vel = payload[6+84];

    frame->p_bar = payload[6+85] | (payload[6+86] << 8);
    frame->h_bar = payload[6+87] | (payload[6+88] << 8) |
        (payload[6+89] << 16) | (payload[6+90] << 24);
    frame->new_gps = payload[6+91];

    unsigned short checksum = 0;
    for (unsigned long i = 2; i < 6+98; ++i)
    {
        checksum += payload[i];
    }
    if (checksum != (payload[6+99] | (payload[6+100] << 8)))
    {
    //    return 1;
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

void println_opvt(FILE *out, struct opvt *frame)
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
    fprintf(out, "%15hhu",
        frame->solnSVs);
    fprintf(out, "%19hhd%19hhd",
        frame->latency_ms_pos, frame->latency_ms_vel);
    fprintf(out, "%15lu%15.2f%15hhu\n",
        ((unsigned long) frame->p_bar)*2, frame->h_bar/100.0, frame->new_gps);
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

    // open the OPVT file; if can't open, return error
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

    unsigned long long rptr = 0;

    // verify ACK
    if ((file_buffer[0] != 0xAA) | (file_buffer[1] != 0x55) |
        (file_buffer[2] != 0x01) | (file_buffer[4] != 0x08))
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
    println_opvt(outfile, 0);
    while (rptr < filelen)
    {
        // at the beginning of every iteration,
        // rptr will point at the AA in the beginning
        // of every packet
        struct opvt frame;
        int error = payload2opvt(&frame, file_buffer + rptr);
        if (error) ++rptr;
        else
        {
            println_opvt(outfile, &frame);
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
    return 0;
}
