#include "server.h"


int parse_port(int argc, char* argv[]) {
	long parsed_value;
	if (argc != 2) {
		printf("Numero non corretto di argomenti. La sintassi è <program> <sv_port>\n");
		exit(1);
	}
	parsed_value = strtol(argv[1], NULL, 10);
	if (!parsed_value) {
		printf("Porta server non valida.\n");
		exit(1);
	}
	if (parsed_value < 1 || parsed_value > 65535) {
		printf("La porta deve essere nel range [1, 65535].\n");
		exit(1);
	}
	return (uint16_t)parsed_value;
}


int do_command(int sock,uint16_t cmd,char* username){
	if(cmd == CMD_HELP){
		printf("Comando non valido\n");
		return -1;
	}else if(cmd == CMD_WHO){
		return _who(sock);
	}else if(cmd == CMD_QUIT){
		return _quit(sock);
	}else if(cmd == CMD_REGISTER){
		return _register(sock, username);
	}else if(cmd == CMD_SEND){
		return _send(sock, username);
	}else if(cmd == CMD_DEREGISTER){
		return _deregister(sock);
	}else{
		printf("Comando inesistente\n");
		return -1;
	}
}

void showcommand(int cmd){
	if(cmd == CMD_HELP){
		printf("help");
	}else if(cmd == CMD_WHO){
		printf("who");
	}else if(cmd == CMD_QUIT){
		printf("quit");
	}else if(cmd == CMD_REGISTER){
		printf("register");
	}else if(cmd == CMD_SEND){
		printf("send");
	}else if(cmd == CMD_DEREGISTER){
		printf("deregister");
	}
}

//funzione che legge il comando che arriva dal socket e manda in esecuzione la funzione che lo esegue
int get_command(int sock) {
	int ret;
	uint16_t cmd;
	char* username = NULL;
	ret = get_cmd(sock, &cmd, &username);
	if (ret == 0) {
		ret = do_command(sock, cmd, username);
		printf("Eseguito il comando '");
		showcommand(cmd);
		printf("' dal socket %d. Return value = %d\n", sock, ret);
	}
	if (username != NULL)
		free(username);
	return ret;
}





//crea un oggetto messaggio e lo aggiunge alla lista dei messaggi di dest
int store_message(char* sender, char* dest, char* body) {
	message* m;
	list_chat_user_node* user_node;
	list_message* lmp;
	pthread_mutex_lock(&chat_user_list_mutex);
	user_node = list_chat_user_find_by_name(lcu, dest);
	pthread_mutex_unlock(&chat_user_list_mutex);
	if (!user_node)
		return -1;
	lmp = &(user_node->pending_msg);
	m = new_message(sender, body, false);
	pthread_mutex_lock(&chat_user_list_mutex);
	list_message_add_head(lmp, &m);
	pthread_mutex_unlock(&chat_user_list_mutex);
	printf("Salvo messaggio ofline(%s -> %s)\n", sender, dest);
	return 0;
}

