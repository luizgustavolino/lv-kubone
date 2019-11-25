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

#define GAME_PAGE "<head><title>Snake over http</title><style> body { background-color: black; }  </style> </head> <body><img/> <script type=\"text/javascript\"> var dpad = {};	dpad.stateIdle 			= 0;	dpad.stateKeyDown 		= 1;	dpad.stateKeyPressed	= 2;	dpad.stateKeyUp 		= 3;	dpad.keys = {		up:{			state: 0,			next: 0,			dirty: false,			bindingCode: 87		},		down:{			state: 0,			next: 0,			dirty: false,			bindingCode: 83		},		left:{			state: 0,			next: 0,			dirty: false,			bindingCode: 65		},		right:{			state: 0,			next: 0,			dirty: false,			bindingCode: 68		}	};	dpad.update = function () {				for(var aKeyTag in dpad.keys){						var aKey = dpad.keys[aKeyTag];						if(aKey.state == dpad.stateKeyUp && !aKey.next){				aKey.state = dpad.stateIdle;			}else if(aKey.state == dpad.stateKeyDown && !aKey.next){				aKey.state = dpad.stateKeyPressed;			}else{				if(aKey.next){					aKey.state = aKey.next;				}			}						aKey.next 	= null;			aKey.dirty = false;		}	};	dpad.setup = function(){		document.onkeydown 	= dpad.keyDown;		document.onkeyup 	= dpad.keyUp;	};	dpad.keyDown = function () {		dpad.processKeyEvent(\"down\");	};	dpad.keyUp = function () {		dpad.processKeyEvent(\"up\");	};	dpad.processKeyEvent = function (type){				var x = null;		if(window.event) x = event.keyCode;		else if(event.which) x = event.which;				for(var aKeyTag in dpad.keys){					var aKey =  dpad.keys[aKeyTag];			if(aKey.bindingCode == x){				if(!aKey.dirty){					aKey.dirty = true;										if(aKey.state == dpad.stateIdle){						if(type == \"down\"){							aKey.next = dpad.stateKeyDown;							}					}else if (aKey.state == dpad.stateKeyDown) {						if(type == \"down\"){							aKey.next = dpad.stateKeyPressed;							}else if(type == \"up\"){							aKey.next = dpad.stateKeyUp;						}								}else if (aKey.state == dpad.stateKeyPressed) {						if(type == \"up\"){							aKey.next = dpad.stateKeyUp;							}								}				}else{					if(type == \"up\"){						aKey.next = dpad.stateKeyUp;					}				}			}		}			};	var image = document.images[0];	var downloadingImage = new Image();	var frame = 0;	downloadingImage.onload = function(){    	image.src = this.src;    	window.requestAnimationFrame(doFrame);	};	downloadingImage.onerror = function(){    	window.requestAnimationFrame(doFrame);	};	function doFrame(time){		if (frame++ % 2 == 0 ) {			dpad.update();			downloadingImage.src = \"/?salt=\" + (new Date().getTime()) + \"&wasd=\" 			+ dpad.keys.up.state + \"\"			+ dpad.keys.left.state + \"\"			+ dpad.keys.down.state + \"\"			+ dpad.keys.right.state + \"\";		} else {			window.requestAnimationFrame(doFrame);		}	};	dpad.setup();	window.requestAnimationFrame(doFrame);</script></body></html>"

struct HTTP {

	void (*onRequest)(HTTP*);
	int clientfd;
	Pad pad;

	HTTP(const char *port, void (*_onRequest)(HTTP*)) {
		onRequest = _onRequest;
		if (bindSocket(port)){
			acceptFoverer();
		}
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
				fprintf(stdout, "accept() error. skipping...");
				fflush(stdout);
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
			printf("2\n");
			fprintf(stdout, ("Erro on recv().\n"));
			fflush(stdout);

		} else if (rcvd == 0) {

			// receive socket closed
			printf("1\n");
			fprintf(stdout, "Client disconnected upexpectedly.\n");
			fflush(stdout);

		} else {

			// receive message
			// printf("%s\n", msg);

			const char* hint = "wasd=\0";
			char* location = strstr(msg, hint);

			if (location != NULL) {

  				pad.up    = ((unsigned short) *(location + 5)) - 48;
  				pad.left  = ((unsigned short) *(location + 6)) - 48;
  				pad.down  = ((unsigned short) *(location + 7)) - 48;
  				pad.right = ((unsigned short) *(location + 8)) - 48;

  				write(clientfd, "HTTP/1.1 200 OK\n", 16);
  				write(clientfd, "Content-Type: image/bmp\n", 24);
  				write(clientfd, "Expires: 0\n", 11);
  				write(clientfd, "Cache-Control: no-cache\n\n", 25);
				if(onRequest != NULL) onRequest(this);
				
			} else {
				write(clientfd, "HTTP/1.1 200 OK\n", 16);
				write(clientfd, "Content-Type: text/html;\n\n", 27);
				write(clientfd, GAME_PAGE, sizeof(GAME_PAGE));
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
        	fprintf(stdout, "getaddrinfo() error");
        	fflush(stdout);
        	return false;
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
        	fprintf(stdout, "fail to socket or bind");
        	fflush(stdout);
        	return false;
    	}

    	freeaddrinfo(res);

    	// listen for incoming connections
    	if( listen (listenfd, 1000000) != 0 ){
    		fprintf(stdout, "listen error");
    		fflush(stdout);	
    		return false;
    	}

    	fprintf(stdout, "Simulator server started at %s%s%s%s\n","\033[92m", "localhost:", port ,"\033[0m");
		fflush(stdout);
		return true;
	}
};