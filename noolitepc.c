/*   Consloe Utility for nooLite PC11xx
     (c) Mikhail Ermolenko
*/

#define LINUX

#ifdef WINDOWS
#include "libusb.h"
#endif
#ifdef LINUX
#include <libusb-1.0/libusb.h>
#endif

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
    
     if (argc <= 2) {
          printf("Using %s -api -<command> <channel> [<level>|<RGB>]\n", argv[0]);
          printf("     <command> may be:\n");
          printf("          --on - Turn channel ON\n");
          printf("          --off - Turn channel OFF\n");
          printf("          --sw - Switch channel ON/OFF\n");
          printf("          --set - Set level for channel\n");
          printf("          --bind - Bind channel\n");
          printf("          --unbind - Unbind channel\n");
          printf("          --load_preset - Load preset channel \n");
          printf("          --save_preset - Save preset channel \n");
          printf("          --stop_reg - Stop change channel \n");
          printf("          --roll_color - Rolling color\n");
          printf("          --sw_color - Swith color\n");
          printf("          --sw_mode - Swith mode\n");
          printf("          --speed_mode_sw - Swith speed change color\n");
          printf("          --set_color - Set color R[0..255] G[0..255] B[0..255] \n");
          printf("     <channel> must be [1..32]\n");
          printf("     <level> must be [0..100] - using for -set\n");
          printf("     <RGB> must be [0..255] [0..255] [0..255] - using for - set_color \n");
          return -1;
     }
    
     if (argc >= 3) {
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
               COMMAND_ACTION[2]= 1; // формат
               if (argc >= 5) {
                 level     = atoi(argv[3]);
               } else {
                 printf("Не указан уровень \nИспользование: %s --<command> <channel> [<level>]\n", argv[0]);
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
          else if (strcmp(argv[1],"--load_preset")==0) //Команда вызова записанного сценария из памяти силового блока для канала X
          {
               COMMAND_ACTION[1] = 7;
          } 
          else if (strcmp(argv[1],"--save_preset")==0) //Команда записи сценария в память силового блока для канала X
          {
               COMMAND_ACTION[1] = 8;
          } 
          else if (strcmp(argv[1],"--stop_reg")==0) //остановить регулировку 
          {
               COMMAND_ACTION[1] = 10;
          } 
          else if (strcmp(argv[1],"--roll_color")==0) //включение плавного перебора цвета, выключается командой 10.
          {
               COMMAND_ACTION[1] = 16;
               COMMAND_ACTION[2] = 4; // формат
          } 
          else if (strcmp(argv[1],"--sw_color")==0) //переключение цвета 
          {
               COMMAND_ACTION[1] = 17;
               COMMAND_ACTION[2] = 4; // формат
          } 
          else if (strcmp(argv[1],"--sw_mode")==0) //переключение режима работы 
          {
               COMMAND_ACTION[1] = 18;
               COMMAND_ACTION[2] = 4; // формат
          } 
          else if (strcmp(argv[1],"--speed_mode_sw")==0) //переключение скорости эффекта в режиме работы
          {
               COMMAND_ACTION[1] = 19;
               COMMAND_ACTION[2] = 4; // формат
          } 
          else if (strcmp(argv[1],"--set_color")==0) //Установка яркости на каждый канал независимо (R - 1, G - 2, B - 3). Уровень передается параметрами в формате 0…255
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
     } else {
          printf("Не указана команда\nИспользование: %s --<command> <channel> [<level>]\n", argv[0]);
          return -1;
     }

     if (argc >= 4) {
          channel     = atoi(argv[2]);
          channel--;
          if ((channel>31)||(channel<0)) {
               printf("Неверно указан канал (1-32)\nИспользование: %s --<command> <channel> [<level>]\n", argv[0]);
               return -1;
          }
          COMMAND_ACTION[4] = channel;
     } else {
          printf("Не указан канал\nИспользование: %s --<command> <channel> [<level>]\n", argv[0]);
          return -1;
     }

//Prepare Command string
     libusb_init(NULL);
     libusb_set_debug(NULL, 3);
     handle = libusb_open_device_with_vid_pid(NULL, DEV_VID, DEV_PID);
     if (handle == NULL) {
         printf("Не удалось найти устройство\n");
         libusb_exit(NULL);
         return 0;
     }
     if (libusb_kernel_driver_active(handle,DEV_INTF))
          libusb_detach_kernel_driver(handle, DEV_INTF);
     if ((ret = libusb_set_configuration(handle, DEV_CONFIG)) < 0)
     {
          printf("Ошибка конфигурации\n");
          libusb_close(handle);
          libusb_exit(NULL);
          if (ret == LIBUSB_ERROR_BUSY)
              printf("B\n");
          printf("ret:%i\n", ret);   
          return 0;
     }
     if (libusb_claim_interface(handle,  DEV_INTF) < 0)
     {
          printf("Ошибка интерфейса\n");
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
