#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>

char numstr[11] = {0};

char* num2str(unsigned long num)
{
    sprintf(numstr, "%lu", num);
    return numstr;
}

// encoding for the NovAtel OEM7 COM port ID table;
// does not contain all COM port mappings
// https://docs.novatel.com/OEM7/Content/Messages/Binary.htm
char* port_str(unsigned long port_id)
{
    // there are literally 11,456 cases
    // for this enumeration, so I'm not going
    // to try to include all of them; just
    // the most common ones

    switch (port_id)
    {
        case 0: return "NO_PORTS";
        case 1: return "COM1_ALL";
        case 2: return "COM2_ALL";
        case 3: return "COM3_ALL";
        case 6: return "THISPORT_ALL";
        case 7: return "FILE_ALL";
        case 8: return "ALL_PORTS";
        case 9: return "XCOM1_ALL";
        case 10: return "XCOM2_ALL";
        case 13: return "USB1_ALL";
        case 14: return "USB2_ALL";
        case 15: return "USB3_ALL";
        case 16: return "AUX_ALL";
        case 17: return "XCOM3_ALL";
        case 19: return "COM4_ALL";
        case 20: return "ETH1_ALL";
        case 21: return "IMU_ALL";
        case 23: return "ICOM1_ALL";
        case 24: return "ICOM2_ALL";
        case 25: return "ICOM3_ALL";
        case 26: return "NCOM1_ALL";
        case 27: return "NCOM2_ALL";
        case 28: return "NCOM3_ALL";
        case 29: return "ICOM4_ALL";
        case 30: return "WCOM4_ALL";
        case 32: return "COM1";
        case 33: return "COM1_1";
        // ...
        case 63: return "COM1_31";
        case 64: return "COM2";
        case 65: return "COM2_1";
        // ...
        case 95: return "COM2_31";
        case 96: return "COM3";
        case 97: return "COM3_1";
        // ...
        case 127: return "COM3_31";
        case 160: return "SPECIAL";
        case 161: return "SPECIAL_1";
        // ...
        case 191: return "SPECIAL_31";
        case 192: return "THISPORT";
        case 193: return "THISPORT_1";
        // ...
        case 223: return "THISPORT_31";
        case 225: return "FILE";
        case 226: return "FILE_1";
        case 255: return "FILE_31";
    }
    return num2str(port_id);
}

// encoding for NovAtel OEM7 INS solution status
// https://docs.novatel.com/OEM7/Content/SPAN_Logs/INSATT.htm#InertialSolutionStatus
char* insstat_str(unsigned long ins_status)
{
    switch (ins_status)
    {
        case 0: return "INS_INACTIVE";
        case 1: return "INS_ALIGNING";
        case 2: return "INS_HIGH_VARIANCE";
        case 3: return "INS_SOLUTION_GOOD";
        case 6: return "INS_SOLUTION_FREE";
        case 7: return "INS_ALIGNMENT_COMPLETE";
        case 8: return "DETERMINING_ORIENTATION";
        case 9: return "WAITING_INITIALPOS";
        case 10: return "WAITING_AZIMUTH";
        case 11: return "INITIALIZING_BIASES";
        case 12: return "MOTION_DETECT";
    }
    return num2str(ins_status);
}


// encoding for NovAtel OEM7 receiver fix status
// https://docs.novatel.com/OEM7/Content/Logs/BESTPOS.htm#SolutionStatus
char* solstat_str(unsigned long sol_status)
{
    switch (sol_status)
    {
        case 0: return "SOL_COMPUTED";
        case 1: return "INSUFFICIENT_OBS";
        case 2: return "NO_CONVERGENCE";
        case 3: return "SINGULARIY";
        case 4: return "COV_TRACE";
        case 5: return "TEST_DIST";
        case 6: return "COLD_START";
        case 7: return "V_H_LIMIT";
        case 8: return "VARIANCE";
        case 9: return "RESIDUALS";
        case 13: return "INTEGRITY_WARNING";
        case 18: return "PENDING";
        case 19: return "INVALID_FIX";
        case 20: return "UNAUTHORIZED";
        case 22: return "INVALID_RATE";
    }
    return num2str(sol_status);
}

