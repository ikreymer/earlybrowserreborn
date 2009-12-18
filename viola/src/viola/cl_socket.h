/*
 * class        : socket
 * superClass	: client
 */
#include "cl_client.h"

extern ClassInfo class_socket;
int helper_socket_get();
int helper_socket_set();

int meth_socket__startClient();
int meth_socket_freeSelf();
int meth_socket_get();
int meth_socket_set();
