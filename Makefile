  CC=gcc
  INCLUDE=include
  CFLAGS+= -I$(INCLUDE)
  
  all : cassini saturnd
  
  cassini : cassini.o function_cassini.o timing-text-io.o
	$(CC) -o cassini cassini.o function_cassini.o timing-text-io.o
  
  cassini.o : src/cassini.c
	$(CC) $(CFLAGS) -o cassini.o -c src/cassini.c
  
  function_cassini.o : src/function_cassini.c
	$(CC) $(CFLAGS) -o function_cassini.o -c src/function_cassini.c
	
  saturnd : saturnd.o function_saturnd.o function_saturnd_requete.o function_saturnd_cmd.o timing-text-io.o 
	$(CC) -o saturnd saturnd.o function_saturnd.o function_saturnd_requete.o function_saturnd_cmd.o timing-text-io.o
  
  saturnd.o : src/saturnd.c
	$(CC) $(CFLAGS) -o saturnd.o -c src/saturnd.c
  
  function_saturnd.o : src/function_saturnd.c 
	$(CC) $(CFLAGS) -o function_saturnd.o -c src/function_saturnd.c
	
  function_saturnd_requete.o : src/function_saturnd_requete.c 
	$(CC) $(CFLAGS) -o function_saturnd_requete.o -c src/function_saturnd_requete.c
	
  function_saturnd_cmd.o : src/function_saturnd_cmd.c
	$(CC) $(CFLAGS) -o function_saturnd_cmd.o -c src/function_saturnd_cmd.c
     
  timing-text-io.o : src/timing-text-io.c
	$(CC) $(CFLAGS) -o timing-text-io.o -c src/timing-text-io.c
  
  distclean : 
	rm -rf *.o
