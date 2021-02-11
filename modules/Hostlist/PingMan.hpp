#pragma once
#include <windows.h>
#include <winsock2.h>
#include "Host.hpp"
#include "Status.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>

#define PING_UPDATE_RATE 2000L
#define MESSAGE_LEN 37

extern LARGE_INTEGER timer_frequency;

// This is very messy, but it works and that's enough for me
// If someone else wants to fix it up, please do.
namespace PingMan {
enum {
	ERROR_INITFAILED = -1,
	ERROR_SOCKETFAILED = -2,
	ERROR_SENDTOFAILED = -3,
	ERROR_SELECTFAILED = -4,
	ERROR_PINGTIMEOUT = -5,
	ERROR_RECVFROMFAILED = -6,
	ERROR_INVALIDRESPONSE = -7,
	ERROR_INVALIDIP = -8,

	PING_UNINITIALIZED = -9,

	PACKET_OLLEH = 3
};

struct PingInfo {
	unsigned long long oldTime;
	unsigned long long ping;

	PingInfo(): oldTime(0), ping(PING_UNINITIALIZED){};
};

WSADATA wsa;
char message[MESSAGE_LEN];
std::unordered_map<long, PingInfo> pings;
SOCKET sock = INVALID_SOCKET;

// Helper functions
namespace {
void MessageSetup(char *message, SOCKADDR_IN *addr) {
	memset(message, 0, MESSAGE_LEN);
	message[0] = 1;
	memcpy(message + 1, addr, sizeof(SOCKADDR_IN));
	memcpy(message + 17, addr, sizeof(SOCKADDR_IN));
	message[36] = '\xBC';
}

int SockAddrInSetup(SOCKADDR_IN *addr, const char *ipstr, short port) {
	unsigned long ip;
	if ((ip = inet_addr(ipstr)) == INADDR_NONE) {
		printf("[WinSock] Invalid ip string %s.\n", ipstr);
		return ERROR_INVALIDIP;
	}

	memset(addr, 0, sizeof(SOCKADDR_IN));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	addr->sin_addr.S_un.S_addr = ip;

	return 0;
}

int SocketSend(long ip, short port) {
	SOCKADDR_IN addr;

	addr.sin_family = AF_INET;
	addr.sin_port = port;
	addr.sin_addr.S_un.S_addr = ip;

	MessageSetup(message, &addr);

	if (sendto(sock, message, MESSAGE_LEN, 0, (SOCKADDR *)&addr, sizeof(addr)) == SOCKET_ERROR) {
		printf("[WinSock] sendto() failed with error code: %d.\n", WSAGetLastError());
		return ERROR_SENDTOFAILED;
	}

	return 0;
}

int SocketReceive(long *ip) {
	char response;
	SOCKADDR_IN addr;
	int addr_len = sizeof(addr);
	// non blocking recvfrom b/c of socket receive timeout
	if (recvfrom(sock, &response, sizeof(response), NULL, (SOCKADDR *)&addr, &addr_len) < 0) {
		if (WSAGetLastError() == WSAETIMEDOUT) {
			return ERROR_PINGTIMEOUT;
		}
		printf("[WinSock] recvfrom() failed with error code : %d.\n", WSAGetLastError());
		return ERROR_RECVFROMFAILED;
	}

	if (response == PACKET_OLLEH) {
		*ip = addr.sin_addr.S_un.S_addr;
		return 0;
	} else
		return ERROR_INVALIDRESPONSE;
}
}; // namespace

int Init() {
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("[WinSock] Winsock Init failed with error code: %d.\n", WSAGetLastError());
		return ERROR_INITFAILED;
	}

	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) {
		printf("[WinSock] socket() failed with error code: %d.\n", WSAGetLastError());
		return ERROR_SOCKETFAILED;
	}

	// force noop bind to register socket to autopunch
	SOCKADDR_IN local;
	SockAddrInSetup(&local, "0.0.0.0", 0);
	::bind(sock, (SOCKADDR *)&local, sizeof(local));

	// don't use select, because autopunch doesn't handle those
	// and could cause select to return that the socket is ready,
	// but the read call could read a packet from the relay, then
	// read again internally and block.
	// instead, set a receive timeout (can't bother using nonblocking
	// sockets ;)) and do a regular read.
	// use a 1ms recv timeout for the non blocking read
	// (because a 0ms recv timeout means "block forever" in win32).
	// good enough.
	DWORD timeout = 1;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)(&timeout), sizeof(timeout));

	return 0;
}

void Cleanup() {
	WSACleanup();
}

// Credits to cc for the central ping manager idea.
long Ping(long ip, short port) {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	unsigned long long newTime = counter.QuadPart * 1000 / timer_frequency.QuadPart;
	PingInfo &ping = pings[ip];
	if (newTime - ping.oldTime > PING_UPDATE_RATE) {
		if (SocketSend(ip, port) == 0) {
			ping.oldTime = newTime;
		}
	}
	return ping.ping;
}

long Ping(Host &host) {
	return Ping(host.netIp, host.netPort);
}

void Update() {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	unsigned long long time = counter.QuadPart * 1000 / timer_frequency.QuadPart;

	long ip;
	while (SocketReceive(&ip) == 0) {
		PingInfo &ping = pings[ip];
		ping.ping = (time - ping.oldTime) / 2;
	}
}
} // namespace PingMan
