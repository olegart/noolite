/*        Linux console utility for nooLite smart home PC receiver RX1164 (see http://www.noo.com.by/sistema-noolite.html)
        (c) Mikhail Ermolenko (ermolenkom@yandex.ru)
        (c) vvzvlad
        (c) Oleg Artamonov (oleg@olegart.ru)
*/

#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define DEV_VID 0x16c0 //0x5824
#define DEV_PID 0x05dc //0x1500
#define DEV_CONFIG 1
#define DEV_INTF 0
#define EP_IN 0x81
#define EP_OUT 0x01

unsigned char COMMAND_ACTION[8] = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //{80,0x00,0xaa,0x00,0x0,0x1,0x14,0x05}

void usage()
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
        return(NULL);
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

int main(int argc, char * argv[])
{
        libusb_device_handle * handle;
        int i, ret;
        unsigned int level;
        unsigned char command[1], buf[8], channel, togl;
        char param;
        char commandtxt[255];

        int daemonize = 0;
        int timeout = 250; // default timeout 250ms
        int customcommand = 0;
        
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
                    return 0;
                break;
                case '?':
                    if (optopt == 'c')
                        fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                    else if (isprint (optopt))
                        fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                    else
                        fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                    usage();
                return 1;
            default:
            abort ();
           }
         }

        libusb_init(NULL);
        libusb_set_debug(NULL, 3);
        handle = libusb_open_device_with_vid_pid(NULL, DEV_VID, DEV_PID);
        if (handle == NULL)
        {
            printf("No compatible devices were found\n");
            libusb_exit(NULL);
            return 0;
        }
        
        if (libusb_kernel_driver_active(handle,DEV_INTF))
        {
            libusb_detach_kernel_driver(handle, DEV_INTF);
        }
        
        if ((ret = libusb_set_configuration(handle, DEV_CONFIG)) < 0)
        {
            printf("USB configuration error\n");
            libusb_close(handle);
            libusb_exit(NULL);
            if (ret == LIBUSB_ERROR_BUSY)
                printf("B\n");
            printf("ret:%i\n", ret);
            return 0;
        }
        
        if (libusb_claim_interface(handle, DEV_INTF) < 0)
        {
            printf("USB interface error\n");
            libusb_close(handle);
            libusb_exit(NULL);
            return 0;
        }
        //0x9 - номер запроса
        //0x300 - значение запроса - их надо получить из мониторинга

        //ret = libusb_control_transfer(handle, LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_IN, 0x9, 0x300, 0, COMMAND_ACTION, 8, 100);

    // fork to background if needed
    if (daemonize)
    {
        if (daemon(0, 0))
        {
            printf("Error forking to background\n");
            libusb_close(handle);
            libusb_exit(NULL);
            return -1;
        }
    }
    
    ret = libusb_control_transfer(handle, LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_IN, 0x9, 0x300, 0, buf, 8, 1000);
    togl = (buf[0] & 128);
    
    while (1)
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
        
        ret = libusb_control_transfer(handle, LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_IN, 0x9, 0x300, 0, buf, 8, 1000);
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
/*
        scanf("%c",i);

        if (i=='\r')
        {
            printf("Выход\n");
            break;
        }
*/        
    }

    libusb_attach_kernel_driver(handle, DEV_INTF);
    libusb_close(handle);
    libusb_exit(NULL);
    if (commandtxt)
        free (commandtxt);
    return 0;
}
