#ifndef FUNCTIONS_SATURND_H
#define FUNCTIONS_SATURND_H

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

void gerer_requete();

void execute_cmds(int pid);

#endif
