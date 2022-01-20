all: statclient statclient_th statserver statserver_th

statclient: statclient.c
	gcc -Wall -g -o statclient statclient.c -lrt
	
statclient_th: statclient_th.c
	gcc -Wall -g -pthread -o statclient_th statclient_th.c -lrt
	
statserver: statserver.c
	gcc -Wall -g -o statserver statserver.c -lrt
	
statserver_th: statserver_th.c
	gcc -Wall -g -pthread -o statserver_th statserver_th.c -lrt
	
 clean:
	rm -fr *~ *.o statclient statclient_th statserver statserver_th
