#include "function_saturnd.h"
#include "function_saturnd_cmd.h"
#include "function_saturnd_requete.h"
int min=0x7fffffff;

//fonction qui gere les requetes
void gerer_requete(){
  int fd_in,fd_out,argc2;
  uint16_t op;
  uint32_t len_string;
  void *reply;
  char *argv2;
  int termine=1;
    
  while(termine){
     fd_in=open(SATURND_REQUEST_PIPE,O_RDONLY);
     if(fd_in==-1)exit(1);
        read(fd_in,&op,2);
     switch(htobe16(op)){
        case CLIENT_REQUEST_LIST_TASKS:
            close(fd_in);
            list_tasks(SATURND_REPLY_PIPE);
            break;
        case CLIENT_REQUEST_CREATE_TASK:
            create_task(fd_in,SATURND_REPLY_PIPE);
            break;
        case CLIENT_REQUEST_REMOVE_TASK:
            remove_task(fd_in,SATURND_REPLY_PIPE);
            break;
        case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES:
            GET_TIMES_AND_EXITCODES(fd_in,SATURND_REPLY_PIPE);
            break;
   	case CLIENT_REQUEST_TERMINATE:
   	    close(fd_in);
   	    terminate(SATURND_REPLY_PIPE);
   	    kill(getppid(),9);
   	    termine=0;
            break;
      	case CLIENT_REQUEST_GET_STDOUT:
      	    GET_STDOUT(fd_in,SATURND_REPLY_PIPE);
            break;
  	case CLIENT_REQUEST_GET_STDERR:
  	    GET_STDERR(fd_in,SATURND_REPLY_PIPE);
            break;
        default:
            printf("commande inconnu");
            break;
      }
    }
}

int execute_cmd(uint64_t id,void *tmp,int fd,uint32_t taille,time_t t){
   int pid = fork();
   //creer un fils pour executer la tache
   if ( pid == 0 )
   { 
      //ouvrir les fichier d'erreur et sortie de la task
      //avec la fonction open_file
      int fd_out=open_file(id,OUTPUT);
      if(fd_out==-1)return 1;
      int fd_err=open_file(id,ERROR);
      if(fd_err==-1)return 1;
      //redirection de la sortie standart et la sorite d'erreur 
      //sur le fichier d'erreur et le fichier out de la task
      dup2(fd_out,1);
      dup2(fd_err,2);
      //liberer les fds inutiles
      close(fd_out);
      close(fd_err);
              
      int argc2=(int)(htobe32(*((uint32_t*)(tmp+13))));
     //creer un pointeur argv de taille nombre de argc
      char **argv=malloc(argc2*sizeof(char*));
      uint32_t len_string;
      taille-=17;
    //boucler argc fois et 
    //mettre pour chaque case i (argv[i]) le string de la tache 
    //corespondant          
      for(int j=0;j<argc2;j++){
         if(read(fd,&len_string,4)!=4)return 1;
         len_string=htobe32(len_string);
         taille-=4;
         argv[j]=malloc((int)len_string+1);
         if(read(fd,argv[j],(int)len_string)!=(int)len_string)return 1;
         argv[j][(int)len_string]='\0';
         taille-=len_string;   
      }
      //recouverement pour executer la tache
      execvp(argv[0], argv);
      //retourner le code d'erreur si le recouverement echoue
      exit(0xFFFF);
   }
   //attendre le fils et stocke ces information
   int status;
   waitpid(pid, &status, 0);
 
   if ( WIFEXITED(status) ){
      uint16_t exit_status = WEXITSTATUS(status); 
      //mettre le chemin du fichier execute code de la tache dans buff 
      char buff[100];
      sprintf(buff,"%s/Task_%ld/%s",DIRECTORY_FILES_CMD,id,T_E);
      int fd_exit=open(buff,O_RDWR);  
     //lire combien il existe de ligne 
      if(read(fd_exit,buff,4)<4)return 1;
      uint32_t nb_ex=*(uint32_t*)buff;
      lseek(fd_exit,0,SEEK_SET);
      nb_ex++;
      //incrementer et le re ecrire
      if(write(fd_exit,&nb_ex,4)<4)return 1;
      //deplacement a la fin du fichier et ecrire la nouvelle ligne
      lseek(fd_exit,0,SEEK_END);
      *(uint64_t*)buff=be64toh((uint64_t)t);
      *(uint16_t*)(buff+8)=be16toh(exit_status);
      if(write(fd_exit,buff,10)<10)return 1;
      return 0;
   }
   return 1;
}

