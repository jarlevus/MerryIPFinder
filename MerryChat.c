/*
Compiled on Microsoft Visual Studio Community 2019, Version 16.7.6
on October 29, 2020
*/

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT "7777"


void *get_in_addr(PSOCKADDR sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((PSOCKADDR_IN)sa)->sin_addr);
	}

	return &(((PSOCKADDR_IN6)sa)->sin6_addr);
}

int main(int argc, char **argv)
{
	WSADATA wsaData;
	FD_SET master;    
    FD_SET read_fds;
	int fdmax;

	int listener;
	int newfd;
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen;

	char buf[256];
	int nbytes;

	char remoteIP[INET6_ADDRSTRLEN];

	char yes = 1;
	int i, j, rv;

    ADDRINFOA hints;
    PADDRINFOA ai, p;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
		fprintf(stderr, "WSAStartup() failed: %s\n", gai_strerrorA(WSAGetLastError()));
		return 1;
	}

    ZeroMemory(&hints, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != NO_ERROR) {
        fprintf(stderr, "getaddrinfo() failed: %s\n", gai_strerrorA(WSAGetLastError()));
        WSACleanup();
        return 2;
    }

    for (p = ai; p != NULL; p = p->ai_next) {
        if ((listener = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == SOCKET_ERROR)
                continue;

        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == SOCKET_ERROR) {
            closesocket(listener);
            continue;
        }  

        if (bind(listener, p->ai_addr, p->ai_addrlen) == SOCKET_ERROR) {
            closesocket(listener);
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        closesocket(listener);
        WSACleanup();
        return 3;
    }

    freeaddrinfo(ai);

    if (listen(listener, 10) == SOCKET_ERROR) {
        fprintf(stderr, "Listen() failed: %s\n", gai_strerrorA(WSAGetLastError()));
        closesocket(listener);
        WSACleanup();
        return 4;
    }

    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(listener, &master);
    fdmax = listener;

    for (;;) {
        read_fds = master;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == SOCKET_ERROR) {
            fprintf(stderr, "socket() failed: %s\n", gai_strerrorA(WSAGetLastError()));
            closesocket(listener);
            WSACleanup();
            return 5;
        }

        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == listener) {
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (PSOCKADDR)&remoteaddr,
                        &addrlen);

                    if (newfd == INVALID_SOCKET) {
                        fprintf(stderr, "accept() failed: %s\n", gai_strerrorA(WSAGetLastError()));
                    }
                    else {
                        FD_SET(newfd, &master);
                        if (newfd > fdmax) {
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                            "socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((PSOCKADDR)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN), newfd);
                        send(newfd, "Welcome to LevServer! Enjoy your time!\n", 40, 0);
                    }
                }
                else {
                    
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        
                        if (nbytes == 0) {
                            
                            printf("selectserver: socket %d hung up\n", i);
                        }
                        else {
                            perror("recv");
                        }
                        closesocket(i);
                        FD_CLR(i, &master);
                    }
                    else {
                        
                        for (j = 0; j <= fdmax; j++) {
                            
                            if (FD_ISSET(j, &master)) {
                                
                                if (j != listener && j != i) {
                                    if (send(j, buf, nbytes, 0) == SOCKET_ERROR) {
                                        fprintf(stderr, "send() error: %s\n",
                                            gai_strerrorA(WSAGetLastError()));
                                    }
                                }
                            }
                        }
                    }
                } 
            }
        }
    }
	WSACleanup();
	return 0;
}