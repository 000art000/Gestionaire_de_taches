#include "function_cassini.h"

//fonction qui liste les tasks
int list_tasks(int fd_wr,char * reply){
  char rep[3];
  uint32_t nb_tasks,len_string;
  uint64_t task_id;
  uint16_t ls=be16toh(CLIENT_REQUEST_LIST_TASKS);
  struct timing t;
  char buff[26];
  char dest[TIMING_TEXT_MIN_BUFFERSIZE];
  char *argv;
  int argc;
  int fd_rd;
  //envoyé le code de ls
  if(write(fd_wr,&ls,2)<2)goto error;
  close(fd_wr);
  //lire reponse
  if((fd_rd=open(reply,O_RDONLY))==-1)goto error;
  if(read(fd_rd,rep,2)!=2)goto error;
  
  if(htobe16(*((uint16_t*)rep))==SERVER_REPLY_OK){
  //cas du ok
      if(read(fd_rd,&nb_tasks,4)!=4)goto error;
      //boucler nombre_de_task fois
      for(int i=0;i<(int)(htobe32(nb_tasks));i++){
          //lire 25 octet de task_id jusqua argc
          if(read(fd_rd,buff,25)!=25)goto error;
          printf("%ld: ",htobe64(*((uint64_t*)buff)));
          t.minutes=htobe64(*((uint64_t*)(buff+8)));
          t.hours=htobe32(*((uint32_t*)(buff+16)));
          t.daysofweek=*((uint8_t*)(buff+20));
          timing_string_from_timing(dest,&t);
          printf("%s ",dest); 
          argc=(int)(htobe32(*((uint32_t*)(buff+21))));
          //boulcer argc fois pour lire les string de task
          for(int j=0;j<argc;j++){
              //lire la taille du string
              if(read(fd_rd,&len_string,4)!=4)goto error;
              len_string=htobe32(len_string);
              argv=malloc((int)len_string+1);
              //lire le string de task 
              if(read(fd_rd,argv,(int)len_string)!=(int)len_string)
              goto error;
              argv[(int)len_string]='\0';
              printf("%s ",argv);
              free(argv);
          }
        printf("\n");
      }
  }
  else  goto error;
  close(fd_rd);
  return 0;
  error:
    close(fd_rd);
    close(fd_wr);
    return 1;
}

int terminate(int fd_wr,char *reply){
  uint16_t buff=be16toh(CLIENT_REQUEST_TERMINATE);
  int fd_rd;
  //envoyer le code TR
  if(write(fd_wr,&buff,sizeof(uint16_t))<sizeof(uint16_t)) goto error;
  close(fd_wr);
  //lire la reponse
  if((fd_rd=open(reply,O_RDONLY))==-1) goto error;
  if(read(fd_rd,&buff,2)<2) goto error;
  //verifier la reponse du serveur
  if(htobe16(buff) == SERVER_REPLY_OK){}
  else goto error;
  close(fd_rd);
  return 0;
  error:
    close(fd_rd);
    close(fd_wr);
    return 1;
}

int GET_TIMES_AND_EXITCODES(int fd_wr,char *reply,uint64_t taskid){
  void * buff=malloc(10),*buff2=malloc(6);
  uint16_t rep;int fd_rd;

  *((uint16_t*) buff)= be16toh(CLIENT_REQUEST_GET_TIMES_AND_EXITCODES);
  *((uint64_t*) (buff+2))= be64toh(taskid);
  //envoyer le code de la command x avec le task id
  if(write (fd_wr,buff,10)<10) goto error;
  close(fd_wr);
  //lire la reponse et eventualemennt le nombre de d'execution
  if((fd_rd=open(reply,O_RDONLY))==-1)goto error;
  if(read(fd_rd,buff2,6)<4)goto error;
  rep=*((uint16_t*)buff2);
  if(htobe16(rep)==SERVER_REPLY_OK){
  //cas du ok
    uint32_t i=htobe32(*((uint32_t*)(buff2+2)));
    uint64_t time;
    int h;
    for(int j=0;j<i;j++){
    //lire 10 octets du run time et execute code
      if(read(fd_rd,buff,10)<10)goto error;
      time = htobe64(*(uint64_t*)(buff));
      rep = htobe16(*(uint16_t*)(buff+8));
      h=rep;
      struct tm *t= localtime(&time);
      char buf[21];
      //afficher une ligne
      strftime(buf,500,"%Y-%m-%d %H:%M:%S",t);
      printf("%s %d\n",buf,h);
    }
  }else if(htobe16(rep)==SERVER_REPLY_ERROR){
  //cas d'erreur
     rep=*((uint16_t*)(buff2+2));
     if(htobe16(rep)!=SERVER_REPLY_ERROR_NOT_FOUND) goto error;
     goto error;
  }else goto error;
  close(fd_rd);
  return 0;
  error:
    close(fd_rd);
   // close(fd_wr);
    return 1;
}
 
