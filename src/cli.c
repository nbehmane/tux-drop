#include "cli-internal.h"

void usage()
{
    fprintf(stderr, "\t-s x Scan for x seconds.\n");
    fprintf(stderr, "\t-l List current devices.\n");
    fprintf(stderr, "\t-e List debug info.\n");
    fprintf(stderr, "\t-c Connect to a device.\n");
    fprintf(stderr, "\t-d Disconnect from a device.\n");
    fprintf(stderr, "\t-r Remove a device.\n");
    fprintf(stderr, "\t-p Pair a device.\n");
}

static struct option long_options[] =
        {
                {"help", no_argument, 0, 'h'},
                {"scan", required_argument, 0, 's'},
                {"debug", no_argument, 0, 'e'},
                {"list", no_argument, 0, 'l'},
                {"pair", no_argument, 0, 'p'},
                {"connect", no_argument, 0, 'c'},
                {"remove", no_argument, 0, 'r'},
                {"quit", no_argument, 0, 'q'},
                {0, 0, 0, 0}
        };

/**
 * @brief Runs the CLI. Meant to be in the main loop.
 *
 * @param argc Count from main.
 * @param argv Argv from main.
 * @return
 */
int cli_run(int argc, char **argv, GDBusConnection *conn)
{
    int c = 0;
    int time_seconds = 0;
    while (1)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, "hs:elpcdrq", long_options, &option_index);
        if (c == -1)
            break;
        switch (c)
        {
            case 'h':
                usage();
                break;
            case 's': // Scan Time
                time_seconds = atoi(optarg);
                g_print("Scan Time\n");
                break;
            case 'e': // Debug List devices
                g_print("Debug List Devices\n");
                break;
            case 'l': // List devices
                g_print("List Devices\n");
                break;
            case 'p': // Pairing
                g_print("Pairing\n");
                break;
            case 'c': // Connecting
                g_print("Connecting\n");
                break;
            case 'd': // Disconnect
                g_print("Disconnect\n");
                break;
            case 'r': // Remove device
                g_print("Remove Device\n");
                break;
            case 'q': // Quit
                return -1;
            default:
                break;
        }
    }
}