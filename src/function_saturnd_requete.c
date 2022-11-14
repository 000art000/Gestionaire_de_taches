#include "function_saturnd_requete.h"
#include "function_saturnd_cmd.h"


//fonction qui liste les tasks et renvoi le resultat a cassini
int list_tasks(char * reply_pipe){
  Reply *reply;
  int fd_out=open(reply_pipe,O_WRONLY);
  if(fd_out==-1)return 1;
  //appel a lister_cmd qui retourne la reponse a envoyer 
  reply=lister_cmd();
  //envoi de la reponse a cassini
  write(fd_out,reply->reply,reply->size);
  close(fd_out);
  return 0;
}

//fonction qui traite la terminaison de saturnd
int terminate(char * reply_pipe){
  void *reply;
  int fd_out=open(reply_pipe,O_WRONLY);
  if(fd_out==-1)return 1;
  reply=malloc(2);
  *((uint16_t*)reply)=be16toh(SERVER_REPLY_OK);
  //envoi de la reponse
  write(fd_out,reply,2);
  close(fd_out);
  return 0;
}

//fonction qui traite la suppression d'une commande
int remove_task(int fd_in,char * reply_pipe){
   void *reply;
   int fd_out=open(reply_pipe,O_WRONLY);
   if(fd_out==-1)return 1;
   reply=malloc(8);
   //lecture de l'id du task qu'on va enlever
   if(read(fd_in,reply,8)!=8)return 1;
   close(fd_in);
   uint64_t id=htobe64(*(uint64_t*)(reply));
   //appel a remove_task qui supprimme  le id s'il existe
   uint16_t ret=remove_cmd(id);
   if(ret=SERVER_REPLY_OK){
      //cas ou il y avait une suppression
      *((uint16_t*)reply)=be16toh(SERVER_REPLY_OK);
      write(fd_out,reply,2);
   }
   else{
      //cas ou le id n'existe pas
      *((uint16_t*)reply)=be16toh(SERVER_REPLY_ERROR);
      *((uint16_t*)(reply+2))=be16toh(SERVER_REPLY_ERROR_NOT_FOUND);
      write(fd_out,reply,4);
   }
   close(fd_out);
   return 0;
}

//fonction qui traite l'option x (times_and_exitcodes)
int GET_TIMES_AND_EXITCODES(int fd_in,char * reply_pipe){
   void *reply;
   int fd_out;
   reply=malloc(8);
   //lecture de id 
   if(read(fd_in,reply,8)!=8)return 1;
   close(fd_in);
   fd_out=open(reply_pipe,O_WRONLY);
   if(fd_out==-1)return 1;
   //appel a get_t_e qui retourne le reply si le id existe sinon null
   Reply *r=get_t_and_e(*((uint64_t*)reply));
   if(r==NULL){
      //cas ou le id n'existe pas 
      *((uint16_t*)reply)=be16toh(SERVER_REPLY_ERROR);
      *((uint16_t*)(reply+2))=be16toh(SERVER_REPLY_ERROR_NOT_FOUND);
      write(fd_out,reply,4);
   }
   else {
      //cas ou le il faut envoyer le reply
      *((uint16_t*)(r->reply))=be16toh(SERVER_REPLY_OK); 
      write(fd_out,r->reply,r->size);
   }
   close(fd_out);
   return 0;
}


//fonction qui traite le std_out d'un id
int GET_STDOUT(int fd_in,char * reply_pipe){
   void *reply;
   int fd_out=open(reply_pipe,O_WRONLY);
   if(fd_out==-1)return 1;
   reply=malloc(8);
   //lecture de id
   if(read(fd_in,reply,8)!=8)return 1;
   close(fd_in);
   //appel a get_information_from_files avec l'argument OUTPUT qui retourne le contenu du fichier output de id s'il existe
   Reply * r=get_information_from_files(*((uint64_t*)reply),OUTPUT);
   if(r==NULL){
      //cas ou le id n'existe pas
      *((uint16_t*)reply)=be16toh(SERVER_REPLY_ERROR);
      *((uint16_t*)(reply+2))=be16toh(SERVER_REPLY_ERROR_NOT_FOUND);
      write(fd_out,reply,4);
   }
   else {
      //cas d'envoie de contenu
      *((uint32_t*)((r->reply)+2))=be32toh((uint32_t)((r->size)-6));
      *((uint16_t*)(r->reply))=be16toh(SERVER_REPLY_OK); 
      write(fd_out,r->reply,r->size);
   }
   close(fd_out);
   return 0;
}


int GET_STDERR(int fd_in,char * reply_pipe){
  void *reply;
   int fd_out=open(reply_pipe,O_WRONLY);
   if(fd_out==-1)return 1;
   reply=malloc(8);
   //lecture de id
   if(read(fd_in,reply,8)!=8)return 1;
   close(fd_in);
   //appel a get_information_from_files avec l'argument ERROR qui retourne le contenu du fichier erreur de id s'il existe
   Reply * r=get_information_from_files(*((uint64_t*)reply),ERROR);
   if(r==NULL){
      //cas ou le id n'existe pas
      *((uint16_t*)reply)=be16toh(SERVER_REPLY_ERROR);
      *((uint16_t*)(reply+2))=be16toh(SERVER_REPLY_ERROR_NOT_FOUND);
      write(fd_out,reply,4);
   }
   else {
      //cas d'envoie de contenu
      *((uint32_t*)((r->reply)+2))=be32toh((uint32_t)((r->size)-6));
      *((uint16_t*)(r->reply))=be16toh(SERVER_REPLY_OK); 
      write(fd_out,r->reply,r->size);
   }
   close(fd_out);
   return 0;
}


//fonction qui traite la creation d'un nouveau task
int create_task(int fd_in,char * reply_pipe){
     char** argv;
     void *reply;
     void *timing=malloc(13);
     reply=malloc(20);
     int argc2;
     uint32_t len_string;
     char *argv2;
     int fd_out=open(reply_pipe,O_WRONLY);
     int taille;
     if(fd_out==-1)return 1;
     //lecture de timing et le argc
     read(fd_in,reply,17);
     *(uint64_t*)timing=*(uint64_t*)reply;
     *(uint32_t*)(timing+8)=*(uint32_t*)(reply+8);
     *(uint8_t*)(timing+12)=*(uint8_t*)(reply+12);
     argc2=(int)(htobe32(*((uint32_t*)(reply+13))));
     argv=malloc(argc2*sizeof(char*));
     taille=17;
     //boucler argc pour extraire les argument du task
     for(int j=0;j<argc2;j++){
        //lecture de taille d'un argument
        if(read(fd_in,&len_string,4)!=4)return 1;
           len_string=htobe32(len_string);
           argv2=malloc((int)len_string+1);
           //lecture de l'argument
           if(read(fd_in,argv2,(int)len_string)!=(int)len_string)return 1;
           argv2[(int)len_string]='\0';
           argv[j]=argv2;
           taille+=4+len_string;   
     }
     close(fd_in);
     //appel add_cmd qui ajoute le task et retourne son id 
     uint64_t id=add_cmd(timing,argv,argc2,taille);
     //envoie de la reponse a cassini
     *((uint16_t*)reply)=be16toh(SERVER_REPLY_OK);
     *((uint64_t*)(reply+2))=be64toh(id);
     write(fd_out,reply,10);
     close(fd_out);
     //envoie d'un signal a son pere pour l'informer de l'ajout d'un nouveau task (pour mettre a jour le procahin task a executer)
     kill(getppid(),SIGUSR1);
     return 0;
}


