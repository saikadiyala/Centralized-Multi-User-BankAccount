 #defining the compiler
	GG = g++
	CFLAGS = -g -Wall -c
 #compile command  
  compile : server client
  
  server: server.o
	$(GG) server.o -o server -lpthread ../other/myqu.cpp
  client: client.o
	$(GG) client.o -o client -pthread

 server.o: server.cpp
	$(GG) -c -g server.cpp ../other/myqu.cpp -pthread
 client.o:client.cpp
	$(GG) -c client.cpp -pthread
	
  clean:
	rm -rf *.o compile
