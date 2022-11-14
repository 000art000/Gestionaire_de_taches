#ifndef FUNCTIONS_SATURND_CMD_H
#define FUNCTIONS_SATURND_CMD_H

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


typedef struct{
   void* reply;
   int size;
}Reply;

Reply * get_t_and_e(uint64_t id);

int creer_fichier(char *path,char *name,int bol);

int supp_fichier(char *path,char *name);

int open_file(uint64_t id,char *name);

Reply* get_information_from_files(uint64_t id,char *name);

Reply* lister_cmd();

uint64_t add_cmd(void *timing,char ** argv,int argc,int taille);

uint16_t  remove_cmd(uint64_t id);

#endif
