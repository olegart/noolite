/*        Linux console utility for nooLite smart home PC receiver RX1164 (see http://www.noo.com.by/sistema-noolite.html)
        (c) Mikhail Ermolenko (ermolenkom@yandex.ru)
        (c) vvzvlad
        (c) Oleg Artamonov (oleg@olegart.ru)
*/

#include "nooliterx.h"

unsigned char COMMAND_ACTION[8] = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //{80,0x00,0xaa,0x00,0x0,0x1,0x14,0x05}

int main(int argc, char * argv[])
{
        int i, ret;
        unsigned int level;
        libusb_device_handle * usbhandle;
        unsigned char command[1], buf[8], channel, togl;
        char param;
        char commandtxt[255];

        int daemonize = 0;
        int timeout = 250; // default timeout 250ms
        int customcommand = 0;
        
        do_exit = 0;
        usbhandle = NULL;
        
        static struct sigaction act; 
        sigemptyset (&act.sa_mask);
        act.sa_flags = 0;
        act.sa_handler = SIG_IGN;
        sigaction (SIGHUP, &act, NULL);
        act.sa_handler = cleanup;
        sigaction (SIGINT, &act, 0);
        act.sa_handler =  cleanup;
        sigaction (SIGTERM, &act, 0);
        act.sa_handler =  cleanup;
        sigaction (SIGKILL, &act, 0);
        
        FILE* config_fp;
        char line[255] ;
        char* token;
    
        config_fp = fopen( "/etc/noolite.conf", "r" );
        if (config_fp)
        {
            while(fgets(line, 254, config_fp) != NULL)
            {
                token = strtok(line, "\t =\n\r");
                if (token != NULL && token[0] != '#')
                {
                    if (!strcmp(token, "command"))
                    {
                        strcpy(commandtxt, strtok(NULL, "\t=\n\r"));
                        while( *commandtxt==' ' )
                            memmove(commandtxt,commandtxt+1,strlen(commandtxt));
			customcommand = 1;
                    }
                    if (!strcmp(token, "timeout"))
                    {
                        timeout = atoi(strtok(NULL, "=\n\r"));
                    }
                }
            }
        }

        while ((i = getopt (argc, argv, "dc:t:h")) != -1)
        {
            switch (i)
            {
                case 'd':
                    daemonize = 1;
                break;
                case 't':
                    timeout = atoi(optarg);
                break;
                case 'c':
                    strcpy(commandtxt, optarg);
                    customcommand = 1;
                break;
                case 'h':
                    usage();
                    exit (EXIT_SUCCESS);
                break;
                case '?':
                    if (optopt == 'c')
                        fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                    else if (isprint (optopt))
                        fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                    else
                        fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                    usage();
                    exit(EXIT_SUCCESS);
            default:
            abort ();
           }
         }

        libusb_init(NULL);
        libusb_set_debug(NULL, 3);
        usbhandle = libusb_open_device_with_vid_pid(NULL, DEV_VID, DEV_PID);
        if (usbhandle == NULL)
        {
            printf("No compatible devices were found\n");
            libusb_exit(NULL);
            exit(EXIT_FAILURE);
        }
        
        if (libusb_kernel_driver_active(usbhandle,DEV_INTF))
        {
            libusb_detach_kernel_driver(usbhandle, DEV_INTF);
        }
        
        if ((ret = libusb_set_configuration(usbhandle, DEV_CONFIG)) < 0)
        {
            printf("USB configuration error\n");
            if (ret == LIBUSB_ERROR_BUSY)
                printf("B\n");
            printf("ret:%i\n", ret);
            libusb_close(usbhandle);
            libusb_exit(NULL);
            exit(EXIT_FAILURE);
        }
        
        if (libusb_claim_interface(usbhandle, DEV_INTF) < 0)
        {
            printf("USB interface error\n");
            libusb_close(usbhandle);
            libusb_exit(NULL);
            exit(EXIT_FAILURE);
        }
        
    // fork to background if needed and create pid file
    if (daemonize)
    {
        if (daemon(0, 0))
        {
            printf("Error forking to background\n");
            libusb_close(usbhandle);
            libusb_exit(NULL);
            exit(EXIT_FAILURE);
        }
        
        char pidval[10];
        int pidfile = open("/var/run/nooliterx.pid", O_CREAT | O_RDWR, 0666);
        if (lockf(pidfile, F_TLOCK, 0) == -1)
        {
            libusb_close(usbhandle);
            libusb_exit(NULL);
            exit(EXIT_FAILURE);
        }
        sprintf(pidval, "%d\n", getpid());
        write(pidfile, pidval, strlen(pidval));
    }
    
    ret = libusb_control_transfer(usbhandle, LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_IN, 0x9, 0x300, 0, buf, 8, 1000);
    togl = (buf[0] & 128);
    
    while (!do_exit)
    {
        char i;
        char cmd[255];
        if (timeout)
        {
            snprintf(cmd, 255, "timeout %i %s", timeout, commandtxt);
        }
        else
        {
            snprintf(cmd, 255, "%s", commandtxt);
        }
        
        ret = libusb_control_transfer(usbhandle, LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_IN, 0x9, 0x300, 0, buf, 8, 1000);
        if (togl!=(buf[0] & 128)) // TOGL is a 7th bit of the 1st data byte (adapter status), it toggles value every time new command received
        {
            togl = (buf[0] & 128);
            
            if (customcommand)
            {
                strcpy(cmd, str_replace(cmd, "%st", int_to_str(buf[0]))); // adapter status
                strcpy(cmd, str_replace(cmd, "%ch", int_to_str(buf[1]))); // channel
                strcpy(cmd, str_replace(cmd, "%cm", int_to_str(buf[2]))); // command
                strcpy(cmd, str_replace(cmd, "%df", int_to_str(buf[3]))); // data format
                strcpy(cmd, str_replace(cmd, "%d0", int_to_str(buf[4]))); // 1st data byte
                strcpy(cmd, str_replace(cmd, "%d1", int_to_str(buf[5]))); // 2nd data byte
                strcpy(cmd, str_replace(cmd, "%d2", int_to_str(buf[6]))); // 3rd data byte
                strcpy(cmd, str_replace(cmd, "%d3", int_to_str(buf[7]))); // 4th data byte
            }
            else
            {
                sprintf(cmd, "echo -e 'Adapter status:\t%i\\nChannel:\t%i\\nCommand:\t%i\\nData format:\t%i\\nData:\t\t%i %i %i %i\\n\\n'", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
            }
            system(cmd);
            //printf("\ncomm: %s\n", cmd);
        }
        usleep(150000);   
    }
    libusb_attach_kernel_driver(usbhandle, DEV_INTF);
    libusb_close(usbhandle);
    libusb_exit(NULL);
    remove("/var/run/nooliterx.pid");
}

void usage(void)
{
    printf("Usage: nooliterx [-c command] [-t timeout] [-d] [-h]\n");
    printf("  -c\tcommand to execute. Default is to print received data to stdout.\n");
    printf("  -t\tcommand execution timeout, milliseconds (0 to disable). Default is 250 (250 ms).\n");
    printf("  -d\trun in the background.\n");
    printf("  -h\tprint help and exit.\n");
    printf("\nCommand examples:\n");
    printf("  echo 'Status: %%st Channel: %%ch Command: %%cm Data format: %%df Data bytes: %%d0 %%d1 %%d2 %%d3'\n");
    printf("  wget http://localhost/noolight/?script=switchNooLitePress\\&channel=%%ch\\&command=%%cm\n");
    printf("\nAvailable variables (for detailed description, see RX1164 API manual at http://www.noo.com.by/):\n");
    printf("  %%st\t RX1164 adapter status\n");
    printf("  %%ch\t Channel number\n");
    printf("  %%cm\t Command number\n");
    printf("  %%df\t Data format\n");
    printf("  %%d0\t Data (1st byte)\n");
    printf("  %%d1\t Data (2nd byte)\n");
    printf("  %%d2\t Data (3rd byte)\n");
    printf("  %%d3\t Data (4th byte)\n");
}

char *str_replace(const char *s, const char *old, const char *new)
{
    char *ret;
    int i, count = 0;
    size_t newlen = strlen(new);
    size_t oldlen = strlen(old);

    for (i = 0; s[i] != '\0'; i++)
    {
        if (strstr(&s[i], old) == &s[i])
        {
            count++;
            i += oldlen - 1;
        }
    }

    ret = malloc(i + count * (newlen - oldlen));
    if (ret == NULL)
    {
        exit(EXIT_FAILURE);
    }
    
    i = 0;
    while (*s)
    {
        if (strstr(s, old) == s)
        {
            strcpy(&ret[i], new);
            i += newlen;
            s += oldlen;
        }
        else
        {
            ret[i++] = *s++;
        }
    }
    
    ret[i] = '\0';
    return ret;
}

char* int_to_str(int num)
{
    static char retstr[4];
    snprintf(retstr, 4, "%d", num);
    return retstr;
}

void cleanup(int sig)
{
    do_exit = 1;
}
