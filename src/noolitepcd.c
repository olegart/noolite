/*   nooLite PC11xx transmitter daemon
*/

#include "noolitepcd.h"

const char BITRATE = 2; // 2 = 1000 bps, 3 = 500 bps
const char REPEAT = 1; // repeat N times
const long INTERVAL = 400000000L; // 400 ms interval between transmit, suitable for 1000 bps transmission speed and 1 repeat

unsigned char COMMAND_ACTION[8] = {0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; 

int main(int argc, char * argv[])
{
    COMMAND_ACTION[0] = (BITRATE << 3) + (REPEAT << 5);

    libusb_device_handle * handle;
    
    int i, ret;
    unsigned int level;
    unsigned char command[1], buf[8], channel;
    char param;
    unsigned char channels;
    FILE *fifo;
    
    char input[26];
    
    do_exit = 0;
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
    
    setlogmask(LOG_UPTO(LOG_INFO));

    int s, s2, t, len;
    struct sockaddr_un local, remote;
    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        printf("Socket error\n");
        exit(EXIT_FAILURE);
    }
    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, NSOCKET);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(s, (struct sockaddr *)&local, len) == -1)
    {
        printf("Socket bind failed\n");
        exit(EXIT_FAILURE);
    }
    if (listen(s, 5) == -1) {
        printf("Socket listen failed\n");
        exit(EXIT_FAILURE);
    }
    
    libusb_init(NULL);
    libusb_set_debug(NULL, 3);
    handle = libusb_open_device_with_vid_pid(NULL, DEV_VID, DEV_PID);

    if (handle == NULL)
    {
        printf("No compatible devices were found.\n");
        libusb_exit(NULL);
        exit(EXIT_FAILURE);
    }

    char str_desc[10];
    struct libusb_device_descriptor desc;
    libusb_get_device_descriptor(libusb_get_device(handle), &desc);
    libusb_get_string_descriptor_ascii(handle, desc.iProduct, str_desc, 10);
    channels = atoi(str_desc+4);
    
    if (libusb_kernel_driver_active(handle,DEV_INTF))
    {
        libusb_detach_kernel_driver(handle, DEV_INTF);
    }
    if ((ret = libusb_set_configuration(handle, DEV_CONFIG)) < 0)
    {
        printf("USB configuration error %i.\n", ret);
        libusb_close(handle);
        libusb_exit(NULL);
        exit(EXIT_FAILURE);
    }
    if (libusb_claim_interface(handle,  DEV_INTF) < 0)
    {
        printf("USB interface error.\n");
        libusb_close(handle);
        libusb_exit(NULL);
        exit(EXIT_FAILURE);
    }

    // fork to background
    if (argc > 1)
    {
        if (strcmp (argv[1], "-d") == 0)  // run as daemon
        {
            if (daemon(0, 0))
            {
                printf("Error forking to background\n");
                libusb_close(handle);
                libusb_exit(NULL);
                exit(EXIT_FAILURE);
            }
        }
    }
    
    char pidval[10];
    int pidfile = open(PID_NAME, O_CREAT | O_RDWR, 0666);
    
    if (lockf(pidfile, F_TLOCK, 0) == -1)
    {
        libusb_close(handle);
        libusb_exit(NULL);
        exit(EXIT_FAILURE);
    }
    sprintf(pidval, "%d\n", getpid());
    write(pidfile, pidval, strlen(pidval));
    
    openlog("noolitepcd", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
    syslog(LOG_INFO, "noolitepcd started");
    
    while (!do_exit)
    {
        s2 = accept(s, (struct sockaddr *)&remote, &t);
        i = recv(s2, input, 25, 0);
        input[i] = 0; // null-terminated string
        close(s2);
        
        char * cmd[5];
        cmd[0] = strtok(input, "- \n");
        for (i=1; i<5; i++)
        {
            cmd[i] = strtok(NULL, "- \n");
        }
            
        if (strcmp (cmd[0], "on") == 0)  //Set cnannel ON
        {
            COMMAND_ACTION[1] = 2;
        }
        else if (strcmp(cmd[0], "off") == 0) //Set channel OFF
        {
            COMMAND_ACTION[1] = 0;
        }
        else if (strcmp(cmd[0], "switch") == 0) //Switch channel ON/OFF
        {
            COMMAND_ACTION[1] = 4;
        }
        else if (strcmp(cmd[0], "set") == 0) //Set level on channel - needed arg "level"
        {
            COMMAND_ACTION[1] = 6;
            COMMAND_ACTION[2] = 1; // формат
            if (cmd[2] != NULL)
            {
                level = atoi(cmd[2]);
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
        else if (strcmp(cmd[0], "bind") == 0) //Привязать канал
        {
            COMMAND_ACTION[1] = 15;
        }
        else if (strcmp(cmd[0], "unbind") == 0) //отвязать канал
        {
            COMMAND_ACTION[1] = 9;
        }
        else if (strcmp(cmd[0], "preset") == 0) //Вызов записанного ранее в программе сценария освещения presetX, где X – номер сценария в программе (1…5)
        {
            //COMMAND_ACTION[1] = ?; // не реализовано
        } 
        else if (strcmp(cmd[0], "load") == 0) //Команда вызова записанного сценария из памяти силового блока для канала X
        {
            COMMAND_ACTION[1] = 7;
        } 
        else if (strcmp(cmd[0], "save") == 0) //Команда записи сценария в память силового блока для канала X
        {
            COMMAND_ACTION[1] = 8;
        } 
        else if (strcmp(cmd[0], "stop") == 0) //остановить регулировку 
        {
            COMMAND_ACTION[1] = 10;
        } 
        else if (strcmp(cmd[0], "color_roll") == 0) //включение плавного перебора цвета, выключается командой 10.
        {
            COMMAND_ACTION[1] = 16;
            COMMAND_ACTION[2] = 4; // формат
        } 
        else if (strcmp(cmd[0], "color_switch") == 0) //переключение цвета 
        {
            COMMAND_ACTION[1] = 17;
            COMMAND_ACTION[2] = 4; // формат
        } 
        else if (strcmp(cmd[0], "mode") == 0) //переключение режима работы 
        {
            COMMAND_ACTION[1] = 18;
            COMMAND_ACTION[2] = 4; // формат
        } 
        else if (strcmp(cmd[0], "mode_speed") == 0) //переключение скорости эффекта в режиме работы
        {
            COMMAND_ACTION[1] = 19;
            COMMAND_ACTION[2] = 4; // формат
        } 
        else if (strcmp(cmd[0], "color") == 0) //Установка яркости на каждый канал независимо (R - 1, G - 2, B - 3). Уровень передается параметрами в формате 0…255
        { 
            COMMAND_ACTION[1] = 6;
            COMMAND_ACTION[2] = 3; // формат
            COMMAND_ACTION[5] = atoi(cmd[2]); // R
            COMMAND_ACTION[6] = atoi(cmd[3]); // G
            COMMAND_ACTION[7] = atoi(cmd[4]); // B
        }
        else
        {
            continue;
        }
            
        if (cmd[1] != NULL)
        {
            channel = atoi(cmd[1]);
            channel--;
            COMMAND_ACTION[4] = channel;
        }
            
        //0x9 - номер запроса
        //0x300 - значение запроса - их надо получить из мониторинга

        if ((ret = libusb_control_transfer(handle, LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_OUT, 0x9, 0x300, 0, COMMAND_ACTION, 8, 100)) < 0)
        {
            printf("USB data transfer error %i.\n", ret);
            syslog(LOG_ERR, "USB data transfer error %i", ret);
        }
        syslog(LOG_INFO, "Sent: %s: mode %i, command %i, format %i, address %i %i, data %i %i %i", input, COMMAND_ACTION[0], COMMAND_ACTION[1], COMMAND_ACTION[2], COMMAND_ACTION[3], COMMAND_ACTION[4], COMMAND_ACTION[5], COMMAND_ACTION[6], COMMAND_ACTION[7]);
		
        struct timespec tw = {0, INTERVAL}; // wait for current transmission to complete
        while (nanosleep (&tw, &tw) == -1) continue;
    }
    
    libusb_attach_kernel_driver(handle, DEV_INTF);
    libusb_close(handle);
    libusb_exit(NULL);
    syslog(LOG_INFO, "noolitepcd terminated");
    closelog();
    
    lockf(pidfile, F_ULOCK, 0);
    close(pidfile);
    remove(PID_NAME);
    
    return 0;
}

void cleanup(int sig)
{
    do_exit = 1;
}
