/*   Consloe Utility for nooLite PC11xx
     (c) Mikhail Ermolenko
     (c) Oleg Artamonov
*/

#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define DEV_VID 0x16c0 //0x5824
#define DEV_PID 0x05df //0x1503
#define DEV_CONFIG 1
#define DEV_INTF 0

unsigned char COMMAND_ACTION[8] = {0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; 

int main(int argc, char * argv[])
{
     libusb_device_handle * handle;
    
     int i, ret;
     unsigned int level;
     unsigned char command[1], buf[8], channel;
     char param;
    
     //Arg Control
     
     if (argc == 1) {
          printf("Usage: %s --help\n", argv[0]);
          return -1;
     }

    if (strcmp (argv[1],"--help")==0)
    {
        printf("Usage: %s <command> <channel> [<level>|<RGB>]\n", argv[0]);
        printf("     <command>:\n");
        printf("          --on - Turn channel ON\n");
        printf("          --off - Turn channel OFF\n");
        printf("          --switch - Switch channel ON/OFF\n");
        printf("          --set - Set level for channel\n");
        printf("          --bind - Bind channel\n");
        printf("          --unbind - Unbind channel\n");
        printf("          --load - Load preset channel\n");
        printf("          --save - Save preset channel\n");
        printf("          --stop - Stop changing level\n");
        printf("          --color_roll - Rolling color\n");
        printf("          --color_switch - Switch color\n");
        printf("          --color - Set color R[0..255] G[0..255] B[0..255]\n");        
        printf("          --mode - Switch mode\n");
        printf("          --mode_speed - Switch mode speed\n");
        printf("     <channel> must be [1..32]\n");
        printf("     <level> must be [0..100] - use with -set_ch\n");
        printf("     <RGB> must be [0..255] [0..255] [0..255] - use with -set_color\n");
        return -1;
    }

    if (argc >= 3)
    {
        if (strcmp (argv[1],"--on")==0)  //Set cnannel ON
        {
            COMMAND_ACTION[1] = 2;
        }
        else if (strcmp(argv[1],"--off")==0) //Set channel OFF
        {
            COMMAND_ACTION[1] = 0;
        }
        else if (strcmp(argv[1],"--switch")==0) //Switch channel ON/OFF
        {
            COMMAND_ACTION[1] = 4;
        }
        else if (strcmp(argv[1],"--set")==0) //Set level on channel - needed arg "level"
        {
            COMMAND_ACTION[1] = 6;
            COMMAND_ACTION[2] = 1; // формат
            if (argc >= 4)
            {
                level     = atoi(argv[3]);
            }
            else
            {
                printf("Missing brightness value. \nUsage: %s <command> <channel> [<level>]\n", argv[0]);
                return -1;
            }
            if (level>100)
            {
                level=100;
            }
            if (level<0)
            {
                level=0;
            }
            if (level>0)
            {  
                level=(int)(34+(float)level*1.23);
            }
            COMMAND_ACTION[5]= level;
        } 
        else if (strcmp(argv[1],"--bind")==0) //Привязать канал
        {
            COMMAND_ACTION[1] = 15;
        }
        else if (strcmp(argv[1],"--unbind")==0) //отвязать канал
        {
            COMMAND_ACTION[1] = 9;
        }
        else if (strcmp(argv[1],"--preset")==0) //Вызов записанного ранее в программе сценария освещения presetX, где X – номер сценария в программе (1…5)
        {
            //    COMMAND_ACTION[1] = ?; // не реализовано
        } 
        else if (strcmp(argv[1],"--load")==0) //Команда вызова записанного сценария из памяти силового блока для канала X
        {
            COMMAND_ACTION[1] = 7;
        } 
        else if (strcmp(argv[1],"--save")==0) //Команда записи сценария в память силового блока для канала X
        {
            COMMAND_ACTION[1] = 8;
        } 
        else if (strcmp(argv[1],"--stop")==0) //остановить регулировку 
        {
            COMMAND_ACTION[1] = 10;
        } 
        else if (strcmp(argv[1],"--color_roll")==0) //включение плавного перебора цвета, выключается командой 10.
        {
            COMMAND_ACTION[1] = 16;
            COMMAND_ACTION[2] = 4; // формат
        } 
        else if (strcmp(argv[1],"--color_switch")==0) //переключение цвета 
        {
            COMMAND_ACTION[1] = 17;
            COMMAND_ACTION[2] = 4; // формат
        } 
        else if (strcmp(argv[1],"--mode")==0) //переключение режима работы 
        {
            COMMAND_ACTION[1] = 18;
            COMMAND_ACTION[2] = 4; // формат
        } 
        else if (strcmp(argv[1],"--mode_speed")==0) //переключение скорости эффекта в режиме работы
        {
            COMMAND_ACTION[1] = 19;
            COMMAND_ACTION[2] = 4; // формат
        } 
        else if (strcmp(argv[1],"--color")==0) //Установка яркости на каждый канал независимо (R - 1, G - 2, B - 3). Уровень передается параметрами в формате 0…255
        { 
            COMMAND_ACTION[1] = 6;
            COMMAND_ACTION[2] = 3; // формат
            COMMAND_ACTION[5] = atoi(argv[3]); // R
            COMMAND_ACTION[6] = atoi(argv[4]); // G
            COMMAND_ACTION[7] = atoi(argv[5]); // B
        } 
        else 
        {
            printf("Command unknown\n");
            return -1;
        }
    }
    else
    {
        printf("Unknownw command.\nUsage: %s <command> <channel> [<level>]\n", argv[0]);
        return -1;
    }

    if (argc >= 3)
    {
        channel = atoi(argv[2]);
        channel--;
        COMMAND_ACTION[4] = channel;
    }
    else
    {
        printf("No channel number.\nUsage: %s <command> <channel> [<level>]\n", argv[0]);
        return -1;
    }

    //Prepare Command string
    libusb_init(NULL);
    libusb_set_debug(NULL, 3);
    handle = libusb_open_device_with_vid_pid(NULL, DEV_VID, DEV_PID);
    if (handle == NULL)
    {
        printf("No compatible devices were found.\n");
        libusb_exit(NULL);
        return 0;
    }

    char str_desc[10];
    struct libusb_device_descriptor desc;
    libusb_get_device_descriptor(libusb_get_device(handle), &desc);
    libusb_get_string_descriptor_ascii(handle, desc.iProduct, str_desc, 10);
    if ( (channel < 0) || (channel >= atoi(str_desc+4)))
    {
    printf("Channel number is out of range (1-%d for the %s transmitter you are using)\nUsage: %s <command> <channel> [<level>]\n", atoi(str_desc+4), str_desc, argv[0]);
        return -1;
    } 

    if (libusb_kernel_driver_active(handle,DEV_INTF))
    {
        libusb_detach_kernel_driver(handle, DEV_INTF);
    }
    if ((ret = libusb_set_configuration(handle, DEV_CONFIG)) < 0)
    {
        printf("USB configuration error %i.\n", ret);
        libusb_close(handle);
        libusb_exit(NULL);
        return 0;
    }
    if (libusb_claim_interface(handle,  DEV_INTF) < 0)
    {
        printf("USB interface error.\n");
        libusb_close(handle);
        libusb_exit(NULL);
        return 0;
    }
    
    //0x9 - номер запроса
    //0x300 - значение запроса - их надо получить из мониторинга

    if ((ret = libusb_control_transfer(handle, LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_OUT, 0x9, 0x300, 0, COMMAND_ACTION, 8, 100)) < 0)
    {
        printf("USB data transfer error %i.\n", ret);
    }

    libusb_attach_kernel_driver(handle, DEV_INTF);
    libusb_close(handle);
    libusb_exit(NULL);
    
    return 0;
}