int remove_task(int fd_wr,char *reply,uint64_t taskid){
    void * buff=malloc(10),*buff2=malloc(4);;
    int fd_rd;
    //envoyer le code de la command r avec le task id
    *((uint16_t*) buff)= be16toh(CLIENT_REQUEST_REMOVE_TASK);
    *((uint64_t*) (buff+2))= be64toh(taskid);
    if(write (fd_wr,buff,10)<10) goto error;
    close(fd_wr);
    //lire la reponse , eventualement avec le ocde d'erreur
    if((fd_rd=open(reply,O_RDONLY))==-1) goto error;
    if(read(fd_rd,buff2,4)<2) goto error;
    uint16_t rep=*((uint16_t*)buff2);
    
    if(htobe16(rep)==SERVER_REPLY_OK){
    //cas du ok
    }else if(htobe16(rep)==SERVER_REPLY_ERROR){
        //cas du erreur
        rep=*((uint16_t*)(buff2+2));
        if(htobe16(rep)!=SERVER_REPLY_ERROR_NOT_FOUND) goto error;
        return 1;
     }else return 1;
     close(fd_rd);
    return 0;
    error:
    close(fd_rd);
    close(fd_wr);
    return 1;
}
int GET_STDOUT(int fd_wr,char *reply,uint64_t taskid){
    void * buff=malloc(10),*buff2=malloc(6);
    int i;
    char *s;
    int fd_rd;

    //envoyer le code de la commande out avec le task id  
    *((uint16_t*) buff)= be16toh(CLIENT_REQUEST_GET_STDOUT);
    *((uint64_t*) (buff+2))= be64toh(taskid);
    if(write (fd_wr,buff,10)<10) goto error;
    close(fd_wr);
  
    if((fd_rd=open(reply,O_RDONLY))==-1) goto error;
    //lire la reponse (4 ou 6 octets : 
                    //code+(taille du string ou error code))
    if(read(fd_rd,buff2,6)<4) goto error;
    uint16_t rep=*((uint16_t*)buff2);
    if(htobe16(rep)==SERVER_REPLY_OK){
    //cas du ok
    //lire le string et l'afficher
       i=(int)htobe32(*((uint32_t*)(buff2+2)));
       s=malloc(i+1);
       if(read(fd_rd,s,i)<i) goto error;
       s[i]='\0';
       printf("%s",s);
     }else if(htobe16(rep)==SERVER_REPLY_ERROR){
     //cas du erreur
        rep=*((uint16_t*)(buff2+2));
           if(htobe16(rep)==SERVER_REPLY_ERROR_NOT_FOUND) goto error;
           else if(htobe16(rep)==SERVER_REPLY_ERROR_NEVER_RUN) goto error;
           else goto error;
     }else goto error;
    close(fd_rd);
    return 0;
    error:
      close(fd_rd);
      //close(fd_wr);
      return 1;
}

int GET_STDERR(int fd_wr,char *reply,uint64_t taskid){
    void * buff=malloc(10),*buff2=malloc(6);
    int i;
    char *s;
    int fd_rd;
   //envoyer le code de la commande out avec le task id 
    *((uint16_t*) buff)= be16toh(CLIENT_REQUEST_GET_STDERR);
    *((uint64_t*) (buff+2))= be64toh(taskid);
    if(write (fd_wr,buff,10)<10) goto error;
    close(fd_wr);
   //lire la reponse (4 ou 6 octets : 
                    //code+(taille du string ou error code))
    if((fd_rd=open(reply,O_RDONLY))==-1)goto error;
    if(read(fd_rd,buff2,6)<4)goto error;
    uint16_t rep=*((uint16_t*)buff2);
    if(htobe16(rep)==SERVER_REPLY_OK){
    //cas du ok
    //lire le string et l'afficher
       i=(int)htobe32(*((uint32_t*)(buff2+2)));
       s=malloc(i+1);
       if(read(fd_rd,s,i)<i) goto error;
       s[i]='\0';
       printf("%s",s);
     }else if(htobe16(rep)==SERVER_REPLY_ERROR){
     //cas du erreur
         rep=*((uint16_t*)(buff2+2));
         //lire et afficher le cas d'erreur
           if(htobe16(rep)==SERVER_REPLY_ERROR_NOT_FOUND) printf("NOT  FOUND\n");
           else if(htobe16(rep)==SERVER_REPLY_ERROR_NEVER_RUN) printf("NEVER RUN\n"); 
           else goto error;
     }else goto error;
    close(fd_rd);
    return 0;
    error:
    close(fd_rd);
    close(fd_wr);
    return 1;
}

int create_task(int fd_wr,char *reply,char* minutes_str,char *hours_str,char* daysofweek_str,int argc,char** argv){
    struct timing dest;
    void *buff;
    int cmp=19;
    int fd_rd;
    //formater le temps d'excution
    if(timing_from_strings(&dest, minutes_str,hours_str,daysofweek_str)!=0)
    return 1;
    //calculer la taille de la commande
    for(int i=0;i<argc;i++){
        cmp+=4+strlen(argv[i]);
    }
    buff=malloc(cmp);
    *(uint16_t*)buff=be16toh(CLIENT_REQUEST_CREATE_TASK);
    *(uint64_t*)(buff+2)=be64toh(dest.minutes);
    *(uint32_t*)(buff+10)=be32toh(dest.hours);
    *(uint8_t*)(buff+14)=dest.daysofweek;
    *(uint32_t*)(buff+15)=be32toh((uint32_t)argc);
    cmp=19;
    //mettre les string de la commande un par un
    for(int i=0;i<argc;i++){
        *(uint32_t*)(buff+cmp)=be32toh((uint32_t)(strlen(argv[i])));
        for(int j=0;j<strlen(argv[i]);j++){
            *(char*)(buff+4+cmp+j)=argv[i][j];
        }
        cmp+=4+strlen(argv[i]);
    }
    //envoyer la commande de creation
    if(write(fd_wr,buff,cmp)<cmp) goto error;
    close(fd_wr);
    if((fd_rd=open(reply,O_RDONLY))==-1) goto error;
    //lire la reponse
    if(read(fd_rd,buff,10)<10) goto error;
    uint16_t rep=*((uint16_t*)buff);
    if(htobe16(rep)==SERVER_REPLY_OK){
       //cas du ok 
       uint64_t i=htobe64(*((uint64_t*)(buff+2)));
       //afficher le numero du task id 
       printf("%ld\n",i);
    }else goto error;
    close(fd_rd);
    return 0;
    error:
    close(fd_rd);
    close(fd_wr);
    return 1;
}


