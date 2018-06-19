#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>

struct oem7_header
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

struct inspva
{
    struct oem7_header header;
    unsigned long week;
    double seconds;
    double latitude, longitude, altitude,
           v_north, v_east, v_up,
           roll, pitch, azimuth;

    // actually an enum
    unsigned long status, checksum;
};

int payload2inspva(struct inspva *frame, unsigned char *payload)
{
    if (!frame || !payload) return 1;
    
    if ((payload[0] != 0xAA) || (payload[1] != 0x44) || (payload[2] != 0x12) ||
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
    return "(parse error!)";
}

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
    return "(parse error!)";
}

void println_inspva(FILE *out, struct inspva *frame)
{
    fprintf(out, "#INSPVAX,"
                 "%hhu," // port_addr - uchar
                 // "%hhu," // msg_type - uchar
                 // "%hu," // msg_len - ushort
                 "%hu," // sequence - ushort
                 "%.1f," // idle_time - float
                 "%s," // time_status - string
                 "%hu," // week - ushort
                 "%.3f,%lu,%hx,%hu",
        frame->header.port_addr, 
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

    // open the SPAN bin file; if can't open, return error
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
    
    // unsigned char progress = 0, old_progress = 255;
    unsigned long long rptr = 0;

    // find first aa 44 12 sequence
    for (int i = 0; rptr == 0; ++i)
    {
        if ((file_buffer[i]) == 0xAA && (file_buffer[i+1]) == 0x44 &&
            (file_buffer[i+2] == 0x12) && (file_buffer[i+4] == 0xFB))
        {
            rptr = i;
        }
    }
    
    printf("%llx\n", rptr);
    
    struct inspva frame;
    while (rptr < filelen)
    {
        if (payload2inspva(&frame, file_buffer + rptr) == 0)
        {
            println_inspva(outfile, &frame);
            rptr += 120;
        }
        else ++rptr;
    }
    fclose(outfile);
    return 0;
}
    