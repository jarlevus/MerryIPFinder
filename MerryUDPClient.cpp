/*
An MS Windows version
compiled on Microsoft Visual Studio Community 2019, Version 16.7.6
20 October 2020
*/
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "WS2_32.lib")

#define THEPORT "7777"
#define MAXBUFLEN 1024
#define RECBYTES 7

char buf[MAXBUFLEN];
char buffer[INET6_ADDRSTRLEN];

int main(int argc, char** argv)
{
	WSADATA wsaData;
	int returnVal;
	int bytesSent;
	char yes = 1;

	ADDRINFOA hint;
	PADDRINFOA res, p;
	SOCKET sock;

	if (argc != 3) {
		fprintf(stdout, "Enter in the following format: <address> <message>\n");
		return 1;
	}
	
	if ((returnVal = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		fprintf(stderr, "WSAStartup failed with error %d", returnVal);
		return 1;
	}

	ZeroMemory(&hint, sizeof hint);
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_DGRAM;

	if ((returnVal = getaddrinfo(argv[1], THEPORT, &hint, &res)) != 0) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerrorA(returnVal));
		WSACleanup();
		return 2;
	}

	for (p = res; p != NULL; p = p->ai_next) {
		if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			fprintf(stderr, "socket() error: %d", WSAGetLastError());
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "Something wrong happened to the addresspointer p\n.");
		freeaddrinfo(res);
		closesocket(sock);
		WSACleanup();
		return 3;
	}

	if ((setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof yes)) == -1) {
		fprintf(stderr, "socket() error: %d", WSAGetLastError());
		freeaddrinfo(res);
		closesocket(sock);
		WSACleanup();
		return 4;
	}

	if ((bytesSent = sendto(sock, argv[2], strlen(argv[2]), 0,
		p->ai_addr, p->ai_addrlen)) < 0) {
		fprintf(stderr, "sendto() error: %s", gai_strerrorA(WSAGetLastError()));
		freeaddrinfo(res);
		closesocket(sock);
		WSACleanup();
		return 5;
	}

	fprintf(stdout, "%d bytes sent.\n", bytesSent);

	freeaddrinfo(res);
	closesocket(sock);
	WSACleanup();
	return 0;
}
