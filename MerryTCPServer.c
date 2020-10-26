/*
Compiled on Microsoft Visual Studio Community 2019, Version 16.7.6
on October 24, 2020
*/

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")
#define THEPORT "7777"
#define BACKLOG 13
#define BUFLEN 1024

char addrbuf[INET6_ADDRSTRLEN];
char buf[BUFLEN];

PVOID get_in_addr(PSOCKADDR sa);

int main(int argc, char** argv)
{
	WSADATA wsaData;
	PADDRINFOA res, p;
	ADDRINFOA hints;

	int iRes;
	char yes = 1;
	SOCKET sockServer;
	SOCKET sockClient;
	SOCKADDR_STORAGE incoming;
	socklen_t sin_size = sizeof incoming;

	if ((iRes = WSAStartup(MAKEWORD(2, 2), &wsaData)) != NO_ERROR) {
		fprintf(stderr, "WSAStartup() failure: %s", gai_strerrorA(WSAGetLastError()));
		return 1;
	}

	ZeroMemory(&hints, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;


	if ((iRes = getaddrinfo(NULL, THEPORT, &hints, &res)) != NO_ERROR) {
		fprintf(stderr, "getaddrinfo() failure: %s", gai_strerrorA(WSAGetLastError()));
		WSACleanup();
		return 2;
	}

	for (p = res; p != NULL; p = p->ai_next) {
		if ((sockServer = socket(p->ai_family,
			p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET) {
			fprintf(stderr, "socket() failure: %s", gai_strerrorA(WSAGetLastError()));
			continue;
		}

		if ((iRes = setsockopt(sockServer, SOL_SOCKET, SO_REUSEADDR,
			&yes, sizeof yes)) == SOCKET_ERROR) {
			fprintf(stderr, "setsockopt() failure: %s", gai_strerrorA(WSAGetLastError()));
			continue;
		}

		if ((iRes = bind(sockServer, p->ai_addr, p->ai_addrlen)) == SOCKET_ERROR) {
			fprintf(stderr, "bind() failure: %s\n", gai_strerrorA(WSAGetLastError()));
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "failed to bind the socket\n");
		closesocket(sockServer);
		freeaddrinfo(res);
		WSACleanup();
		return 3;
	}

	freeaddrinfo(res);

	if ((iRes = listen(sockServer, BACKLOG)) == SOCKET_ERROR) {
		fprintf(stderr, "listen() failure: %s\n", gai_strerrorA(WSAGetLastError()));
		closesocket(sockServer);
		WSACleanup();
		return 4;
	}
	fprintf(stdout, "waiting to connect...\n");
	for (;;) {
		if ((sockClient = accept(sockServer, (PSOCKADDR)&incoming,
			&sin_size)) == INVALID_SOCKET) {
			fprintf(stderr, "accept() failure: %s\n", gai_strerrorA(WSAGetLastError()));
			continue;
		}

		if (inet_ntop(incoming.ss_family,
			get_in_addr((PSOCKADDR)&incoming), addrbuf, INET6_ADDRSTRLEN) == NULL) {
			(stderr, "inet_ntop() failure: %s\n", gai_strerrorA(WSAGetLastError()));
			continue;
		}

		fprintf(stdout, "Connected with %s\n", addrbuf);

		for (iRes = 1; iRes > 0;) {

			if ((iRes = recv(sockClient, buf, BUFLEN, 0)) == SOCKET_ERROR) {
				fprintf(stderr, "recv() failed: %s\n", gai_strerrorA(WSAGetLastError()));
				closesocket(sockClient);
				closesocket(sockServer);
				WSACleanup();
				return 5;
			}
			if (iRes == 0)
				fprintf(stdout, "\nConnection ended\n");
			if (iRes > 0) {

				if(iRes <= BUFLEN) buf[iRes] = '\0';
				fprintf(stdout, "%s", buf);
			}
		}
		closesocket(sockClient);
	}

	closesocket(sockServer);
	WSACleanup();
	return 0;
}

PVOID get_in_addr(PSOCKADDR sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((PSOCKADDR_IN)sa)->sin_addr);
	}
	return &(((PSOCKADDR_IN6)sa)->sin6_addr);
}