// encoding for NovAtel OEM7 receiver position type
// https://docs.novatel.com/OEM7/Content/Logs/BESTPOS.htm#Position_VelocityType
char* postype_str(unsigned long pos_type)
{
    switch (pos_type)
    {
        case 0: return "NONE";
        case 1: return "FIXEDPOS";
        case 2: return "FIXEDHEIGHT";
        case 4: return "FLOATCONV";
        case 5: return "WIDELANE";
        case 6: return "NARROWLANE";
        case 8: return "DOPPLER_VELOCITY";
        case 16: return "SINGLE";
        case 17: return "PSRDIFF";
        case 18: return "WAAS";
        case 19: return "PROPAGATED";
        case 32: return "L1_FLOAT";
        case 33: return "IONOFREE_FLOAT";
        case 34: return "NARROW_FLOAT";
        case 48: return "L1_INT";
        case 49: return "WIDE_INT";
        case 50: return "NARROW_INT";
        case 51: return "RTK_DIRECT_INS";
        case 52: return "INS_SBAS";
        case 53: return "INS_PSRSP";
        case 54: return "INS_PSRDIFF";
        case 55: return "INS_RTKFLOAT";
        case 56: return "INS_RTKFIXED";
        case 68: return "PPP_CONVERGING";
        case 69: return "PPP";
        case 70: return "OPERATIONAL";
        case 71: return "WARNING";
        case 72: return "OUT_OF_BOUNDS";
        case 73: return "INS_PPP_CONVERGING";
        case 74: return "INS_PPP";
        case 77: return "PPP_BASIC_CONVERGING";
        case 78: return "PPP_BASIC";
        case 79: return "INS_PPP_BASIC";
        case 80: return "INS_PPP_BASIC_CONVERGING";
    }
    return num2str(pos_type);
}

// encoding for NovAtel OEM7 GPS reference time status
// https://docs.novatel.com/OEM7/Content/Messages/GPS_Reference_Time_Statu.htm
char* timestat_str(unsigned char time_status)
{
    switch (time_status)
    {
        case 20: return "UNKNOWN";
        case 60: return "APPROXIMATE";
        case 80: return "COARSEADJUSTING";
        case 100: return "COARSE";
        case 120: return "COARSESTEERING";
        case 130: return "FREEWHEELING";
        case 150: return "FINEADJUSTING";
        case 160: return "FINE";
        case 170: return "FINEBACKUPSTEERING";
        case 180: return "FINESTEERING";
        case 200: return "SATTIME";
    }
    return num2str(time_status);
}

// encoding for NovAtel OEM7 datum ID
// https://docs.novatel.com/OEM7/Content/Commands/DATUM.htm
char* datum_str(unsigned char datum_ID)
{
    switch (datum_ID)
    {
        case 60: return "WGS72";
        case 61: return "WGS84";
    }
    return num2str(datum_ID);
}

// NovAtel OEM7 header structure
// https://docs.novatel.com/OEM7/Content/Messages/ASCII.htm
struct oem7_header_t
{
    unsigned char sync_bytes[3], header_len;
    unsigned short msg_ID;
    unsigned char msg_type, port_addr;
    unsigned short msg_len, sequence;
    unsigned char idle_time;

    // this is actually an enum:
    unsigned char time_status;

    unsigned short week;
    unsigned long ms, rcvr_stat;
    unsigned short reserved, version;
};

// NovAtel OEM7 INS Position, Velocity, and Attitude message
// https://docs.novatel.com/OEM7/Content/SPAN_Logs/INSPVA.htm
struct inspva_t
{
    struct oem7_header_t header;
    unsigned long week;
    double seconds;
    double latitude, longitude, altitude,
           v_north, v_east, v_up,
           roll, pitch, azimuth;

