/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_socket.h
 * Copyright (C) 2015 Dr.NP <np@bsgroup.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Unknown nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Unknown AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL Unknown OR ANY OTHER
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Socket header
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/09/2015
 * @changelog
 *      [02/09/2015] - Creation
 */

#ifndef _NET_BSP_SOCKET_H

#define _NET_BSP_SOCKET_H
/* Headers */

/* Definations */
// Inet type of socket
typedef enum bsp_inet_type_e
{
    BSP_INET_ANY        = 0x0, 
#define BSP_INET_ANY                    BSP_INET_ANY
    BSP_INET_IPV4       = 0x1, 
#define BSP_INET_IPV4                   BSP_INET_IPV4
    BSP_INET_IPV6       = 0x2, 
#define BSP_INET_IPV6                   BSP_INET_IPV6
    BSP_INET_LOCAL      = 0x7F
#define BSP_INET_LOCAL                  BSP_INET_LOCAL
} BSP_INET_TYPE;

// Socket type
typedef enum bsp_sock_type_e
{
    BSP_SOCK_ANY        = 0x0, 
#define BSP_SOCK_ANY                    BSP_SOCK_ANY
    BSP_SOCK_TCP        = 0x1, 
#define BSP_SOCK_TCP                    BSP_SOCK_TCP
    BSP_SOCK_UDP        = 0x2, 
#define BSP_SOCK_UDP                    BSP_SOCK_UDP
    BSP_SOCK_SCTP       = 0x3, 
#define BSP_SOCK_SCTP                   BSP_SOCK_SCTP
    BSP_SOCK_SCTP_TO_ONE
                        = 0x31, 
#define BSP_SOCK_SCTP_TO_ONE            BSP_SOCK_SCTP_TO_ONE
    BSP_SOCK_SCTP_TO_MANY
                        = 0x32
#define BSP_SOCK_SCTP_TO_MANY           BSP_SOCK_SCTP_TO_MANY
} BSP_SOCK_TYPE;

// Socket state
enum bsp_sock_state_e
{
    BSP_SOCK_STATE_IDLE
                        = 0b00000000, 
#define BSP_SOCK_STATE_IDLE             BSP_SOCK_STATE_IDLE
    BSP_SOCK_STATE_ACCEPTABLE
                        = 0b00000001, 
#define BSP_SOCK_STATE_ACCEPTABLE       BSP_SOCK_STATE_ACCEPTABLE
    BSP_SOCK_STATE_CONNECTED
                        = 0b00000010, 
#define BSP_SOCK_STATE_CONNECTED        BSP_SOCK_STATE_CONNECTED
    BSP_SOCK_STATE_READABLE
                        = 0b00000100, 
#define BSP_SOCK_STATE_READABLE         BSP_SOCK_STATE_READABLE
    BSP_SOCK_STATE_WRITABLE
                        = 0b00001000, 
#define BSP_SOCK_STATE_WRITABLE         BSP_SOCK_STATE_WRITABLE
    BSP_SOCK_STATE_ERROR
                        = 0b00010000, 
#define BSP_SOCK_STATE_ERROR            BSP_SOCK_STATE_ERROR
    BSP_SOCK_STATE_PRECLOSE
                        = 0b00100000, 
#define BSP_SOCK_STATE_PRECLOSE         BSP_SOCK_STATE_PRECLOSE
    BSP_SOCK_STATE_CLOSE
                        = 0b01000000
#define BSP_SOCK_STATE_CLOSE            BSP_SOCK_STATE_CLOSE
};

#define BSP_MAX_SERVER_SOCKETS          128

/* Macros */

/* Structs */
typedef struct bsp_socket_t
{
    // Summaries
    int                 fd;
    struct sockaddr_storage
                        saddr;
    struct addrinfo     addr;
    BSP_INET_TYPE       inet_type;
    BSP_SOCK_TYPE       sock_type;

    // State
    int                 state;

    // Reflex
    void                *ptr;
} BSP_SOCKET;

typedef struct bsp_socket_client_t
{
    struct bsp_socket_t sck;
    time_t              last_active;
    void                *additional;
} BSP_SOCKET_CLIENT;

typedef struct bsp_socket_connector_t
{
    struct bsp_socket_t sck;
    time_t              last_active;
    void                *additional;
} BSP_SOCKET_CONNECTOR;

typedef struct bsp_socket_server_t
{
    struct bsp_socket_t scks[BSP_MAX_SERVER_SOCKETS];
    size_t              nscks;
    void                *additional;
} BSP_SOCKET_SERVER;

/* Functions */
/**
 * Initialize socket mempool
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_socket_init();

/**
 * Create a new socket server, listened on a network port
 * One server should have multiple socket file descrptions
 *
 * @param string addr Address (Domain or IP) to bind
 * @param int port Port number to listen
 * @param int inet_type INET_*
 * @param int sock_type SOCK_*
 *
 * @return p BSP_SOCKET_SERVER
 */
BSP_DECLARE(BSP_SOCKET_SERVER *) bsp_new_net_server(const char *addr, uint16_t port, BSP_INET_TYPE inet_type, BSP_SOCK_TYPE sock_type);

/**
 * Create a new socket server, listened on UNIX local sock pipe
 *
 * @param string sock_file Sock filename
 * @param int access_mask User mask of sock file
 *
 * @return p BSP_SOCKET_SERVER
 */
BSP_DECLARE(BSP_SOCKET_SERVER *) bsp_new_unix_server(const char *sock_file, uint16_t access_mask);

/**
 * Connect to a remote socket server by given address:port
 *
 * @param string addr Address (Domain or IP) to connect
 * @param int port Port to connect
 * @param int inet_type INET_*
 * @param int sock_type SOCK_*
 *
 * @return p BSP_SOCKET_CONNECTOR
 */
BSP_DECLARE(BSP_SOCKET_CONNECTOR *) bsp_new_net_connector(const char *addr, uint16_t port, BSP_INET_TYPE inet_type, BSP_SOCK_TYPE sock_type);

/**
 * Connect a local socket server (UNIX sock pipe) by given path
 *
 * @param string sock_file Sock filename
 *
 * @return p BSP_SOCKET_CONNECTOR
 */
BSP_DECLARE(BSP_SOCKET_CONNECTOR *) bsp_new_unix_connector(const char *sock_file);

/**
 * Create (accept) a new client by given socket (server)
 *
 * @param BSP_SOCKET sck Server's socket
 *
 * @return p BSP_SOCKET_SERVER
 */
BSP_DECLARE(BSP_SOCKET_CLIENT *) bsp_new_client(BSP_SOCKET *sck);

/**
 * Proceed socket IO
 *
 * @param BSP_SOCKET sck Socket to proceed
 *
 * @return int Status
 */
BSP_DECLARE(int) bsp_drive_socket(BSP_SOCKET *sck);

#endif  /* _NET_BSP_SOCKET_H */
