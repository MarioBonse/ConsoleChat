#ifndef LIST_H_
#define LIST_H_

#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "parameters.h"


typedef struct message {
	char* sender;
	char* body_text;
	bool instant;
	struct message* next;
} message;
typedef message* list_message;
typedef message list_message_node;


typedef struct chat_user {
	char* username;
  int sock;
	struct sockaddr_in full_address;
  bool online;
	list_message pending_msg;
	struct chat_user *next;
} chat_user;
typedef chat_user* list_chat_user;
typedef chat_user list_chat_user_node;





//funzioni per chat_user
list_chat_user_node* list_chat_user_find_by_name(list_chat_user l, char* username);
list_chat_user_node* list_chat_user_find_by_socket(list_chat_user l, int sockfd);
void list_chat_user_add_head(list_chat_user* l, chat_user** cu);
//void print_chat_user(void* d);
bool list_chat_user_remove(list_chat_user* l, char* username);
bool list_chat_user_node_delete(list_chat_user_node* p);

//funzioni per message
list_message_node* list_message_extract_head(list_message* l);
void list_message_add_head(list_message* l, message** m);
bool remove_all_messages(list_message* l);
//messaggi
message* new_message(char* sender, char* content, bool instant);
bool delete_message(message* m);

//Creazione sockaddr_in
struct sockaddr_in build_sockaddr_in(char* ip, unsigned short port);
struct sockaddr_in build_sockaddr_in2(struct in_addr ip, unsigned short port);

//Nuovo utente
bool delete_chat_user(chat_user* cu);
chat_user* new_chat_user(char *name, int sockfd, struct sockaddr_in full_addr);
chat_user* new_chat_user2(char *name, int sockfd, struct in_addr ip, unsigned short port);
chat_user* new_chat_user3(char *name, int sockfd, char* ip, unsigned short port);

#endif