    // actually an enum
    unsigned long status, checksum;
};

// takes a pointer to a inspva_t struct, and a pointer to an unsigned
// char array. the unsigned char pointer MUST point to the first
// sync byte of any NovAtel message, i.e. 0xAA. the sync bytes defined
// by NovAtel OEM7 are 0xAA, 0x44, 0x12. if the payload pointer does
// not point at the sync bytes of an INSPVA binary message, the
// function will return an error code and the resulting inspva_t is
// invalid. upon success, the function will return 0.
int payload2inspva(struct inspva_t *frame, unsigned char *payload)
{
    if (!frame || !payload) return 1;

    if ((payload[0] != 0xAA) || (payload[1] != 0x44) ||
        (payload[2] != 0x12) ||
        (payload[4] != 0xFB) || (payload[5] != 0x01))
    {
        // this isn't the start of a SPAN INSPVA packet
        return 1;
    }

    memcpy(frame->header.sync_bytes, payload, 3);
    unsigned short N = (frame->header.header_len = payload[3]);
    frame->header.msg_ID = payload[4] | (payload[5] << 8);
    frame->header.msg_type = payload[6];
    frame->header.port_addr = payload[7];
    frame->header.msg_len = payload[8] | (payload[9] << 8);
    frame->header.sequence = payload[10] | (payload[11] << 8);
    frame->header.idle_time = payload[12];
    frame->header.time_status = payload[13]; // enum
    frame->header.week = payload[14] | (payload[15] << 8);
    frame->header.ms = payload[16] | (payload[17] << 8) |
                       (payload[18] << 16) | (payload[19] << 24);
    frame->header.rcvr_stat = payload[20] | (payload[21] << 8) |
                              (payload[22] << 16) | (payload[23] << 24);
    frame->header.reserved = payload[24] | (payload[25] << 8);
    frame->header.version = payload[26] | (payload[27] << 8);

    frame->week = payload[N] | (payload[N+1] << 8) |
                  (payload[N+2] << 16) | (payload[N+3] << 24);
    memcpy(&frame->seconds, payload+N+4, 8);

    memcpy(&frame->latitude, payload+N+12, 8);
    memcpy(&frame->longitude, payload+N+20, 8);
    memcpy(&frame->altitude, payload+N+28, 8);

    memcpy(&frame->v_north, payload+N+36, 8);
    memcpy(&frame->v_east, payload+N+44, 8);
    memcpy(&frame->v_up, payload+N+52, 8);

    memcpy(&frame->roll, payload+N+60, 8);
    memcpy(&frame->pitch, payload+N+68, 8);
    memcpy(&frame->azimuth, payload+N+76, 8);

    frame->status = payload[N+84] | (payload[N+85] << 8) |
                    (payload[N+86] << 16) | (payload[N+87] << 24);
    frame->checksum = payload[N+88] | (payload[N+89] << 8) |
                      (payload[N+90] << 16) | (payload[N+91] << 24);
    return 0;
}

// imitates (imperfectly) the ASCII output produced by NovAtel Convert;
// prints a single line of INSPVAA onto the FILE* provided.
void println_inspva(FILE *out, struct inspva_t *frame)
{
    if (!out || !frame) return;

    fprintf(out, "#INSPVAA,"
                 "%s," // port_addr - string
                 // "%hhu," // msg_type - uchar
                 // "%hu," // msg_len - ushort
                 "%hu," // sequence - ushort
                 "%.1f," // idle_time - float
                 "%s," // time_status - string
                 "%hu," // week - ushort
                 "%.3f,%lu,%hx,%hu",
        port_str(frame->header.port_addr),
        // frame->header.msg_type,
        // frame->header.msg_len,
        frame->header.sequence,
        frame->header.idle_time/2.0,
        timestat_str(frame->header.time_status),
        frame->header.week,
        frame->header.ms/1000.0,
        frame->header.rcvr_stat,
        frame->header.reserved,
        frame->header.version);
    fprintf(out, ";");
    fprintf(out, "%lu,%.9f,%.11f,%.11f,%.4f,"
                 "%.4f,%.4f,%.4f,%.9f,%.9f,%.9f,%s*%08lx\n",
        frame->week, frame->seconds,
        frame->latitude, frame->longitude, frame->altitude,
        frame->v_north, frame->v_east, frame->v_up,
        frame->roll, frame->pitch, frame->azimuth,
        insstat_str(frame->status), frame->checksum);
}

