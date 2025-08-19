#define _CRT_SECURE_NO_WARNINGS
#include "labview_bridge.h"

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

static SOCKET g_socket = INVALID_SOCKET;
static bool g_initialized = false;

bool lvb_init(const char* host, unsigned short port) {
	if (g_initialized) return true;

	WSADATA wsaData;
	int wsaerr = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaerr != 0) {
		return false;
	}

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	char port_str[16];
	snprintf(port_str, sizeof(port_str), "%hu", port);

	struct addrinfo* result = NULL;
	if (getaddrinfo(host, port_str, &hints, &result) != 0) {
		WSACleanup();
		return false;
	}

	SOCKET s = INVALID_SOCKET;
	for (struct addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		s = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (s == INVALID_SOCKET) continue;
		if (connect(s, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
			closesocket(s);
			s = INVALID_SOCKET;
			continue;
		}
		break;
	}
	freeaddrinfo(result);

	if (s == INVALID_SOCKET) {
		WSACleanup();
		return false;
	}

	g_socket = s;
	g_initialized = true;
	return true;
}

void lvb_close(void) {
	if (g_socket != INVALID_SOCKET) {
		closesocket(g_socket);
		g_socket = INVALID_SOCKET;
	}
	if (g_initialized) {
		WSACleanup();
		g_initialized = false;
	}
}

void lvb_send_peak(bool is_positive, float amplitude, int bin_index) {
	if (!g_initialized || g_socket == INVALID_SOCKET) return;
	char buf[64];
	// Формат: P/N,amp,bin\n
	int len = snprintf(buf, sizeof(buf), "%c,%.6f,%d\n", is_positive ? 'P' : 'N', amplitude, bin_index);
	if (len <= 0) return;
	int sent = send(g_socket, buf, len, 0);
	(void)sent;
}


