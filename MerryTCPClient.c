/*
Compiled on Microsoft Visual Studio Community 2019, Version 16.7.6
on October 26, 2020
*/

#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define THEPORT "7777"
#define BUFLEN 1024

char buf[BUFLEN];

int main(int argc, char** argv)
{
	if (argc < 2) {
		fprintf(stderr, "Please provide at least an IP or URL address\n");
		return 13;
	}

	int iRet;
	char yes = 1;
		
	WSADATA wsaData;
	ADDRINFOA hints;
	PADDRINFOA res, p;
	SOCKET sock;

	if ((iRet = WSAStartup(MAKEWORD(2, 2), &wsaData)) != NO_ERROR) {
		fprintf(stderr, "WSAStartup() error: %s\n", gai_strerrorA(WSAGetLastError()));
		return 1;
	}

	ZeroMemory(&hints, sizeof hints);
	hints.ai_flags = AI_CANONNAME;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

	if ((iRet = getaddrinfo(argv[1], THEPORT, &hints, &res)) != NO_ERROR) {
		fprintf(stderr, "Getaddrinfo() error: %s\n", gai_strerrorA(WSAGetLastError()));
		WSACleanup();
		return 2;
	}

	for (p = res; p != NULL; p = p->ai_next) {
		if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET) {
			fprintf(stderr, "socket() error: %s\n", gai_strerrorA(WSAGetLastError()));
			continue;
		}

		if ((iRet = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes))) {
			fprintf(stderr, "setsockopt() error: %s\n", gai_strerrorA(WSAGetLastError()));
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "Problem with socket creation\n");
		closesocket(sock);
		WSACleanup();
		return 4;
	}

	if ((iRet = connect(sock, p->ai_addr, p->ai_addrlen)) == SOCKET_ERROR) {
		fprintf(stderr, "connect() error: %s\n", gai_strerrorA(WSAGetLastError()));
		closesocket(sock);
		WSACleanup();
		return 5;
	}

	for (;;) {

		fprintf(stdout, "Message to be sent: ");
		fgets(buf, BUFLEN, stdin);

		if ((iRet = send(sock, buf, strlen(buf), 0)) == SOCKET_ERROR) {
			fprintf(stderr, "send() error: %s\n", gai_strerrorA(WSAGetLastError()));
			closesocket(sock);
			WSACleanup();
			return 5;
		}
	}

	closesocket(sock);
	WSACleanup();
	return 0;
}