// NovAtel OEM7 RTK low latency position data message
// https://docs.novatel.com/OEM7/Content/Logs/RTKPOS.htm
struct rtkpos_t
{
    struct oem7_header_t header;
    unsigned long sol_status, pos_type;
    double latitude, longitude, altitude;
    float undulation;
    unsigned long datum_ID;
    float lat_STD, lon_STD, alt_STD;
    unsigned char station_ID[5];
    float diff_age, sol_age;
    unsigned char SVs, solnSVs, ggL1, solnMultiSVs,
        reserved, ext_sol_stat, GB_mask, GG_mask;

    unsigned long checksum;
};

// takes a rtkpos_t pointer and a pointer to the beginning of a
// RTKPOSB binary message; behavior is principally identical to
// that of the above function
// int payload2inspva(struct inspva*, unsigned char*)
int payload2rtkpos(struct rtkpos_t *frame, unsigned char *payload)
{
    if (!frame || !payload) return 1;

    if ((payload[0] != 0xAA) || (payload[1] != 0x44) ||
        (payload[2] != 0x12) ||
        (payload[4] != 0x8D) || (payload[5] != 0x00))
    {
        // this isn't the start of an RTKPOS packet
        return 1;
    }

    memcpy(frame->header.sync_bytes, payload, 3);
    unsigned short N = (frame->header.header_len = payload[3]);
    frame->header.msg_ID = payload[4] | (payload[5] << 8);
    frame->header.msg_type = payload[6];
    frame->header.port_addr = payload[7];
    frame->header.msg_len = payload[8] | (payload[9] << 8);
    frame->header.sequence = payload[10] | (payload[11] << 8);
    frame->header.idle_time = payload[12];
    frame->header.time_status = payload[13]; // enum
    frame->header.week = payload[14] | (payload[15] << 8);
    frame->header.ms = payload[16] | (payload[17] << 8) |
                       (payload[18] << 16) | (payload[19] << 24);
    frame->header.rcvr_stat = payload[20] | (payload[21] << 8) |
                              (payload[22] << 16) | (payload[23] << 24);
    frame->header.reserved = payload[24] | (payload[25] << 8);
    frame->header.version = payload[26] | (payload[27] << 8);

    frame->sol_status = payload[N+0] | (payload[N+1] << 8) |
        (payload[N+2] << 16) | (payload[N+3] << 24);
    frame->pos_type = payload[N+4] | (payload[N+5] << 8) |
        (payload[N+6] << 16) | (payload[N+7] << 24);
    memcpy(&frame->latitude, payload+N+8, 8);
    memcpy(&frame->longitude, payload+N+16, 8);
    memcpy(&frame->altitude, payload+N+24, 8);
    memcpy(&frame->undulation, payload+N+32, 4);
    frame->datum_ID = payload[N+36] | (payload[N+37] << 8) |
        (payload[N+38] << 16) | (payload[N+39] << 24);
    memcpy(&frame->lat_STD, payload+N+40, 4);
    memcpy(&frame->lon_STD, payload+N+44, 4);
    memcpy(&frame->alt_STD, payload+N+48, 4);
    memcpy(frame->station_ID, payload+N+52, 4);
    frame->station_ID[4] = 0;
    memcpy(&frame->diff_age, payload+N+56, 4);
    memcpy(&frame->sol_age, payload+N+60, 4);
    frame->SVs = payload[N+64];
    frame->solnSVs = payload[N+65];
    frame->ggL1 = payload[N+66];
    frame->solnMultiSVs = payload[N+67];
    frame->reserved = payload[N+68];
    frame->ext_sol_stat = payload[N+69];
    frame->GB_mask = payload[N+70];
    frame->GG_mask = payload[N+71];

    frame->checksum = payload[N+72] | (payload[N+73] << 8) |
        (payload[N+74] << 16) | (payload[N+75] << 24);
    return 0;
}

