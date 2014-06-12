#ifndef NOOLITEPCD_H_
#define NOOLITEPCD_H_

#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/un.h>

#define DEV_VID 0x16c0 //0x5824
#define DEV_PID 0x05df //0x1503
#define DEV_CONFIG 1
#define DEV_INTF 0

#define PID_NAME "/var/run/noolitepcd.pid"
#define NSOCKET "/tmp/noolitepcd.sock"

void cleanup(int sig);

int do_exit;

#endif