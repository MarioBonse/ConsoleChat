#include "client.h"

/* Scarica dal derver i messaggi che ho ricevuto mentre ero offline*/
int download_offline_msg() {
	int ret;
	uint16_t sender_len, body_len;
	char *sender, *body;
	bool told = false;

	while(1) {
		ret = recv(tcp_socket, (void*)&sender_len, sizeof(uint16_t), 0);
    if(ret < 0){
      perror("Errore nel ricevere la lunghezza del mittente");
      return -1;
    }
		sender_len = ntohs(sender_len);
		if (sender_len == 0)	//no messaggi
			break;
		if (!told) {
			printf("Qui ci sono i messaggi he hai ricevuto!\n");
			told = true;
		}
		ret = recv(tcp_socket, (void*)&body_len, sizeof(uint16_t), 0);
    if(ret < 0){
      perror("Errore nel ricevere il messaggio");
      return -1;
    }
		body_len = ntohs(body_len);
		sender = (void*)malloc(sender_len);
		body = (void*)malloc(body_len);
		ret = recv(tcp_socket, (void*)sender, sender_len, 0);
		if (ret < 0) {
			perror("Errore mentre si scaricava il messaggio\n");
			free(sender);
			free(body);
			return -1;
		}
		//ricevo il body
		ret = recv(tcp_socket, (void*)body, body_len, 0);
		if (ret < 0) {
			perror("Errore mentre si scaricava il messaggio");
			free(sender);
			free(body);
			return -1;
		}
		printf("%s (offline msg)>\n%s\n", sender, body);
		free(sender);
		free(body);
	}
	if (!told)
		printf("Non hai ricevuto messaggi.\n");
	return 0;
}

/* funzione per ricevere dal server un indirizzo a cui poi mandare il messaggio*/
int rcv_destination_addres(struct sockaddr_in* dest_addr) {
	char text_addr[100];		//server sends 100 bytes
	int ret;
	ret = recv(tcp_socket, (void*)text_addr, 100, 0);
  if(ret < 0){
    perror("Errore nel ricevere l'indirizzo dal server");
    return -1;
  }
	sscanf(text_addr, "%hu %u",  &dest_addr->sin_port, &dest_addr->sin_addr.s_addr);
	return 0;
}

/* funzione per ricevere una risposta dal server*/
int rcv_response(uint16_t* response_code) {
	int ret;
	ret = recv(tcp_socket, (void*)response_code, sizeof(uint16_t), 0);
  if(ret < 0){
    perror("Errore nel ricevere la risposta dal server");
    return -1;
  }
	*response_code = ntohs(*response_code);
	return 0;
}

/* funzione che prova a ricevere un messaggio udp.
legge dal socket udp e se arriva un messaggio effettivamente allora legge dal buffer.
La forma del buffer è descritta più in fondo*/
int rcv_udp_msg(char* msgbuf) {
	uint32_t addrlen;
	int ret;
	struct sockaddr_in connecting_addr;
	ret = recvfrom(udp_socket, msgbuf, MAX_LENGTH_MESSAGE + MAX_LENGTH_USERNAME+ strlen(" (Messaggio)>\n") + 1,
																	0,	(struct sockaddr*)&connecting_addr, &addrlen);
	if(ret < 0)
      return -1;
	return ret;
}

/* ottiene la lista degli utenti e al mostra*/
int rcv_users() {
	uint32_t list_len, netlist_len;
	char* buf;
	int ret;
	ret = recv(tcp_socket, (void*)&netlist_len, sizeof(uint32_t), 0);
  if(ret < 0){
    perror("Errore nel ricevere la lunghezza della lista");
    return -1;
  }
	list_len = ntohl(netlist_len);
	buf = (char*)malloc(list_len);
	ret = recv(tcp_socket, (void*)buf, list_len, 0);
  if(ret < 0){
    perror("Errore nel ricevere la lista");
    return -1;
  }
	printf("Clients registrati:\n%s\n", buf);
	free(buf);
	return 0;
}