//fonction qui retourne le nombre de secondes restante pour la prochaine execution
int get_min_time(uint64_t minute,uint32_t heure,uint8_t day,uint64_t m,uint32_t h,uint8_t d){
   int tmp=0,tmp2=0,tmp3=1;
   //tester si il existe au moins une execution possible
   if(minute!=0 && heure!=0 && day!=0){
      //calculer le nombre de minute
      m=m<<1;
      while(tmp2==0){
         if((m&0x0fffffffffffffff)==0){
            m=1;
            h=h<<1;
         }
         if((m&minute)!=0){tmp2+=tmp3;break;}
         tmp3++;
         m=m<<1;
      }
      //convertir les minutes en secondes
      tmp+=tmp2*60; 
      tmp2=tmp3=0;
      //calculer le nombre des heures restantes
      while(tmp2==0){
         if((h&0x00ffffff)==0){
            h=1;
            d=d<<1;
         }
         if((h&heure)!=0){tmp2+=tmp3;break;}
         tmp3++;
         h=h<<1;
      }
      //traduire les heures en secondes
      tmp+=tmp2*3600;
      tmp2=tmp3=0;
      //calculer le nombre de jours restants
      while(tmp2==0){
         if((d&0x7f)==0){
            d=1;
         }
         if((d&day)!=0){tmp2+=tmp3;break;}
         tmp3++;
         d=d<<1;
      } 
      //traduire les jours en secondes
      tmp+=tmp2*3600*24;
      return tmp;
   }
   //cas d'erreur
   return 0x7fffffff;
}

void execute(){
   void *entite=malloc(12),*tmp;
   uint64_t not_id,id,minute,m;
   uint32_t heure,h;
   uint8_t day,d;
   int fd_ex;
   time_t t;
   struct tm *time_info;
   
   //mettre dans la variable not_id la valeur qui indique que ce n'est 
   //pas un id (ff dans les octets)
   memset(&not_id,0xff,8);
   int fd=open(FILE_CMD,O_RDONLY);
   if(fd==-1)goto kill_saturnd;
   //sauter dans le fichier où sont stocker les task,le prochain id
   //attribuer
   lseek(fd,8,SEEK_CUR);
   min=0x7fffffff;
   //lire l'entete de chaque tache 
   //lentete est composé de 12 octet task_id 8 octets et la taille de
   //la ligne 4 octets 
      while(read(fd,entite,12)==12){
         id=*(uint64_t*)entite;
         //verifier si c'est une tache
         //si elle ne l'est pas sauter a la prochaine tache
         if(id==not_id){
           lseek(fd,*(uint32_t*)(entite+8),SEEK_CUR);
           continue;
         }
         uint32_t taille=*(uint32_t*)(entite+8);
         tmp=malloc(17);
         //lire le temps et l'argc de la tache
         if(read(fd,tmp,17)<17)goto kill_saturnd;
         minute=htobe64(*(uint64_t*)tmp);
         heure=htobe32(*(uint32_t*)(tmp+8));
         day=*(uint8_t*)(tmp+12);
         t=time(NULL);
         time_info=localtime(&t);
         m=1;
         m=m<<(time_info->tm_min);
         h=1;
         h=h<<(time_info->tm_hour);
         d=1;
         d=d<<(time_info->tm_wday);
         //formater le temps de la tache en seconde
         int val=get_min_time(minute,heure,day,m,h,d);
         //calculer le temps a dormir
         if(val<min)min =val-time_info->tm_sec;
        //si la tache s'execute en ce temps precis
         if(((minute&m)!=0) && ((heure&h)!=0) && ((day&d)!=0)){
            //stocke le pid de saturnd (saturnd initial)
            int p=getpid();
            switch(fork()){ 
              case -1: goto kill_saturnd; 
              case 0 :
              //ouvrir le fichier où sont stocker les task
              //et sauter a ofstet courrant
                fd_ex=open(FILE_CMD,O_RDONLY);
                lseek(fd_ex,lseek(fd,0,SEEK_CUR),SEEK_CUR);
                switch(fork()){ 
                  case -1: 
                    kill(p,SIGUSR2);
                    exit(0); 
                  case 0 :
                    if(execute_cmd(id,tmp,fd_ex,taille,t)!=0)
                       kill(p,SIGUSR2);
                    exit(0);
                    break;
                  default : 
                    exit(0);
                    break;  
               }
              
                break;
                
              default :
                wait(NULL); 
                break;  
            }
         }        
         lseek(fd,taille-17,SEEK_CUR);
      }
      close(fd);
      //si il n'existe pas de task a executer le mettre en pause
      //sinon dormir un temps precis
      if(min==0x7fffffff)
        pause();
      else
        sleep(min);
      return;
     kill_saturnd :
     kill(0,9);
     exit(1);
}

