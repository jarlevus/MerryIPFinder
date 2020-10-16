/*
An MS Windows version
compiled on Microsoft Visual Studio Community 2019, Version 16.7.6
16 October 2020
*/

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "WS2_32.lib")

#define THEPORT "https"

int main(int argc, char **argv)
{
	WSADATA wsaData;
	int returnVal;

	ADDRINFOA hint;
	PADDRINFOA res, p;

	char buffer[INET6_ADDRSTRLEN];

	if (argc != 2) {
		fprintf(stderr, "Please specify the desired URL\n");
		return 1;
	}

	if ((returnVal = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		fprintf(stderr, "WSAStartup failed with error %d", returnVal);
		return 2;
	}

	ZeroMemory(&hint, sizeof hint);
	hint.ai_family = AF_UNSPEC;
	hint.ai_socktype = SOCK_STREAM;

	if ((returnVal = getaddrinfo(argv[1], THEPORT, &hint, &res)) != 0) {
		fprintf(stderr, "%s\n", gai_strerrorA(returnVal));
		WSACleanup();
		return 2;
	}

	for (p = res; p != NULL; p = p->ai_next) {

		void* address = &((PSOCKADDR_IN)p->ai_addr)->sin_addr;
		PCHAR version = "IPv4";

		if (p->ai_family == AF_INET6) {
			address = &((PSOCKADDR_IN6)p->ai_addr)->sin6_addr;
			version = "IPv6";
		}
		
		fprintf(stdout, "%s: %s\n", version,
			inet_ntop(p->ai_family, address, buffer, INET6_ADDRSTRLEN));
	}
	freeaddrinfo(res);
	WSACleanup();
	return 0;
}
