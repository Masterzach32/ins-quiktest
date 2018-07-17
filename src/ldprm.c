#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

// LoadINSPar data structure as specified in INS ICD
// revision 2.9, section 6.3.4
struct short_prm_t
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

// converts LoadINSPar payload to short_prm_t struct; if pointer to struct
// is null, does nothing (note: uses little-endian byte ordering)
void payload2short_prm(struct short_prm_t *prm, const unsigned char payload[60])
{
    if (!prm || !payload) return;

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

// does the opposite of the above function - converts short_prm_t struct
// to 60 byte payload
void struct2payload(const struct short_prm_t *prm, unsigned char payload[60])
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

// prints short_prm_t struct to stdout
// note: values are printed in SI units, as opposed to the way they're
// stored in the data structure; for example, magnetic declination
// is expressed in degrees, rather than hundredths of degrees
void print_struct(struct short_prm_t prm)
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

// converts user input, unsigned long, to the type
// used by fcntl.h, speed_t
speed_t int2speed_t(unsigned long baudrate)
{
    switch (baudrate)
    {
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        case 460800: return B460800;
        case 500000: return B500000;
        case 576000: return B576000;
        case 921600: return B921600;
        case 1000000: return B1000000;
        case 1152000: return B1152000;
        case 1500000: return B1500000;
        case 2000000: return B2000000;
        case 2500000: return B2500000;
        case 3000000: return B3000000;
        case 3500000: return B3500000;
        case 4000000: return B4000000;
        default: return -1;
    }
}

char* speed2str(speed_t speed)
{
    switch (speed)
    {
        case B4800: return "B4800";
        case B9600: return "B9600";
        case B19200: return "B19200";
        case B38400: return "B38400";
        case B57600: return "B57600";
        case B115200: return "B115200";
        case B230400: return "B230400";
        case B460800: return "B460800";
        case B500000: return "B500000";
        case B576000: return "B576000";
        case B921600: return "B921600";
        case B1000000: return "B1000000";
        case B1152000: return "B1152000";
        case B1500000: return "B1500000";
        case B2000000: return "B2000000";
        case B2500000: return "B2500000";
        case B3000000: return "B3000000";
        case B3500000: return "B3500000";
        case B4000000: return "B4000000";
    }
    return "[BAUDRATE]";
}

unsigned char speed2id(speed_t speed)
{
    switch (speed)
    {
        case B4800: return 1;
        case B9600: return 2;
        // case B14400: return 3;
        case B19200: return 4;
        case B38400: return 5;
        case B57600: return 6;
        case B115200: return 7;
        case B230400: return 8;
        case B460800: return 9;
        case B921600: return 10;
    }
    return 0;
}

const char* argument_error =
    "%s: invalid option -- '%s'\n"
    "type '%s --usage' for more info\n";

const char* usage_help =
    "usage: %s device [-n -v -h] [-b br] [-r dr] [-i s] [-l lx ly lz] [-a h p r]\n"
    "  device: INS COM1 serial device path\n"
    "  [-n]: print INS serial number\n"
    "  [-v]: verbose output\n"
    "  [-h]: print INS params in hex\n"
    "  br: operating bitrate for COM1\n"
    "  dr: data rate of INS output, in Hz; must be factor of 200 Hz\n"
    "  s: INS initial alignment time in seconds\n"
    "  lx ly lz: offset from imu to antenna, in meters\n"
    "  h p r: angle offset from vehicle orientation, in degrees\n";

// valid data rates for INS data frame output, in Hz
const unsigned char valid_rates[] = {1, 2, 4, 5, 8, 10, 20, 25, 40, 50, 100, 200};

// possible INS baudrates, in order of likelihood
const speed_t valid_bps[] =
    {B460800, B115200, B921600, B230400, B4800,
     B9600, B19200, B38400, B57600};

int main(int argc, char** argv)
{
    if (argc < 2) // first argument must be INS COM1 device
    {
        fprintf(stderr, "%s: must provide file path\n", argv[0]);
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
    int com1 = open(argv[1], O_RDWR | O_NOCTTY | O_NDELAY);
    if (com1 == -1)
    {
        fprintf(stderr, "%s: failed to open %s\n", argv[0], argv[1]);
        return 1;
    }

    // these flags indicate whether each flag has appeared in argv,
    // rate_flag for -r, init_flag for -i, etc. The default state is 0.
    unsigned char baud_flag = 0;
    unsigned char rate_flag = 0;
    unsigned char init_flag = 0;
    unsigned char lever_flag = 0;
    unsigned char angle_flag = 0;
    unsigned char print_flag = 0;
    unsigned char hex_flag = 0;
    unsigned char name_flag = 0;

    // if user provides arguments, they'll be stored here
    speed_t baud_input;
    unsigned char rate_input;
    unsigned char init_input;
    double lever_input[3];
    double angle_input[3];

    for (int i = 2; i < argc; ++i) // process every element in argv
    {
        // baudrate flag
        if (!strcmp(argv[i], "-b") | !strcmp(argv[i], "--baud"))
        {
            if (argc < i + 2)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 1;
            }
            baud_input = int2speed_t(atoi(argv[++i]));
            baud_flag = 1;

            if (baud_input != B115200 && baud_input != B230400 &&
                baud_input != B460800 && baud_input != B921600)
            {
                fprintf(stderr, "%s: error: invalid or unsupported "
                    "bitrate '%s'\n", argv[0], argv[i]);
                return 1;
            }
        }
        // data rate flag
        else if (!strcmp(argv[i], "-r") | !strcmp(argv[i], "--rate"))
        {
            if (argc < i + 2)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 1;
            }
            rate_input = atoi(argv[++i]);

            // this mess is required because only some data rates are allowed
            // by the INS; if an invalid data rate is provided by the user,
            // the program must return an error and a helpful message

            unsigned char num_of_rates = sizeof(valid_rates)/sizeof(valid_rates[0]);
            for (int j = 0; j < num_of_rates; ++j)
            {
                // check every valid rate; if the provided rate is one of the
                // valid ones, the program will use it

                if (rate_input == valid_rates[j])
                {
                    rate_flag = 1;
                }
            }
            if (!rate_flag) // if the provided rate is invalid...
            {
                fprintf(stderr, "%s: valid data rates are: ", argv[0]);

                // print every valid rate, as provided above main()
                for (int j = 0; j < num_of_rates; ++j)
                {
                    fprintf(stderr, "%hhu", valid_rates[j]);
                    if (j < num_of_rates - 1) fprintf(stderr, ", ");
                }
                fprintf(stderr, "\n");
                return 1;
            }
        }
        // init alignment time flag
        else if (!strcmp(argv[i], "-i") | !strcmp(argv[i], "--init"))
        {
            if (argc < i + 2)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 1;
            }
            init_flag = 1;
            init_input = atoi(argv[++i]);
        }
        // lever arm flag
        else if (!strcmp(argv[i], "-l") | !strcmp(argv[i], "--lever"))
        {
            // verify that there are atleast 3 more arguments, because
            // -l expects lx, ly, and lz
            if (argc < i + 4)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 1;
            }

            lever_flag = 1;
            lever_input[0] = atof(argv[++i]);
            lever_input[1] = atof(argv[++i]);
            lever_input[2] = atof(argv[++i]);

            // likewise, i is incremented by 3, because -l uses 3 arguments
        }
        // alignment angle flag
        else if (!strcmp(argv[i], "-a") | !strcmp(argv[i], "--angles"))
        {
            // -a is similar to -l in structure
            if (argc < i + 4)
            {
                fprintf(stderr, usage_help, argv[0]);
                return 1;
            }
            angle_flag = 1;
            angle_input[0] = atof(argv[++i]);
            angle_input[1] = atof(argv[++i]);
            angle_input[2] = atof(argv[++i]);
        }
        // print to stdout flag
        else if (!strcmp(argv[i], "-v") | !strcmp(argv[i], "--verbose"))
        {
            print_flag = 1;
        }
        // print hex to stdout flag
        else if (!strcmp(argv[i], "-h") | !strcmp(argv[i], "--hex"))
        {
            hex_flag = 1;
        }
        // print SN to stdout flag
        else if (!strcmp(argv[i], "-n") | !strcmp(argv[i], "--name"))
        {
            name_flag = 1;
        }
        else // if any argument is unexpected, throw argument error
        {
            fprintf(stderr, argument_error, argv[0], argv[i], argv[0]);
            return 1;
        }
    }

    // this flag will be 1 if any write commands were issued in argv; if not,
    // the program doesn't need to send a LoadINSPar command at all
    unsigned char write_flag = rate_flag | init_flag | lever_flag | angle_flag;

    { // setting serial device configuration
        /////////////////////////////////////////////
        // altering terminal serial device settings
        // warning: very hard to read
        struct termios settings;
        tcgetattr(com1, &settings);
        settings.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP |
            INLCR | IGNCR | ICRNL | IXON );
        settings.c_oflag &= ~(OPOST | ONLCR);
        settings.c_lflag &= ~(ISIG | ICANON | IEXTEN | ECHO | ECHOE |
            ECHOK | ECHOCTL | ECHOKE);
        settings.c_cflag &= ~(CSIZE | PARENB);
        settings.c_cflag |= CS8;
        settings.c_cc[VMIN] = 0;
        settings.c_cc[VTIME] = 0;
        tcsetattr(com1, TCSANOW, &settings);
        tcflush(com1, TCOFLUSH);
        /////////////////////////////////////////////
    }

    const unsigned char num_of_bps = sizeof(valid_bps)/sizeof(valid_bps[0]);
    int determined_bps = 0;
    speed_t baudrate;

    for (int i = 0; i < num_of_bps && !determined_bps; ++i)
    {
        struct termios settings;
        tcgetattr(com1, &settings);
        cfsetspeed(&settings, valid_bps[i]);
        tcsetattr(com1, TCSANOW, &settings);
        tcflush(com1, TCOFLUSH);

        if (print_flag) printf("Probing %s...\n", speed2str(valid_bps[i]));

        const unsigned char STOP_command[] =
            {0xAA, 0x55, 0, 0, 7, 0, 0xFE, 0x05, 0x01};
        const unsigned char GetBIT_command[] =
            {0xAA, 0x55, 0, 0, 7, 0, 0x1A, 0x21, 0x00};
        write(com1, STOP_command, sizeof(STOP_command));
        usleep(1*1000); // sleep for 100 ms
        unsigned char trash[10000];
        read(com1, trash, sizeof(trash));
        write(com1, GetBIT_command, sizeof(GetBIT_command));
        usleep(1.5*1000*1000); // sleep for 1.5 seconds
        unsigned char bit_response[12];
        int x = read(com1, bit_response, 12);

        unsigned short msg_check = bit_response[10] | (bit_response[11] << 8);
        unsigned short calc_check = 0;
        for (int j = 2; j < x - 2; ++j) calc_check += bit_response[j];

        if (x == 12 && msg_check == calc_check)
        {
            tcgetattr(com1, &settings);
            cfsetspeed(&settings, valid_bps[i]);
            tcsetattr(com1, TCSANOW, &settings);
            tcflush(com1, TCOFLUSH);
            determined_bps = 1;
            baudrate = valid_bps[i];
        }
        write(com1, STOP_command, sizeof(STOP_command));
    }

    if (!determined_bps)
    {
        fprintf(stderr, "%s: error: could not determine baudrate\n", argv[0]);
        return 1;
    }

    if (baud_flag && baudrate != baud_input) // changing COM1 baudrate
    {
        if (print_flag) printf("Changing baudrate from %s to %s\n",
            speed2str(baudrate), speed2str(baud_input));

        unsigned char read_full_prm[] = {0xAA, 0x55, 0, 0, 7, 0, 0x0B, 0x12, 0};
        unsigned char write_full_prm[] =  {0xAA, 0x55, 0, 0, 7, 0, 0x0E, 0x15, 0};
        unsigned char change_com1_bps[] = {0xAA, 0x55, 0, 0, 7, 0, 0xC2, 0xC9, 0};
        int x = write(com1, read_full_prm, sizeof(read_full_prm));
        if (x != sizeof(read_full_prm))
        {
            fprintf(stderr, "%s: error: wrote %d/8 bytes\n", argv[0], x);
            return 1;
        }
        usleep(500*1000); // sleep 500 ms
        unsigned char full_prm_frame[2056];
        x = read(com1, full_prm_frame, 2056);
        if (x != 2056)
        {
            fprintf(stderr, "%s: error: read %d/2056 bytes\n", argv[0], x);
            return 1;
        }

        unsigned short check_calc = 0;
        for (int i = 2; i < x - 2; ++i)
        {
            check_calc += full_prm_frame[i];
        }
        unsigned short checksum = full_prm_frame[2054] |
                                 (full_prm_frame[2055] << 8);
        if (check_calc != checksum)
        {
            fprintf(stderr, "%s: error: checksum mismatch in "
                "full parameter file", argv[0]);
            return 1;
        }

        full_prm_frame[6 + 946] = speed2id(baud_input);
        full_prm_frame[2] = 0;
        full_prm_frame[3] = 0;
        full_prm_frame[4] = 6;
        full_prm_frame[5] = 8;
        check_calc = 0;
        for (int i = 2; i < x - 2; ++i)
        {
            check_calc += full_prm_frame[i];
        }
        full_prm_frame[2054] = check_calc & 0xFF;
        full_prm_frame[2055] = check_calc >> 8;

        write(com1, write_full_prm, sizeof(write_full_prm));
        usleep(100*1000); // sleep 100 ms
        write(com1, full_prm_frame, sizeof(full_prm_frame));
        usleep(2*1000*1000); // sleep 2 seconds
        write(com1, change_com1_bps, sizeof(change_com1_bps));
        usleep(600*1000); // sleep 600 ms

        struct termios settings;
        tcgetattr(com1, &settings);
        cfsetspeed(&settings, baud_input);
        tcsetattr(com1, TCSANOW, &settings);
        tcflush(com1, TCOFLUSH);

        unsigned char dump[100];
        read(com1, dump, sizeof(dump));
    }

    // at this point the command line arguments are processed and the program
    // will have exited if any were invalid.

    // this block reads the entire ReadINSPar message and verifies that
    // the message structure is as expected, but only keeps the payload
    unsigned char payload[60];
    {
        const unsigned char ReadINSPar_command[] =
            {0xAA, 0x55, 0, 0, 7, 0, 0x41, 0x48, 0};
        unsigned char header[6], checksum[2];

        int x = write(com1, ReadINSPar_command, sizeof(ReadINSPar_command));
        if (x != sizeof(ReadINSPar_command))
        {
            fprintf(stderr, "%s: error sending ReadINSPar command "
                            "(%d bytes written)\n", argv[0], x);
            return 1;
        }
        usleep(50*1000); // wait for 50 milliseconds
        x = read(com1, header, sizeof(header)); // read the header
        x += read(com1, payload, sizeof(payload)); // read the payload
        x += read(com1, checksum, sizeof(checksum)); // checksum

        // TODO: verify the message contents

        if (x != sizeof(header) + sizeof(payload) + sizeof(checksum))
        {
            fprintf(stderr, "%s: error receiving ReadINSPar message "
                            "(%d bytes read)\n", argv[0], x);
            return 1;
        }
    }

    // take the payload and convert to usable primitives
    struct short_prm_t dat;
    payload2short_prm(&dat, payload);

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
    if (name_flag) printf("%s\n", dat.device_name);

    // if no write flags are enabled, exit here
    if (!write_flag) return 0;

    // convert the struct back to a byte payload to prepare to send
    struct2payload(&dat, payload);

    const unsigned char command[] = {0xAA, 0x55, 0, 0, 7, 0, 0x40, 0x47, 0};
    const unsigned char header[] = {0xAA, 0x55, 1, 0, 0x42, 0};
    unsigned short checksum = 1 + 0x42;
    for (int i = 0; i < 60; ++i)
    {
        checksum += payload[i];
    }
    const unsigned char checksum_bytes[] = {checksum & 0xFF, checksum >> 8};
    unsigned char combined[68];
    memcpy(combined, header, sizeof(header));
    memcpy(combined + sizeof(header), payload, sizeof(payload));
    memcpy(combined + sizeof(header) + sizeof(payload),
        checksum_bytes, sizeof(checksum_bytes));

    int n = write(com1, command, sizeof(command));
    usleep(200*1000); // wait 200 milliseconds
    n += write(com1, combined, sizeof(combined));
    if (n < sizeof(combined))
    {
        fprintf(stderr, "%s: failed to write all bytes "
            "(%d written)\n", argv[0], n);
    }

    // TODO: verify checksum match

    return 0;
}
