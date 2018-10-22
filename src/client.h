#ifndef CLIENT_H_
#define CLIENT_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include "parameters.h"

extern int tcp_socket;
extern int udp_socket;
extern uint16_t my_port;
extern struct in_addr my_ip;
extern bool logged;
extern char* current_user;


//function in client_command
void parse_arguments(int argc, char* argv[], struct in_addr* ma, uint16_t* mp, struct in_addr* sa, uint16_t* sp);
void showconnection(struct in_addr sa, int sp, int mp);
int read_execute_command();
void showcommands();
int _register();
int _deregister();
int _quit();
int _who();
int _send();

//function in client_network
int download_offline_msg();
int rcv_destination_addres(struct sockaddr_in* dest_addr);
int rcv_response(uint16_t* response_code);
int rcv_udp_msg(char* msgbuf);
int rcv_users();
int send_addr();
int send_cmd(uint16_t cmd_code);
int send_cmd_arg(uint16_t cmd_code, char* username);
int send_tcp_msg(char* msg, uint16_t len);
int send_udp_msg(char* body, struct sockaddr_in dest_addr);

#endif
