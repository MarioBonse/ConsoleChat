#include "server.h"

//chiude il socket e chiude il thread
void close_sock(int sock) {
	close(sock);
	printf("Socket chiuso %d\n", sock);
	pthread_exit(NULL);
}

//ottiene un comando dal client, se il comando ha un secondo argomento(username) allora legge anche quello
int get_cmd(int sock, uint16_t* cmd, char** username) {
	int ret;
	ret = recv(sock, (void*)cmd, sizeof(uint16_t), 0);
	if (ret < 0) {
    perror("Fallita la recv()");
		printf("socket = %d\n", sock);
		return -1;
	} else if (ret == 0) {
		_quit(sock);
		close_sock(sock);
		return -1;
	}
	*cmd = ntohs(*cmd);
	if (*cmd >= NUM_OF_COMMANDS) {
		printf("Comando non valido dal socket %d, codice = %u .\n",sock, (unsigned int)*cmd);
		return -1;
	}
	if (*cmd == CMD_REGISTER  || *cmd == CMD_SEND) {
		uint16_t usrlen;
		ret = recv(sock, (void*)&usrlen, sizeof(uint16_t), 0);
		if (ret < 0) {
	    perror("Fallita la recv()");
			printf("Errore nel ricevre il comando: socket = %d\n", sock);
			return -1;
		} else if (ret == 0) {
			_quit(sock);
			close_sock(sock);
			return -1;
		}
		usrlen = ntohs(usrlen);
		*username = (char*)malloc(usrlen);
		ret = recv(sock, (void*)*username, usrlen, 0);
		if (ret < 0) {
	    perror("Fallita la recv()");
			printf("Errore nel ricevere l'username: socket = %d\n", sock);
			return -1;
		} else if (ret == 0) {
			_quit(sock);
			close_sock(sock);
			return -1;
		}
	}
	return 0;
}

//riceve un messaggio via tcp e lo salva
int read_message(int sock, char* sender, char* destination) {
	int ret;
	uint16_t len;
	char* msg;
	ret = recv(sock, (void*)&len, sizeof(uint16_t), 0);
	if (ret < 0) {
		perror("Fallita la recv()");
		printf("Errore nel ricevere la dimensione del messaggio: socket = %d\n", sock);
		return -1;
	} else if (ret == 0) {
		_quit(sock);
		close_sock(sock);
		return -1;
	}
	len = ntohs(len);
	msg = (char*)malloc(len);
	ret = recv(sock, (void*)msg, len, 0);
	if (ret < 0) {
		perror("Fallita la recv()");
		printf("Errore nel ricevere il messaggio: socket = %d\n", sock);
		return -1;
	} else if (ret == 0) {
		_quit(sock);
		close_sock(sock);
		return -1;
	}
	printf("User %s ha mandato un messaggio offline a %s\n", sender, destination);
	ret = store_message(sender, destination, msg);
	return ret;
}

//riceve l'indirizzo da un client e lo salva nel suo descrittore
int recive_addr(int sock, struct in_addr* ip, uint16_t* port) {
	int ret;
	ret = recv(sock, (void*)ip, sizeof(struct in_addr), 0);
	if (ret < 0) {
		perror("Fallita la recv()");
		printf("Errore nel ricevere l'ip: socket = %d\n", sock);
		return -1;
	} else if (ret == 0) {
		_quit(sock);
		close_sock(sock);
		return -1;
	}
	ret = recv(sock, (void*)port, sizeof(uint16_t), 0);
	if (ret < 0) {
		perror("Fallita la recv()");
		printf("Errore nel ricevere la porta: socket = %d\n", sock);
		return -1;
	} else if (ret == 0) {
		_quit(sock);
		close_sock(sock);
		return -1;
	}
	*port = ntohs(*port);
	return 0;
}

//manda un indirizzo a un client cosicchè possa mandare un messaggio udp a un altro client connesso
int send_addr(int sock, struct sockaddr_in full_addr) {
	int ret;
	char text_addr[100];		//send 100 bytes, it's always enough to store the address in text format
	sprintf(text_addr, "%hu %u", full_addr.sin_port, full_addr.sin_addr.s_addr);
	printf("%s\n", text_addr);
	ret = send(sock , (void*)text_addr , 100, 0);
	if(ret < 0){
		perror("send()");
		printf("Errore nel mandare un indirizzo a un client: socket = %d\n",sock);
		return -1;
	}
	return 0;
}

