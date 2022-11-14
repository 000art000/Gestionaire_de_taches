#ifndef FUNCTIONS_SATURND_REQUETE_H
#define FUNCTIONS_SATURND_REQUETE_H

#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <wait.h>
#include <endian.h>
#include <time.h>

#include "timing.h"
#include "client-request.h"
#include "server-reply.h"
#include "timing-text-io.h"

int list_tasks(char * reply_pipe);

int terminate(char * reply_pipe);

int remove_task(int fd_in,char * reply_pipe);

int GET_TIMES_AND_EXITCODES(int fd_in,char * reply_pipe);

int GET_STDOUT(int fd_in,char * reply_pipe);

int GET_STDERR(int fd_in,char * reply_pipe);

int create_task(int fd_in,char * reply_pipe);

#endif
