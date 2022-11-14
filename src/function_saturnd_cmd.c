#include "function_saturnd_cmd.h"


//fonction auxiliaire qui cree un fichier name dans path vide si bol <> 1 sinon il ecrit un uint32_t qui vaut 0
int creer_fichier(char *path,char *name,int bol){
    char buff[100];
    strcpy(buff,path);
    strcat(buff,"/");
    strcat(buff,name);
   //creer un fichier
   int fd=open(buff,O_CREAT|O_WRONLY,0666);
   if(fd==-1)return 1;
   //si bol =1 alors ecrire 0 au debut
   if(bol==1){
     uint32_t n=0;
     if(write(fd,&n,4)<4){close(fd); return 1;  }
   }
   close(fd);
   return 0;
} 

//fonction auxiliaire qui supprimme un fichier name dans path
int supp_fichier(char *path,char *name){
    char buff[100];
    strcpy(buff,path);
    strcat(buff,"/");
    strcat(buff,name);
   //suppression avec unlink 
   int r=unlink(buff);
   if(r==-1)return -1;
   return 0;
} 

//fonction auxiliaire qui ouvre un fichier name dans le id est specifié en ecriture(avec ecrasement de contenu) et renvoi le fd 
int open_file(uint64_t id,char *name){
    char buff[100];
    sprintf(buff,"%s/Task_%ld/%s",DIRECTORY_FILES_CMD,id,name);
    return open(buff,O_WRONLY|O_TRUNC);
}

//fonction qui ouvre un fichier name dont le id est specifié et renvoie son contenu et sa taille dans la structure Reply
Reply* get_information_from_files(uint64_t id,char *name){
   int size=6,taille=510,nb_read;
   char path[100];
   Reply *r;
   char *buff=malloc(taille),*p,*tmp;
   sprintf(path,"%s/Task_%ld/%s",DIRECTORY_FILES_CMD,htobe64(id),name);
   //ouverture du fichier specifié dans path
   int fd=open(path,O_RDONLY);
   if(fd==-1)return NULL;
   r=malloc(sizeof(Reply));
   //laisser l'espace pour le debut de la reponse (eventuelement ok +taille de la reponse)
   p=buff+6;
   memset(buff,'c',6);
   //boucle qui termine a la fin du fichier
   while(1){
      //lecture de 500 octet
      nb_read=read(fd,p,500);
      if(nb_read==0)break;
      size+=nb_read;
      //si le buffer ne suffit pas on double sa taille
      if(size+500>=taille){
         tmp=malloc(taille*2);
         taille*=2;
         buff[size]='\0';
         strcpy(tmp,buff);
         free(buff);
         buff=tmp;
      }
      p=buff+size;
   }
   //envoie du contenu +la taille via la structure reply
   r->reply=buff;
   r->size=size;
   return r;
}


