/*        Console Utility for nooLite
        (c) Mikhail Ermolenko
        (c) Oleg Artamonov
*/

#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define DEV_VID 0x16c0 //0x5824
#define DEV_PID 0x05dc //0x1500
#define DEV_CONFIG 1
#define DEV_INTF 0
#define EP_IN 0x81
#define EP_OUT 0x01

unsigned char COMMAND_ACTION[8] = {0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; //{80,0x00,0xaa,0x00,0x0,0x1,0x14,0x05}

void usage()
{
    printf("Usage: nooliterxcfg <command> [channel]\n");
    printf("        <command> may be:\n");
    printf("                --clear - clear previously binded channel\n");
    printf("                --clearall - clear ALL channels\n");
    printf("                --bind - bind channel (30 seconds)\n");
    printf("                --stop - stop binding channel\n");
    printf("                --help - print help and exit\n");
    printf("        <channel> must be [1..64]\n");
}

int main(int argc, char * argv[])
{
        libusb_device_handle * handle;
        
        int i, ret;
        unsigned int level;
        unsigned char command[1], buf[8], channel;
        char param;
        
        if (argc == 1) {
                usage();
                return 0;
        }

        if (strcmp (argv[1],"--help")==0)
        {
            usage();
            return 0;
        }
        
        if (argc == 2) // no channel number expected
        {
            if ((strcmp(argv[1], "--bind") == 0) || (strcmp(argv[1], "--clear")))
            {
                printf("No channel number given.\n");
                usage();
                return -1;
            }
            
            if (strcmp(argv[1], "--stop") == 0) // 2 - остановить привязку принудительно
            {
                COMMAND_ACTION[0] = 2;
            }
            else if (strcmp(argv[1], "--clearall") == 0) // 4 - очистить всю память 
            {
                COMMAND_ACTION[0] = 4;
            }
            else
            {
                printf("Unknown command\n");
                usage();
                return -1;
            }
            
            COMMAND_ACTION[1] = 0; // no channel number needed for these commands
        }
        
        if (argc >= 3)
        {
            if (strcmp(argv[1], "--bind") == 0) // 1 - включить привязку на адрес ячейки, 30 секунд
            {
                COMMAND_ACTION[0] = 1;
            }
            else if (strcmp(argv[1], "--clear") == 0) // 3 - очистить ячейку
            {
                COMMAND_ACTION[0] = 3;
            }
            else
            {
                printf("Unknown command\n");
                usage();
                return -1;
            }
            
            channel = atoi(argv[2]);
            channel--;
            if ((channel>63)||(channel<0))
            {
                printf("Channel number out of range (1-64)\n", argv[0]);
                usage();
                return -1;
            }
            COMMAND_ACTION[1] = channel;
        }

//Prepare Command string
        libusb_init(NULL);
        libusb_set_debug(NULL, 3);
        handle = libusb_open_device_with_vid_pid(NULL, DEV_VID, DEV_PID);
        if (handle == NULL)
        {
            printf("No compatible USB devices were found.\n");
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

        ret = libusb_control_transfer(handle, LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_OUT, 0x9, 0x300, 0, COMMAND_ACTION, 8, 100);

        libusb_attach_kernel_driver(handle, DEV_INTF);
        libusb_close(handle);
        libusb_exit(NULL);
        
        return 0;
}