//Funzione per il gestire il comando register:
//L'utente che ha richiesto la registrazione può:
//essere già online(errore)
//essere o meno  un nuovo user
int _register(int sock, char* username) {
	int ret;
	printf("Register\n");
	uint16_t port;
	struct in_addr ip;
	list_chat_user_node* user_node;
	pthread_mutex_lock(&chat_user_list_mutex);
	user_node = list_chat_user_find_by_name(lcu, username);
	pthread_mutex_unlock(&chat_user_list_mutex);
	if (user_node != NULL && (user_node->online == true)) {//se esiste ed è online
		printf("User %s Ha provato a loggarsi due volte.\n", username);
		ret = send_response(sock, REG_DOUBLE_LOGIN);
		if(ret < 0){
			return -1;
		}
		return 0;
	} else if (user_node != NULL && (user_node->online == false)) {	//se esiste e non è online
		pthread_mutex_lock(&chat_user_list_mutex);
		user_node->online = true;
		user_node->sock = sock;
		pthread_mutex_unlock(&chat_user_list_mutex);
		printf("User %s riconnesso.\n", username);
		ret = send_response(sock, REG_RECONNECTED);
		if(ret < 0){
			return -1;
		}
		ret = recive_addr(sock, &ip, &port);
		if(ret < 0){
			return -1;
		}
		pthread_mutex_lock(&chat_user_list_mutex);
		user_node->full_address.sin_family = AF_INET;
		user_node->full_address.sin_port = htons(port);
		user_node->full_address.sin_addr = ip;
		pthread_mutex_unlock(&chat_user_list_mutex);
		ret = send_allmsgs(sock, user_node);
		if (ret < 0) {
			printf("Non sono riuscito a mandare messaggi offline all'user %s.\n", username);
			return -1;
		}
		printf("Messaggi offline mandati a %s dopo la riconnessione\n", username);
		return 0;
	} else {	//Nuovo user
		chat_user* ncu;
		ret = send_response(sock, REG_NEW_USER);
		if(ret < 0){
			return -1;
		}
		ret = recive_addr(sock, &ip, &port);
		if(ret < 0){
			return -1;
		}
		ncu = new_chat_user2(username, sock, ip, port);
		pthread_mutex_lock(&chat_user_list_mutex);
		list_chat_user_add_head(&lcu, &ncu);
		pthread_mutex_unlock(&chat_user_list_mutex);
		printf("Nuovo user registrato %s\n", username);
		return 0;
	}
}

//Funzione per gestire il deregister
int _deregister(int sock) {
	int ret;
	char* username = NULL;
	pthread_mutex_lock(&chat_user_list_mutex);
	list_chat_user_node* usernode = list_chat_user_find_by_socket(lcu, sock);
	pthread_mutex_unlock(&chat_user_list_mutex);
	if (usernode)
		username = usernode->username;
	if (!username) {
		printf("Impossibile deregistrare l'user con socket %d, non esiste\n", sock);
		ret = send_response(sock, DEREG_FAILED);
		return -1;
	}
	printf("User %s deregistrato.\n", username);
	pthread_mutex_lock(&chat_user_list_mutex);
	list_chat_user_remove(&lcu, username);
	pthread_mutex_unlock(&chat_user_list_mutex);
	ret = send_response(sock, DEREG_DONE);
	if(ret < 0){
		return -1;
	}
	return 0;
}


int _who(int sock) {
	int ret;
	if (!lcu) {
		ret = send_response(sock, WHO_EMPTY);
		if(ret < 0){
			return -1;
		}
		return 0;
	}
	ret = send_response(sock, WHO_READY);
	if(ret < 0){
		return -1;
	}
	ret = send_users_list(sock);
	if(ret < 0){
		return -1;
	}
	return 0;
}


int _send(int sock, char* dest_name) {
	int ret;
	list_chat_user_node* p;
	struct sockaddr_in full_addr;
	char* sender_name = NULL;

	list_chat_user_node* sendernode = list_chat_user_find_by_socket(lcu, sock);
	if (sendernode)
		sender_name = sendernode->username;

	p = list_chat_user_find_by_name(lcu, dest_name);
	if (p!=NULL && p->online == true) {	//if recipient is online
		ret = send_response(sock, SEND_ONLINE_USER);
		if(ret < 0){
			return -1;
		}
		full_addr = p->full_address;
		ret = send_addr(sock, full_addr);
		if(ret < 0){
			return -1;
		}
	} else if (p!=NULL && p->online == false) {	//if recipient is offline
		ret = send_response(sock, SEND_OFFLINE_USER);
		if(ret < 0){
			return -1;
		}
		return read_message(sock, sender_name, dest_name);
	}else {	//if recipient doesn't exist (p == NULL)
		ret = send_response(sock, SEND_NONEXIST_USER);
		if(ret < 0){
			return -1;
		}
	}
	return 0;
}


int _quit(int sock) {
	list_chat_user_node* p;
	p = list_chat_user_find_by_socket(lcu, sock);
	if (!p)
		printf("Un host si è disconnesso (sock = %d)\n", sock);
	else {
		pthread_mutex_lock(&chat_user_list_mutex);
		p->online = false;
		p->sock = -1;
		pthread_mutex_unlock(&chat_user_list_mutex);
		printf("User %s disconnesso\n", p->username);
	}
	return -2;
}