//fonction qui rajoute une commande
uint64_t add_cmd(void *timing,char ** argv,int argc,int taille){
    void * entete=malloc(12);
    void * buff;
    uint64_t not_id,id;
    int fd_in_out=open(FILE_CMD,O_RDWR),wr=0;
    char directory_of_task[100];
    char output[100];
    char erreur[100];
    char te[100];
    
    //allouer buff avec la taille de la commande avec le id et la taille et le timing compris
    buff=malloc(taille+12);
    memset(&not_id,0xff,8);
    //lecture du id du prochain task qui se trouve au debut du fichier ou on stock les tasks
    if(read(fd_in_out,&id,8)<8)return not_id;
    sprintf(directory_of_task,"%s/Task_%ld",DIRECTORY_FILES_CMD,id);
    //creation du repertoire task_id du nouveau task
    if(mkdir(directory_of_task,0777)==-1)return not_id;
    //creation des 3 fichier output error et times_and_exitcodes dans le repertoire du nouveau task
    if(creer_fichier(directory_of_task,OUTPUT,0)==1)return not_id;
    if(creer_fichier(directory_of_task,ERROR,0)==1)return not_id;
    if(creer_fichier(directory_of_task,T_E,1)==1)return not_id;
    
    //ajouter le id et le timinget le argc
    *(uint64_t*)buff=id;
    *(uint64_t*)(buff+12)=*(uint64_t*)timing;
    *(uint32_t*)(buff+20)=*(uint32_t*)(timing+8);
    *(uint8_t*)(buff+24)=*(uint8_t*)(timing+12);
    *(int*)(buff+25)=be32toh(argc);
    int cmp=0;
    //rajouter les arguments de la commande
    for(int i=0;i<argc;i++){
       //rajouter la taille du argv
       *(uint32_t*)(buff+29+cmp)=be32toh((uint32_t)strlen(argv[i]));
       cmp+=4;
       //rajouter argv
       strcpy(buff+29+cmp,argv[i]);
       cmp+=strlen(argv[i]);
    }
    //recherche d'une place vide pour ajouter le task
    while(read(fd_in_out,entete,12)==12){
       //si il existe une place vide il faut verifier que la taille est suffisante
       if(not_id==*(uint64_t*)entete){
          if(taille<=*(uint32_t*)(entete+8)){
             //retouner au debut du task
             lseek(fd_in_out,-12,SEEK_CUR);
             //mettre la taille max
             *(uint32_t*)(buff+8)=*(uint32_t*)(entete+8);
             //ajouter le task au fichier
             if(write(fd_in_out,buff,taille+12)<taille+12)return not_id;
             wr=1;
             break;
          }
       }
       //avancer le curseur vers le prochain task
       lseek(fd_in_out,*(uint32_t*)(entete+8),SEEK_CUR);
    }
    //si y'avait pas de place rajouter le task a la fin
    if(!wr){
       //rajouter la taille au buffer
       *(uint32_t*)(buff+8)=(uint32_t)taille;
       //ecrire le contenu
       if(write(fd_in_out,buff,taille+12)<taille+12)return not_id;
    }
    //retourner au debut de fichier et incrementer le compteur des id 
    lseek(fd_in_out,0,SEEK_SET);
    id++;
    write(fd_in_out,&id,8);
    id--;
    return id;
}

//fonction auxiliaire qui enleve les fichiers et le repertoires d'un id 
int remove_task_files(uint64_t id){
   char directory_of_task[100];

   sprintf(directory_of_task,"%s/Task_%ld",DIRECTORY_FILES_CMD,id);
   if(supp_fichier(directory_of_task,OUTPUT)==-1)return -1;
   if(supp_fichier(directory_of_task,ERROR)==-1)return -1;
   if(supp_fichier(directory_of_task,T_E)==-1)return -1;
   return rmdir(directory_of_task);
} 

//la fonction qui gere la suppression d'un task dont le id est specifié si il existe
uint16_t  remove_cmd(uint64_t id){
  uint64_t tmp,not_id;
  uint32_t taille;
  void *entete=malloc(12);
  int fd_in_out=open(FILE_CMD,O_RDWR);

  //lecture du compteur des ids
  if(read(fd_in_out,&tmp,8)<8)return SERVER_REPLY_ERROR;
  memset(&not_id,0xff,8);

  //parcourire le fichier pour rechercher le task du id donné en parametre
  while(read(fd_in_out,entete,12)==12){
     tmp=*(uint64_t*)entete;
     taille=*(uint32_t*)(entete+8);
     //si le task existe remplacer le id par not_id et supprimer le repertoire du task
     if(tmp==id){
        lseek(fd_in_out,-12,SEEK_CUR);
        write(fd_in_out,&not_id,8);
        if(remove_task_files(id)==-1)return SERVER_REPLY_ERROR; 
        return SERVER_REPLY_OK;
     }    
     //sinon avancer vers le prochain task
     lseek(fd_in_out,taille,SEEK_CUR);
  }
  
  return SERVER_REPLY_ERROR;  
}