void handler_ajout(int sig){
   void *entite=malloc(12);
   char tmp[18];
   uint64_t not_id,id,minute,m;
   uint64_t last_id;
   uint32_t heure,h;
   uint32_t taille;
   uint8_t day,d;
   int fd_ex;
   time_t t;
   struct tm *time_info;
   
   //mettre dans la variable not_id la valeur qui indique que ce n'est 
   //pas un id (ff dans les octets)
   memset(&not_id,0xff,8);
   int fd=open(FILE_CMD,O_RDONLY);
   if(fd==-1)goto kill_saturnd;
   //lire dans le fichier où sont stocker les task,le prochain id
   //attribuer et le desincrementer pour avoir le dernier id attribuer
   if(read(fd,&last_id,8)<8)goto kill_saturnd;
   last_id--;
   
   min=0x7fffffff;
   //lire l'entete de chaque tache 
   //lentete est composé de 12 octet task_id 8 octets et la taille de
   //la ligne 4 octets 
      while(read(fd,entite,12)==12){
         id=*(uint64_t*)entite;
         //verifier si c'est une tache
         //si elle ne l'est pas sauter a la prochaine tache
         if(id==not_id){
           lseek(fd,*(uint32_t*)(entite+8),SEEK_CUR);
           continue;
         }
         
         taille=*(uint32_t*)(entite+8);
         //lire le temps et l'argc de la tache
         if(read(fd,tmp,17)<17)goto kill_saturnd;
         minute=htobe64(*(uint64_t*)tmp);
         heure=htobe32(*(uint32_t*)(tmp+8));
         day=*(uint8_t*)(tmp+12);
         t=time(NULL);
         time_info=localtime(&t);
         m=1;
         m=m<<(time_info->tm_min);
         h=1;
         h=h<<(time_info->tm_hour);
         d=1;
         d=d<<(time_info->tm_wday);
         //formater le temps de la tache en seconde
         int val=get_min_time(minute,heure,day,m,h,d);
         //recalculer le temps a dormir 
         if(val<min)min =val-time_info->tm_sec;
         lseek(fd,taille-17,SEEK_CUR);
      }
      close(fd);
      if(min==0x7fffffff)
         pause();
      else
      sleep(min);
      return;
     kill_saturnd :
     kill(0,9);
     exit(1);
}

//handler pour tuer le saturnd, il envoie le signal a tous le groupe
void kill_saturnd(int sig){
   kill(0,9);
}

//fonction qui lance la boucle infinie pour executer les commandes
void execute_cmds(int pid){
   struct sigaction action,action_kill;
   //specifier le comportement pour le signal SIGUSR1
   memset(&action,0,sizeof(sigaction));
   action.sa_handler=handler_ajout;
   sigaction(SIGUSR1,&action,NULL);
   //specifier le comportement pour le signal SIGUSR2
   memset(&action_kill,0,sizeof(sigaction));
   action_kill.sa_handler=kill_saturnd;
   sigaction(SIGUSR2,&action_kill,NULL);
   /*calculer le temps min dans handler_ajout et dormir jusqu'a la 
   prochaine execution */
   handler_ajout(0);
   while(1){
      execute();
   }
    
}