/* Funzione per mandare ip e porta al server.*/
int send_addr() {
	int ret;
	uint16_t netmp;
	netmp = htons(my_port);
	ret = send(tcp_socket, (void*)&my_ip, sizeof(struct in_addr), 0);
  if(ret < 0){
    perror("Errore nel mandare l'ip");
    return -1;
  }
	ret = send(tcp_socket, (void*)&netmp, sizeof(uint16_t), 0);
  if(ret < 0){
    perror("Errore nel mandare la porta locale");
    return -1;
  }
	return 0;
}

/* funzione che manda solo il comando */
int send_cmd(uint16_t cmd_code){
  int ret;
  cmd_code = htons(cmd_code);
  ret = send(tcp_socket, (void*)&cmd_code, sizeof(uint16_t), 0);
  if(ret < 0){
    perror("Errore nel mandare il comando");
    return -1;
  }
	return 0;
}


/* funzione che manda comando e argomento*/
int send_cmd_arg(uint16_t cmd_code, char* username) {
	int ret;
	uint16_t usrlen, netusrlen;
	char* buf;
	cmd_code = htons(cmd_code);
	ret = send(tcp_socket, (void*)&cmd_code, sizeof(uint16_t), 0);
  if(ret < 0){
    perror("Errore nel mandare il comando");
    return -1;
  }
	usrlen = strlen(username);
	netusrlen = htons(usrlen + 1);
	buf = (char*)malloc(usrlen + 1);
	strcpy(buf, username);
	ret = send(tcp_socket, (void*)&netusrlen, sizeof(uint16_t), 0);
  if(ret < 0){
    perror("Errore nel mandare la dimensione dell'argomento");
    return -1;
  }
	ret = send(tcp_socket, (void*)buf, usrlen + 1, 0);
  if(ret < 0){
    perror("Errore nel mandare l'argomento del comando");
    return -1;
  }
	free(buf);
	return 0;
}

/* funzione per mandare un messaggio al server*/
//si manda prima la dimensione del messaggio e poi il messaggio list_message_node
//l'user è implicito in quanto è dipendente dalla connessione
int send_tcp_msg(char* msg, uint16_t len) {
	int ret;
	uint16_t netlen;
	netlen = htons(len);
	ret = send(tcp_socket, (void*)&netlen, sizeof(uint16_t), 0);
  if(ret < 0){
    perror("Errore nel mandare la dimensione del messaggio");
    return -1;
  }
	ret = send(tcp_socket, (void*)msg, len, 0);
  if(ret < 0){
    perror("Errore nel mandare il messaggio");
    return -1;
  }
	return 0;
}

/*Per mandare un messaggio istantaneo a un altro client mando due stringhe di lunghezza fissa,
quella con l'user di provenienza e quella con il messaggio di dimensioni prefissate
Il messaggio mandato è formato in maniera fissata. La prima porzione del buffer(di dimensione MAX_LENGTH_USERNAME)
contiene l'username mentre la seconda prozione, di dimensione MAX_LENGTH_MESSAGE, contiene il Messaggio*/
int send_udp_msg(char* body, struct sockaddr_in dest_addr) {
	int ret;
	char sendbuf[MAX_LENGTH_MESSAGE + MAX_LENGTH_USERNAME + strlen(" (Messaggio)>\n")];
	sendbuf[0]= '\0';
	strcpy(sendbuf, current_user);
	strcat(sendbuf," (Messaggio)>\n");
	strcat(sendbuf, body);
	ret = sendto(udp_socket, sendbuf, MAX_LENGTH_MESSAGE + MAX_LENGTH_USERNAME + strlen(" (Messaggio)>\n") + 1,
	 																	0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
	if(ret < 0){
		perror("Errore nella sendto");
		return -1;
	}
	return 0;
}