//fonction qui liste le contenu du fichier des tasks
Reply* lister_cmd(){
   int size=500;
   char *reply=malloc(size),*tmp,*argv2,*entete=malloc(12);
   int t=6,nb_cmds=0,argc2;
   uint32_t taille,len_string;
   uint64_t id;
   uint64_t not_id;
   memset(&not_id,0xff,8);
   int fd_in=open(FILE_CMD,O_RDONLY);
   //mettre OK au debut du reply
   *((uint16_t*)reply)=be16toh(SERVER_REPLY_OK);
   //lire le compteur des id
   if(read(fd_in,&id,8)<8)return NULL;
   //boucler tant que il ya un task
   while(read(fd_in,entete,12)==12){
     id=*(uint64_t*)entete;
     //verifier qu'il sagit d'un task sinon avnacer vers le prochain entête
     if(id==not_id){
         lseek(fd_in,*(uint32_t*)(entete+8),SEEK_CUR);
         continue;
     }
     //rajouter le id
     *(uint64_t*)(reply+t)=be64toh(id);
     t+=8;
     taille=*(uint32_t*)(entete+8);
     //si la taille du buffer n'est pas suffisante doubler sa taille
     if(taille+t+12>size){
        reply[t]='\0';
        size*=2;
        tmp=malloc(size);
        strcpy(tmp,reply);
        free(reply);
        reply=tmp;
     }
     tmp=malloc(17);
     //lecture du timing et le argc
     read(fd_in,tmp,17);
     taille-=17;
     *(uint64_t*)(reply+t)=*((uint64_t*)(tmp));
     *(uint32_t*)(reply+t+8)=*((uint32_t*)(tmp+8));
     *(uint8_t*)(reply+t+12)=*((uint8_t*)(tmp+12));
     t+=13;
     argc2=(int)(htobe32(*((uint32_t*)(tmp+13))));
     *(uint32_t*)(reply+t)=*((uint32_t*)(tmp+13));
     t+=4;
     nb_cmds++;
     //boucler pour lire les arguments du task
     for(int j=0;j<argc2;j++){
        //lire la taille d'un argument
        if(read(fd_in,&len_string,4)!=4)return NULL;
           taille-=4;
           *(uint32_t*)(reply+t)=len_string;
           t+=4;
           len_string=htobe32(len_string);
           argv2=malloc((int)len_string+1);
           //lecture de l'argument
           if(read(fd_in,argv2,(int)len_string)!=(int)len_string)return NULL;
           taille-=len_string;
           argv2[(int)len_string]='\0';
           strcpy(reply+t,argv2);
           t+=len_string;
     }
     //avancer vers le prochain task
     lseek(fd_in,taille,SEEK_CUR);
   }
   *(uint32_t*)(reply+2)=be32toh((uint32_t)nb_cmds);
   Reply *r=malloc(sizeof(Reply));
   //retourner le resultat et sa taille via la structure reply
   r->reply=reply;
   r->size=t;
   return r;
}

//fonction qui retourne le contenu du fichier de time_and_exitcodes
Reply * get_t_and_e(uint64_t id){
   int size=6,nb_read;
   char path[100];
   Reply *r;
   char *buff;
   sprintf(path,"%s/Task_%ld/%s",DIRECTORY_FILES_CMD,htobe64(id),T_E);
   //ouverture du fichier
   int fd=open(path,O_RDONLY);
   if(fd==-1)return NULL;
   r=malloc(sizeof(Reply));
   uint32_t nb;
   //lire le nombre des executions
   if(read(fd,&nb,4)<4)return NULL;
   //allouer la bonne taille
   buff=malloc(nb*10+6);
   *(uint32_t*)(buff+2)=be32toh(nb);
   //lire le contenu du fichier
   if(read(fd,buff+size,nb*10)<nb*10)return NULL;
   size+=nb*10;
   //retourner le resultat et sa taille via la structure reply
   r->reply=buff;
   r->size=size;
   return r;
}



