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

PVOID get_in_addr(PSOCKADDR sa);

int main(int argc, char** argv)
{
	WSADATA wsaData;
	ADDRINFOA hint;
	PADDRINFOA res, p;
	SOCKET sock;
	SOCKADDR_STORAGE retrieved_addr;
	socklen_t addr_len = sizeof retrieved_addr;

	int returnVal;
	int bytesReceived;
	char yes = 1;

	if ((returnVal = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		fprintf(stderr, "WSAStartup failed with error %d", returnVal);
		return 1;
	}

	ZeroMemory(&hint, sizeof hint);
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_DGRAM;
	hint.ai_flags = AI_PASSIVE;

	if ((returnVal = getaddrinfo("192.168.1.106", THEPORT, &hint, &res)) != 0) {
		fprintf(stderr, "getaddrinfo() error: %s\n", gai_strerrorA(returnVal));
		WSACleanup();
		return 2;
	}

	for (p = res; p != NULL; p = p->ai_next) {
		if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			fprintf(stderr, "socket() error: %d", WSAGetLastError());
			continue;
		}

		if ((returnVal = bind(sock, p->ai_addr, p->ai_addrlen)) == -1) {
			fprintf(stderr, "bind() error: %d", WSAGetLastError());
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "The socket was not bound. Alas!\n");
		return 3;
	}

	freeaddrinfo(res);

	fprintf(stdout, "Waiting for data to come ...\n");

	for (bytesReceived = RECBYTES; bytesReceived != SOCKET_ERROR;) {
		if ((bytesReceived = recvfrom(sock, buf, MAXBUFLEN, 0,
			(PSOCKADDR) &retrieved_addr, &addr_len)) <= 0) {
			fprintf(stderr, "recvfrom() error: %d", WSAGetLastError());
			return 4;
		}
		buf[bytesReceived] = '\0';
		fprintf(stdout, "Packet received from %s\n",
			inet_ntop(retrieved_addr.ss_family, get_in_addr((PSOCKADDR) &retrieved_addr),
				buffer, INET6_ADDRSTRLEN));

		fprintf(stdout, "Message: %s\nLength: %d bytes\n", buf, bytesReceived);
	}

	closesocket(sock);
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