// imitates (imperfectly) the ASCII output produced by NovAtel Convert;
// prints a single line of RTKPOSA onto the FILE* provided
void println_rtkpos(FILE *out, struct rtkpos_t *frame)
{
    if (!out || !frame) return;

    fprintf(out, "#RTKPOSA,"
                 "%s," // port_addr - uchar
                 // "%hhu," // msg_type - uchar
                 // "%hu," // msg_len - ushort
                 "%hu," // sequence - ushort
                 "%.1f," // idle_time - float
                 "%s," // time_status - string
                 "%hu," // week - ushort
                 "%.3f,%lu,%hx,%hu",
        port_str(frame->header.port_addr),
        // frame->header.msg_type,
        // frame->header.msg_len,
        frame->header.sequence,
        frame->header.idle_time/2.0,
        timestat_str(frame->header.time_status),
        frame->header.week,
        frame->header.ms/1000.0,
        frame->header.rcvr_stat,
        frame->header.reserved,
        frame->header.version);
    fprintf(out, ";");
    fprintf(out, "%s,%s,"
        "%.11f,%.11f,%.4f,%.4f,"
        "%s,"
        "%.4f,%.4f,%.4f,"
        "\"%s\",%.3f,%.3f,"
        "%hhu,%hhu,%hhu,%hhu,"
        "%02hhx,%02hhx,%02hhx,"
        "*%08lx"
        "\n",
        solstat_str(frame->sol_status),
        postype_str(frame->pos_type),
        frame->latitude, frame->longitude,
        frame->altitude, frame->undulation,
        datum_str(frame->datum_ID),
        frame->lat_STD, frame->lon_STD, frame->alt_STD,
        frame->station_ID, frame->diff_age, frame->sol_age,
        frame->SVs, frame->solnSVs, frame->ggL1, frame->solnMultiSVs,
        frame->ext_sol_stat, frame->GB_mask, frame->GG_mask,
        frame->checksum
        );
}

const char* argument_error =
    "%s: invalid option -- '%s'\n"
    "type '%s --usage' for more info\n";

