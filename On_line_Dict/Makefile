CC=gcc                                                                                  
CFLAGS=-c -g
app:dict_server_main.o dict_server.o
	    $(CC)  $^ -o $@ -lsqlite3
dict_server_main.o:dict_server_main.c
	    $(CC) $(CFLAGS) $^ -o $@
dict_server.0:dict_server.c
	    $(CC) $(CFLAGS) $^ -o $@

