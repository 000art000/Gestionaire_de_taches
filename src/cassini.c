#include "cassini.h"
#include "function_cassini.h"
const char usage_info[] = "\
   usage: cassini [OPTIONS] -l -> list all tasks\n\
      or: cassini [OPTIONS]    -> same\n\
      or: cassini [OPTIONS] -q -> terminate the daemon\n\
      or: cassini [OPTIONS] -c [-m MINUTES] [-H HOURS] [-d DAYSOFWEEK] COMMAND_NAME [ARG_1] ... [ARG_N]\n\
          -> add a new task and print its TASKID\n\
             format & semantics of the \"timing\" fields defined here:\n\
             https://pubs.opengroup.org/onlinepubs/9699919799/utilities/crontab.html\n\
             default value for each field is \"*\"\n\
      or: cassini [OPTIONS] -r TASKID -> remove a task\n\
      or: cassini [OPTIONS] -x TASKID -> get info (time + exit code) on all the past runs of a task\n\
      or: cassini [OPTIONS] -o TASKID -> get the standard output of the last run of a task\n\
      or: cassini [OPTIONS] -e TASKID -> get the standard error\n\
      or: cassini -h -> display this message\n\
\n\
   options:\n\
     -p PIPES_DIR -> look for the pipes in PIPES_DIR (default: /tmp/<USERNAME>/saturnd/pipes)\n\
";

int main(int argc, char * argv[]) {
  errno = 0;
  
  char * minutes_str = "*";
  char * hours_str = "*";
  char * daysofweek_str = "*";
  char * pipes_directory = NULL;
  char * request=NULL;
  char * reply=NULL;
  int fd_in,fd_out,offset=2;
  register struct passwd *pwd;
  register uid_t uid;
  uint16_t operation = CLIENT_REQUEST_LIST_TASKS;
  uint64_t taskid;
  struct stat st_req,st_rep;
  
  int opt;
  char * strtoull_endp;
  while ((opt = getopt(argc, argv, "hlcqm:H:d:p:r:x:o:e:")) != -1) {
    switch (opt) {
    case 'm':
      minutes_str = optarg;
      break;
    case 'H':
      hours_str = optarg;
      break;
    case 'd':
      daysofweek_str = optarg;
      break;
    case 'p':
      pipes_directory = strdup(optarg);
      if (pipes_directory == NULL) goto error;
      break;
    case 'l':
      operation = CLIENT_REQUEST_LIST_TASKS;
      break;
    case 'c':
      operation = CLIENT_REQUEST_CREATE_TASK;
      break;
    case 'q':
      operation = CLIENT_REQUEST_TERMINATE;
      break;
    case 'r':
      operation = CLIENT_REQUEST_REMOVE_TASK;
      taskid = strtoull(optarg, &strtoull_endp, 10);
      if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
      break;
    case 'x':
      operation = CLIENT_REQUEST_GET_TIMES_AND_EXITCODES;
      taskid = strtoull(optarg, &strtoull_endp, 10);
      if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
      break;
    case 'o':
      operation = CLIENT_REQUEST_GET_STDOUT;
      taskid = strtoull(optarg, &strtoull_endp, 10);
      if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
      break;
    case 'e':
      operation = CLIENT_REQUEST_GET_STDERR;
      taskid = strtoull(optarg, &strtoull_endp, 10);
      if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
      break;
    case 'h':
      printf("%s", usage_info);
      return 0;
    case '?':
      fprintf(stderr, "%s", usage_info);
      goto error;
    }
    if(optind<argc){
    if(argv[optind][0]!='-') {offset=optind;break;}}
  }

  //ouverture des pipes
  if(pipes_directory==NULL){
    char rq[]=SATURND_REQUEST_PIPE;
    char rp[]=SATURND_REPLY_PIPE;
    request=malloc(strlen(rq)+1);
    reply=malloc(strlen(rp)+1);
    strcpy(request,rq);
    strcpy(reply,rp);
  }else{
    request=malloc(strlen(pipes_directory)+22);
    reply=malloc(strlen(pipes_directory)+20);
    strcpy(request,pipes_directory);
    strcat(request,"/saturnd-request-pipe");
    strcpy(reply,pipes_directory);
    strcat(reply,"/saturnd-reply-pipe");
  }
  if(lstat(request,&st_req)==-1)return 1;
  if(lstat(reply,&st_rep)==-1)return 1;
  if(st_req.st_mode&S_IFIFO==0)return 1;
  if(st_rep.st_mode&S_IFIFO==0)return 1;  
  fd_out=open(request,O_WRONLY);
  if(fd_out==-1)goto error;
  
  //traitement des operation
  switch(operation){
      case CLIENT_REQUEST_LIST_TASKS:
          if(list_tasks(fd_out,reply)!=0){
              goto error;
          }
          break;
      case CLIENT_REQUEST_CREATE_TASK:
          if(create_task(fd_out,reply,minutes_str,hours_str, 
             daysofweek_str,argc-offset,argv+offset)!=0)
             goto error;
          break;
      case CLIENT_REQUEST_TERMINATE:
          if(terminate(fd_out,reply)!=0){
              goto error;
          }
          break;
      case CLIENT_REQUEST_REMOVE_TASK:
          if(remove_task(fd_out,reply,taskid)!=0){
              goto error;
          }
          break;
      case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES:
          if(GET_TIMES_AND_EXITCODES(fd_out,reply,taskid)!=0){
              goto error;
          }
          break;
      case CLIENT_REQUEST_GET_STDOUT:
          if(GET_STDOUT(fd_out,reply,taskid)!=0){
              goto error;
          }
          break;
      case CLIENT_REQUEST_GET_STDERR:
          if(GET_STDERR(fd_out,reply,taskid)!=0){
              goto error;
          }
          break;
      default:
          fprintf(stderr, "erreur");
          goto error;
          break;
  }
  return EXIT_SUCCESS;

 error:
  if (errno != 0) perror("main");
  free(pipes_directory);
  pipes_directory = NULL;
  return EXIT_FAILURE;
}