const char* usage_help =
    "usage: %s infile\n"
    "  infile: file to be converted to text\n";

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

    // open the SPAN bin file; if can't open, return error
    FILE *infile = fopen(argv[1], "rb");
    if (!infile)
    {
        fprintf(stderr, "%s: failed to open '%s'\n", argv[0], argv[1]);
        return 1;
    }

    // get length of input binary file
    unsigned long long filelen;
    fseek(infile, 0, SEEK_END);
    filelen = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    // expecting at least one INSPVA/RTKPOS log, but this is arbitrary
    if (filelen < 100)
    {
        fprintf(stderr, "%s: %s is too short\n", argv[0], argv[1]);
        return 1;
    }

    // read the binary file into unsigned char array
    unsigned char *file_buffer = (unsigned char*) malloc(filelen);
    if (!file_buffer)
    {
        fprintf(stderr, "%s: memory allocation error\n", argv[0]);
        return 1;
    }
    fread(file_buffer, 1, filelen, infile);
    fclose(infile);

    // allocate and name INSPVA text file
    char *inspva_fn = (char*) malloc(strlen(argv[1]) + 1);
    if (!inspva_fn)
    {
        fprintf(stderr, "%s: memory allocation error\n", argv[0]);
        return 1;
    }
    strcpy(inspva_fn, argv[1]);
    char *ext_ptr = strstr(inspva_fn, ".bin");
    if (!ext_ptr) // tack ".ins" on the end
    {
        free(inspva_fn);
        inspva_fn = (char*) malloc(strlen(argv[1]) + 5);
        if (!inspva_fn)
        {
            fprintf(stderr, "%s: memory allocation error\n", argv[0]);
            return 1;
        }
        strcpy(inspva_fn, argv[1]);
        strcpy(inspva_fn + strlen(inspva_fn), ".ins");
    }
    else // replace ".bin" with ".ins"
    {
        strcpy(ext_ptr, ".ins");
    }

    FILE *inspva_outfile = fopen(inspva_fn, "wb");
    if (!inspva_outfile)
    {
        fprintf(stderr, "%s: failed to open '%s'\n",
            argv[0], inspva_fn);
        return 1;
    }

    // allocate and name RTKPOS text file
    char *rtkpos_fn = (char*) malloc(strlen(argv[1]) + 1);
    if (!rtkpos_fn)
    {
        fprintf(stderr, "%s: memory allocation error\n", argv[0]);
        return 1;
    }
    strcpy(rtkpos_fn, argv[1]);
    ext_ptr = strstr(rtkpos_fn, ".bin");
    if (!ext_ptr) // tack ".rtk" on the end
    {
        free(rtkpos_fn);
        rtkpos_fn = (char*) malloc(strlen(argv[1]) + 5);
        if (!rtkpos_fn)
        {
            fprintf(stderr, "%s: memory allocation error\n", argv[0]);
            return 1;
        }
        strcpy(rtkpos_fn, argv[1]);
        strcpy(rtkpos_fn + strlen(rtkpos_fn), ".rtk");
    }
    else // replace ".bin" with ".rtk"
    {
        strcpy(ext_ptr, ".rtk");
    }

    FILE *rtkpos_outfile = fopen(rtkpos_fn, "wb");
    if (!rtkpos_outfile)
    {
        fprintf(stderr, "%s: failed to open '%s'\n",
            argv[0], rtkpos_fn);
        return 1;
    }

    // find first aa 44 12 sequence
    unsigned long long rptr = 0;
    for (int i = 0; rptr == 0 && i < filelen - 4; ++i)
    {
        if ((file_buffer[i]) == 0xAA && (file_buffer[i+1]) == 0x44 &&
            (file_buffer[i+2] == 0x12))
        {
            rptr = i;
        }
    }
    if (rptr == 0)
    {
        fprintf(stderr, "%s: '%s' does not contain any "
            "NovAtel OEM7 packets\n", argv[0], argv[1]);
        return 1;
    }

    // iterate through the file, looking for sync bytes
    while (rptr < filelen)
    {
        // taking advantage of the fact that payload2____ functions
        // return 1 when passed a pointer that does not point to
        // NovAtel sync bytes, just pass rptr to payload2____
        // and increment until it returns 0

        struct inspva_t INSPVA;
        struct rtkpos_t RTKPOS;
        if (payload2inspva(&INSPVA, file_buffer + rptr) == 0)
        {
            println_inspva(inspva_outfile, &INSPVA);
            rptr += 120; // skip length of INSPVA
        }
        else if (payload2rtkpos(&RTKPOS, file_buffer + rptr) == 0)
        {
            println_rtkpos(rtkpos_outfile, &RTKPOS);
            rptr += 104; // skip length of RTKPOS
        }
        else ++rptr; // not on a sync byte, check the next address

        static unsigned char progress, old_progress = 255;
        progress = 100*rptr/filelen;
        if (progress != old_progress)
        {
            old_progress = progress;
            fprintf(stderr, "\r%s: Writing... %2hhu%%",
                argv[0], progress);
        }
    }
    fprintf(stderr, "\r%s: Writing... Done.\n", argv[0]);
    fclose(inspva_outfile);
    fclose(rtkpos_outfile);
    return 0;
}
