#ifndef FUNCTIONS_CASSINI_H
#define FUNCTIONS_CASSINI_H

#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <endian.h>
#include <time.h>

#include "timing.h"
#include "client-request.h"
#include "server-reply.h"
#include "timing-text-io.h"

int list_tasks(int fd_wr,char * reply);

int terminate(int fd_wr,char * reply);

int remove_task(int fd_wr,char * reply,uint64_t taskid);

int GET_TIMES_AND_EXITCODES(int fd_wr,char * reply,uint64_t taskid);

int GET_STDOUT(int fd_wr,char * reply,uint64_t taskid);

int GET_STDERR(int fd_wr,char * reply,uint64_t taskid);

int create_task(int fd_out,char * reply,char* minutes_str,char *hours_str,char* daysofweek_str,int argc,char** argv);

#endif
