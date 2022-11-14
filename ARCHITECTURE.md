
A-gestion des processus du saturnd :
  on a au debut un processus pere (P1) qui creer les structures necessaires s'ils n'existe pas 
  (pipes,repertoires,fichier),puis il creer un fils (P2) de sorte que (P1) meurt (pour avoir 
  l'execution en arriere plan) et (P2) qui lui meme cree un fils (P3) où : (P2) traite l'execution
   des commandes et (P3) traite la reception des requetes.
  
  -pour l'execution des commandes: pour chaque execution (P2) cree un fils (P4) qui cree (P5), 
     puis P4 meurt pour que (P5) soit adopté par init, ensuite (P5) cree un fils (P6) qui servira
     pour un recouvrement de processus (executer la tache) apres une redirection de la sortie standard
     vers output et une redirection de la sortie d'erreur vers le fichier erreur du task,ensuite (P5) 
     attend (P6) pour recuperer l'exitcode de (P6) et on rajoute une ligne d'exuction dans le fichier
     "time and exitcodes".
     
     
                                                    P1 (P1 meurt)
                                                    |
        (processus qui gere l'execution des tasks)  P2____________P4 (P4 meurt)
                                                    |      ↑        |
                 (processus qui gere les requetes)  P3     ↑        P5__P6
                                                           ↑←←←←←←←←←←←←←←←←(pour chaque execution d'un task)
                                                           
                                          __schema des processus dans le saturnd__
     

B-stockage des fichiers:
   on a tous stocker dans un repertoire (D_CMD) qu'on cree au debut si il n'existe pas où on trouve:
      A)les pipes.
      B)le fichier (cmds) où on stocke tous les tasks.
      C)les repertoires de chaque task (task_"id") ou on trouve aussi 3 fichiers output, erreur, 
       times and exitcodes.
      
1-getsion du fichier cmds:
   cmds est un fichier binaire qui contient au debut le id du prochain task a ajouter, 
   puis on trouve pour chaque task les champs suivants:
     id :uint64_t
     taille :uint32_t(taille de timing +la commande)
     timing :uint64_t + uint32_t + uint8_t
     commandeline : (argc:uint32_t) +les argvs de type string (langeur+argv)
 1-1 suppression d'un task:
   dans le cas ou le id existe on remplace le id du task par une valeur "not_id" pour indiquer qu'il ne sagit 
   pas d'un task et on supprimme le repertoire du task (apres suppression des fichiers qu'il contient). 
     
 1-2 Ajout d'un task:
   on recupere le id qui se retrouve au debut du fichier puis on recherche s'il ya un champs avec "not_id", 
   si on le trouve on compare la taille du task avec la taille existante , si il ya de la place on le rajoute 
   a cet endroit (sans changer l'ancienne taille) sinon on continue à parcourire, si on arrive a la fin on le 
   rajoute directement.
 
2-gestion des fichiers d'un task:
   1-1 pour output et erreur: à chaque execution on ecrase le contenu avec O_TRUNC pour mettre les informations
      de la derniere éxecution
   1-2 pour times and exitcodes on a le nombre d'executions au debut ,on l'incremente et on rajoute la nouvelle 
      ligne d'execution a la fin.
   
C-structure de donnés et algorithmes:
   on a utilisé une structure de donné qui contient deux champs reply qui est un pointeur qui contient une reponse 
     et le champs taille qui determine la taille de la reponse.
      