// manda un emssaggio al client appena registrato
int send_msg(int sock, message* msg) {
	int ret;
	uint16_t sender_len, body_len, netsender_len, netbody_len;
	sender_len = strlen(msg->sender) + 1;
	body_len = strlen(msg->body_text) + 1;
	netsender_len = htons(sender_len);
	netbody_len = htons(body_len);
	ret = send(sock, (void*)&netsender_len, sizeof(uint16_t), 0);
	if(ret < 0){
		perror("send()");
		printf("Errore nel mandare la lunghezza del mittente: socket = %d\n",sock);
		return -1;
	}
	ret = send(sock, (void*)&netbody_len, sizeof(uint16_t), 0);
	if(ret < 0){
		perror("send()");
		printf("Errore nel mandare la lunghezza del messaggio socket = %d\n",sock);
		return -1;
	}
	ret = send(sock, (void*)(msg->sender), sender_len, 0);
	if(ret < 0){
		perror("send()");
		printf("Errore nel mandare il mittente: socket = %d\n",sock);
		return -1;
	}
	ret = send(sock, (void*)(msg->body_text), body_len, 0);
	if(ret < 0){
		perror("send()");
		printf("Errore nel mandare il messaggio: socket = %d\n",sock);
		return -1;
	}
	return 0;
}

//manda tutti i messaggi al client appena connesso
int send_allmsgs(int sock, list_chat_user_node* user_node) {
	int ret;
	list_message_node* lmnp;
	uint16_t zero = 0;

	while (user_node->pending_msg != NULL) {	//finchè ne ho
		lmnp = user_node->pending_msg;
		ret = send_msg(sock, (message*)(lmnp));
		if(ret < 0){
			return -1;
		}
		//qui non utilizzo la mutua in quanto un sol thread serve un user e quindi i suoi messaggi
		lmnp = list_message_extract_head(&(user_node->pending_msg));
		delete_message(lmnp);								//and delete it from the buffer
	}
	//notifico che non ho più messaggi mandando zero
	ret = send(sock, (void*)&zero, sizeof(uint16_t), 0);
	if(ret < 0){
		perror("send()");
		printf("Errore nel mandare l'avviso di fine messaggi: socket = %d\n",sock);
		return -1;
	}
	return 0;
}

// manda un comando al client
int send_response(int sock, uint16_t response_code) {
	int ret;
	response_code = htons(response_code);
	ret = send(sock, (void*)&response_code, sizeof(response_code), 0);
	if(ret < 0){
		perror("send()");
		printf("Errore nel mandare il comando al client: socket = %d\n",sock);
		return -1;
	}
	return 0;
}

//mando tutta la lista degli user
int send_users_list(int sock) {
	char* buf;
	int ret;
	uint32_t list_len = 1;
	uint32_t netlist_len;
	list_chat_user_node* p = lcu;
//calcolo la lunghezza del buffer
	while (p != NULL) {
		int name_len;
		name_len = strlen(p->username);
		list_len += name_len;
		if (p->online)
			list_len += 9;	//(online)\n  -> 9 characters
		else
			list_len += 10;	//(offline)\n -> 10 characters
		p = p->next;
	}
	netlist_len = htonl(list_len);
	ret = send(sock, (void*)&netlist_len, sizeof(uint32_t), 0);
	if(ret < 0){
		perror("send()");
		printf("Errore nel mandare la dimensione della listadegli user: socket = %d\n",sock);
		return -1;
	}
	buf = (char*)malloc(list_len);
	buf[0] = '\0';
	p = lcu;
	while (p != NULL) {
		strcat(buf, p->username);
		if (p->online)
			strcat(buf, "(online)\n");
		else
			strcat(buf, "(offline)\n");
		p = p->next;
	}
	//mando il buffer
	ret = send(sock, (void*)buf, list_len, 0);
	if(ret < 0){
		perror("send()");
		printf("Errore nel mandare la lista degli user: socket = %d\n",sock);
		return -1;
	}
	free(buf);
	return 0;
}
