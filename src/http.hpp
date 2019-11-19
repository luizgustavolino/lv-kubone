#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>

#include "pad.hpp"

struct HTTP {

	void (*onRequest)(HTTP*);
	int clientfd;
	Pad pad;

	HTTP(const char *port, void (*_onRequest)(HTTP*)) {
		onRequest = _onRequest;
		bindSocket(port);
		acceptFoverer();
	}

private:

	int listenfd;
	struct sockaddr_in clientaddr;
	socklen_t addrlen;

	void acceptFoverer(){
		while(true) {

			addrlen = sizeof(clientaddr);
			clientfd = accept(listenfd, (struct sockaddr *) &clientaddr, &addrlen);

			if (clientfd < 0){
				throw std::runtime_error("accept() error");
			} else {
				respond(0);
			}
		}
	}

	void respond(int n) {

		int rcvd;
		char msg[99999];

		memset( (void*)msg, (int)'\0', 99999 );
		rcvd = recv(clientfd, msg, 99999, 0);

		if (rcvd < 0) {

			// receive error
			fprintf(stderr, ("Erro on recv().\n"));

		} else if (rcvd == 0) {

			// receive socket closed
			fprintf(stderr, "Client disconnected upexpectedly.\n");

		} else {

			// receive message
			printf("%s\n", msg);

			const char* hint = "wasd=\0";
			char* location = strstr(msg, hint);

			if (location != NULL) {

  				pad.up    = ((unsigned short) *(location + 5)) - 48;
  				pad.left  = ((unsigned short) *(location + 6)) - 48;
  				pad.down  = ((unsigned short) *(location + 7)) - 48;
  				pad.right = ((unsigned short) *(location + 8)) - 48;

  				write(clientfd, "HTTP/1.0 200 OK\n\n", 17);
				if(onRequest != NULL) onRequest(this);
				
			} else {
				write(clientfd, "HTTP/1.0 400 BAD REQUEST\n\n", 26);
			}
		}

		shutdown(clientfd, SHUT_RDWR);
		close(clientfd);
		clientfd = -1;
	}

	bool bindSocket(const char *port){

    	struct addrinfo hints, *res, *p; 

    	// from man7: hints = criteria for selecting the socket address
    	memset (&hints, 0, sizeof(hints));
    	hints.ai_family 	= AF_INET;
    	hints.ai_socktype 	= SOCK_STREAM;
   	 	hints.ai_flags 		= AI_PASSIVE;

    	if(getaddrinfo( NULL, port, &hints, &res) != 0){
        	throw std::runtime_error("getaddrinfo() error");
        	exit(1);
   		}

    	// socket and bind
    	for(p = res; p != NULL; p = p->ai_next){

        	listenfd = socket(p->ai_family, p->ai_socktype, 0);
        	int option = 1;
        	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

        	if (listenfd == -1) continue;
        	if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
    	}
    
    	if(p == NULL){
        	throw std::runtime_error("fail to socket or bind");
        	exit(1);
    	}

    	freeaddrinfo(res);

    	// listen for incoming connections
    	if( listen (listenfd, 1000000) != 0 ){
        	throw std::runtime_error("listen error");
        	exit(1);
    	}

    	printf("Simulator server started at %s%s%s%s\n","\033[92m", "localhost:", port ,"\033[0m");
	}
};