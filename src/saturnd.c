#include "saturnd.h"
#include "function_saturnd.h"

int main(int argc, char** argv){
    int tmp1,tmp2;
    struct stat st;
    //creer le repetoire ou on stock les repertoires des taches 
    //et le fichier qui contient les taches
    if(stat(DIRECTORY_FILES_CMD,&st)==-1){
       mkdir(DIRECTORY_FILES_CMD,0777);
    }
    //creer le fichier des task si il n'existe pas et inialiser
    //le nombre des tasks qu'il contient a 0
    int fd_cmd=open(FILE_CMD,O_CREAT|O_WRONLY|O_EXCL,0666);
    if(fd_cmd!=-1){
       uint64_t zero=0;
       zero=be64toh(zero);
       write(fd_cmd,&zero,8);
       close(fd_cmd);
    }
    //creer les fifo
    mkfifo(SATURND_REQUEST_PIPE,0666);
    mkfifo(SATURND_REPLY_PIPE,0666);
    switch(tmp1=fork()){
      case -1:
        exit(1);
        break;
        
      case 0:
        switch(tmp2=fork()){
          case -1:
            exit(1);
            break;
          case 0:
            gerer_requete();
            break;
          default :
            execute_cmds(tmp2);
            break;  
         } 
        break;
        
      default :
        break;  
    } 
    
    return 0;
}
