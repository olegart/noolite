#ifndef NOOLITERX_H_   /* Include guard */
#define NOOLITERX_H_

#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#define DEV_VID 0x16c0 //0x5824
#define DEV_PID 0x05dc //0x1500
#define DEV_CONFIG 1
#define DEV_INTF 0
#define EP_IN 0x81
#define EP_OUT 0x01

void usage(void);
char *str_replace(const char *s, const char *old, const char *new);
char* int_to_str(int num);
void cleanup(int sig);

int do_exit;

#endif