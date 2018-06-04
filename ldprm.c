#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

// LoadINSPar data structure as specified in INS ICD
// revision 2.9, section 6.3.4

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

// converts LoadINSPar payload to short_prm struct; if pointer to struct
// is null, does nothing (note: uses little-endian byte ordering)

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

// does the opposite of the above function - converts short_prm struct
// to 60 byte payload

void struct2payload(const struct short_prm *prm, unsigned char payload[60])
{
    if (!prm) return;

    memset(payload, 0, 60);
    memcpy(payload + 50, prm->device_name, 8);

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

// prints payload to stdout with helpful labels

void print_payload(const unsigned char payload[60])
{
    printf("device name:");
    for (int i = 50; i < 58; ++i)
    {
        printf(" %02x", payload[i]);
    }
    printf("\ndata rate: %02x %02x\n", payload[0], payload[1]);
    printf("initial alignment time: %02x %02x\n",
            payload[2], payload[3]);
    printf("magnetic declination: %02x %02x %02x %02x\n",
            payload[4], payload[5], payload[6], payload[7]);
    printf("position: %02x %02x %02x %02x, %02x %02x "
                     "%02x %02x, %02x %02x %02x %02x\n",
            payload[8], payload[9], payload[10], payload[11],
            payload[12], payload[13], payload[14], payload[15],
            payload[16], payload[17], payload[18], payload[19]);
    printf("date: %02x/%02x/%02x\n", payload[20], payload[21], payload[22]);
    printf("alignment angles: <%02x %02x, %02x %02x, %02x %02x>\n",
        payload[23], payload[24], payload[25],
        payload[26], payload[27], payload[28]);
    printf("mounting lever: <%02x %02x, %02x %02x, %02x %02x>\n",
        payload[29], payload[30], payload[31],
        payload[32], payload[33], payload[34]);
    printf("lever arm: <%02x %02x, %02x %02x, %02x %02x>\n",
        payload[35], payload[36], payload[37],
        payload[38], payload[39], payload[40]);
    printf("altitude output: %02x\n", payload[41]);
    printf("baro enabled: %02x\n", payload[58]);
}

// prints short_prm struct to stdout
// note: values are printed in SI units, as opposed to the way they're
// stored in the data structure; for example, magnetic declination
// is expressed in degrees, rather than hundredths of degrees

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
    printf("date: %hu/%02hhu/%02hhu\n", 2000 + prm.year, prm.month, prm.day);
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


const char* argument_error =
    "%s: invalid option -- '%s'\n"
    "type '%s --usage' for more info\n";

const char* usage_help = 
    "usage: %s infile [-o outfile] [-x swapfile] "
            "[-p] [-r dr] [-i s] [-l lx ly lz] [-a h p r]\n"
    "  infile: name of file containing ReadINSPar data\n"
    "  outfile: optional name of output file to contain LoadINSPar command\n"
    "  swapfile: optional name of imitation ReadINSPar file\n"
    "  [-p]: print INS params in plaintext\n"
    "  [-h]: print INS params in hex\n"
    "  dr: data rate of INS output, in Hz; must be multiple of 200 Hz\n"
    "  s: INS initial alignment time in seconds\n"
    "  lx ly lz: offset from imu to antenna, in meters\n"
    "  h p r: angle offset from vehicle orientation, in degrees\n";

// valid data rates for INS data frame output, in Hz
const char valid_rates[] = {1, 2, 4, 5, 8, 10, 20, 25, 40, 50, 100, 200};

const char ins_com1[] =
    "/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A904D6DE-if00-port0";

int main(int argc, char** argv)
{
    if (argc < 2) // first argument must be LoadINSPar response file
    {
        fprintf(stderr, "%s: must provide filename\n", argv[0]);
        return 1;
    }

    // special case: if first argument is "--usage", print the usage
    // help string to stderr
    if (strcmp(argv[1], "--usage") == 0)
    {
        printf(usage_help, argv[0]);
        return 0;
    }

    // open the LoadINSPar file; if can't open, return error
    FILE *fileptr = fopen(argv[1], "rb");
    if (!fileptr)
    {
        fprintf(stderr, "%s: failed to open '%s'\n", argv[0], argv[1]);
        return 2;
    }

    // these flags indicate whether each flag has appeared in argv,
    // rate_flag for -r, init_flag for -i, etc. The default state is 0.
    unsigned char rate_flag = 0;
    unsigned char init_flag = 0;
    unsigned char lever_flag = 0;
    unsigned char angle_flag = 0;
    unsigned char output_flag = 0;
    unsigned char swap_flag = 0;
    unsigned char print_flag = 0;
    unsigned char hex_flag = 0;
    unsigned char serial_flag = 0;

    // if user provides arguments, they'll be stored here
    unsigned char rate_input;
    unsigned char init_input;
    double lever_input[3];
    double angle_input[3];
    char* outfn;
    char* swapfn;

    for (int i = 2; i < argc; ++i) // process every element in argv
    {
        if (strcmp(argv[i], "-o") == 0) // check for output flag
        {
            if (argc < i + 2) // verify at least one more argument exists
            {
                fprintf(stderr, usage_help, argv[0]);
                return 3;
            }
            outfn = (char*) calloc(strlen(argv[i+1]) + 1, 1);
            strcpy(outfn, argv[i+1]);
            output_flag = 1;

            // in all of these logical pathways, i is incremented so that
            // the global for loop doesn't process any argument more than
            // once; in this case, i is incremented once, because -o expects
            // only one argument
            ++i;
        }
        else if (strcmp(argv[i], "-x") == 0) // check for swapped output flag
        {
            if (argc < i + 2)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 3;
            }
            swapfn = (char*) calloc(strlen(argv[i+1]) + 1, 1);
            strcpy(swapfn, argv[i+1]);
            swap_flag = 1;
            ++i;
        }
        else if (strcmp(argv[i], "-r") == 0) // data rate flag
        {
            if (argc < i + 2)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 3;
            }
            rate_input = atoi(argv[i+1]);

            // this mess is required because only some data rates are allowed
            // by the INS; if an invalid data rate is provided by the user,
            // the program must return an error and a helpful message

            unsigned char num_of_rates = sizeof(valid_rates)/sizeof(valid_rates[0]);
            for (int i = 0; i < num_of_rates; ++i)
            {
                // check every valid rate; if the provided rate is one of the
                // valid ones, the program will use it

                if (rate_input == valid_rates[i])
                {
                    rate_flag = 1;
                }
            }
            if (!rate_flag) // if the provided rate is invalid...
            {
                fprintf(stderr, "%s: valid data rates are: ", argv[0]);

                // print every valid rate, as provided above main()
                for (int i = 0; i < num_of_rates; ++i)
                {
                    fprintf(stderr, "%hhu", valid_rates[i]);
                    if (i < num_of_rates - 1) fprintf(stderr, ", ");
                }
                fprintf(stderr, "\n");
                return 3;
}
            ++i;
        }
        else if (strcmp(argv[i], "-i") == 0) // init alignment time flag
        {
            if (argc < i + 2)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 3;
            }
            init_flag = 1;
            init_input = atoi(argv[i+1]);
            ++i;
        }
        else if (strcmp(argv[i], "-l") == 0) // lever arm flag
        {
            // verify that there are atleast 3 more arguments, because
            // -l expects lx, ly, and lz
            if (argc < i + 4)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 3;
            }

            lever_flag = 1;
            lever_input[0] = atof(argv[i+1]);
            lever_input[1] = atof(argv[i+2]);
            lever_input[2] = atof(argv[i+3]);

            // likewise, i is incremented by 3, because -l uses 3 arguments
            i+=3;
        }
        else if (strcmp(argv[i], "-a") == 0) // alignment angles flag
        {
            // -a is similar to -l in structure
            if (argc < i + 4)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 4;
            }
            angle_flag = 1;
            angle_input[0] = atof(argv[i+1]);
            angle_input[1] = atof(argv[i+2]);
            angle_input[2] = atof(argv[i+3]);
            i+=3;
        }
        else if (strcmp(argv[i], "-p") == 0) // print to stdout flag
        {
            print_flag = 1;
        }
        else if (strcmp(argv[i], "-h") == 0) // print hex to stdout flag
        {
            hex_flag = 1;
        }
        else if (strcmp(argv[i], "-s") == 0) // send command to serial flag
        {
            serial_flag = 1;
        }
        else // if any argument is unexpected, throw argument error
        {
            fprintf(stderr, argument_error, argv[0], argv[i], argv[0]);
            return 5;
        }
    }

    // at this point the command line arguments are processed and the program
    // will have exited if any were invalid.

    // this block calculates the length of the file given; the expected length
    // is 68 bytes (6 bytes of header, 60 bytes of payload, and 2 bytes of
    // checksum). if the file is not 68 bytes the program will exit.
    fseek(fileptr, 0, SEEK_END);
    if (ftell(fileptr) != 68)
    {
        fprintf(stderr, "%s: '%s' is not a ReadINSPar response\n", argv[0], argv[1]);
        return 4;
    }
    fseek(fileptr, 6, SEEK_SET); // skip the header (6 bytes)
    unsigned char payload[60];
    fread(payload, 60, 1, fileptr); // read the payload (60 bytes)
    fclose(fileptr);

    // take the payload and convert to usable primitives
    struct short_prm dat;
    payload2struct(&dat, payload);

    // if the user enabled any flags, assign the appropriate data to the struct
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
    if (hex_flag) print_payload(payload);
    struct2payload(&dat, payload); // and then convert back to a byte payload

    // two possible file output structures exist:
    //
    // for a LoadINSPar command to be sent to the INS (indicated by
    // the -o flag, and the output_flag variable), the format is 9 bytes
    // of command, and 60 bytes of header, for a total of 69 bytes.
    //
    // for a ReadINSPar response imitation (indicated by the -x flag,
    // and the swap_flag variable), the format is 6 bytes of header,
    // 60 bytes of payload, and 2 bytes of checksum.

    const unsigned char command[] = {0xAA, 0x55, 0, 0, 7, 0, 0x40, 0x47, 0};
    const unsigned char header[] = {0xAA, 0x55, 1, 0x41, 0x42, 0};

    // the checksum for an imitation ReadINSPar message is calculated
    // by summing the 3rd through 6th bytes of the header, as well as
    // all the bytes of the payload, into an unsigned short, ignoring
    // all overflow imposed by the representation.
    unsigned short sum = 0;
    for (int i = 2; i < 6; ++i)  sum += header[i];
    for (int i = 0; i < 60; ++i) sum += payload[i];
    unsigned char checksum[] = {sum & 0xFF, sum >> 8};

    if (output_flag) // write a LoadINSPar command file
    {
        FILE *outfile = fopen(outfn, "w");
        if (!outfile)
        {
            fprintf(stderr, "%s: failed to open output file '%s'\n", argv[0], outfn);
            return 5;
        }
        fprintf(outfile, "!HEX");

        for (int i = 0; i < sizeof(command); ++i)
        {
            fprintf(outfile, " %02X", command[i]);
        }
        for (int i = 0; i < sizeof(payload); ++i)
        {
            fprintf(outfile, " %02X", payload[i]);
        }
        fprintf(outfile, "\n");
        fclose(outfile);
    }
    if (swap_flag) // write an imitation ReadINSPar response
    {
        FILE *swapfile = fopen(swapfn, "wb");
        if (!swapfile)
        {
            fprintf(stderr, "%s: failed to open swap file '%s'\n", argv[0], swapfn);
            return 6;
        }
        fwrite(header, sizeof(header), 1, swapfile);
        fwrite(payload, sizeof(payload), 1, swapfile);
        fwrite(checksum, sizeof(checksum), 1, swapfile);
        fflush(swapfile);
        fclose(swapfile);
    }
    if (serial_flag)
    {
        int fd = open(ins_com1, O_RDWR | O_NOCTTY | O_NDELAY);
        if (fd == -1)
        {
            fprintf(stderr, "%s: failed to open %s\n", ins_com1);
            return 7;
        }

        const unsigned char payload_header[] = {0xAA, 0x55, 1, 0, 0x42};
        unsigned short _checksum = 1 + 0x42;
        for (int i = 0; i < 60; ++i)
        {
            _checksum += payload[i];
        }

        const unsigned char checksum_bytes[] = {_checksum & 0xFF, _checksum >> 8};

        for (int i = 0; i < 2; ++i)
        {
            printf("0x%02x\n", checksum_bytes[i]);
        }

        int n = write(fd, command, sizeof(command));
        usleep(200*1000); // wait 200 milliseconds
        n += write(fd, payload_header, sizeof(payload_header));
        n += write(fd, payload, sizeof(payload));
        n += write(fd, checksum_bytes, sizeof(checksum_bytes));
        if (n < sizeof(payload_header) + sizeof(payload) + sizeof(checksum_bytes))
        {
            fprintf(stderr, "%s: failed to write all bytes "
                "(%d written)\n", argv[0], n);
        }

        usleep(3*1000*1000); // wait 3 seconds

        unsigned char response[20] = {0};
        int x = read(fd, response, sizeof(response));

        for (int i = 0; i < sizeof(response); ++i)
        {
            printf("%02x ", response[i]);
        }
        printf("\n");
    }

    return 0;
}
