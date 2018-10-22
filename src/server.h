#ifndef SERVER_H_
#define SERVER_H_

#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "list.h"
#include "parameters.h"

extern list_chat_user lcu;
extern pthread_mutex_t chat_user_list_mutex;

//server_command
int get_command();
int parse_port(int argc, char* argv[]);
int store_message(char* sender, char* dest, char* body);
int _command(int sock, uint16_t cmd_code, char* username);
int _help(int sock);
int _register(int sock, char* username);
int _deregister(int sock);
int _who(int sock);
int _send(int sock, char* dest_name);
int _quit(int sock);


//server_network
void close_sock(int sock);
int get_cmd(int sock, uint16_t* cmd, char** username);
int read_message(int sock, char* sender_name, char* dest_name);
int recive_addr(int sock, struct in_addr* ip, uint16_t* port);
int send_addr(int sock, struct sockaddr_in full_addr);
int send_response(int sock, uint16_t response_code);
int send_msg(int sock, message* msg);
int send_allmsgs(int sock, list_chat_user_node* user_node);
int send_users_list(int sock);


#endif
