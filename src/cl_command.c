#include "client.h"

 void parse_arguments(int argc, char* argv[], struct in_addr* ma, uint16_t* mp, struct in_addr* sa, uint16_t* sp) {
 	long parsed_value;
 	if (argc != 5) {
 		printf("La sintassi è: <program> <local_IP> <local_port> <server_IP> <server_port>\n");
 		exit(1);
 	}
 	parsed_value = strtol(argv[2], NULL, 10);
 	if (!parsed_value) {
 		printf("Bad argument (local_port).\n");
 		exit(1);
 	}
 	if (parsed_value < 1 || parsed_value > 65535) {
 		printf("Local port number must be in range [1, 65535].\n");
 		exit(1);
 	}
 	*mp = (unsigned short)parsed_value;
 	parsed_value = strtol(argv[4], NULL, 10);
 	if (!parsed_value) {
 		printf("Bad argument (server_port).\n");
 		exit(1);
 	}
 	if (parsed_value < 1 || parsed_value > 65535) {
 		printf("Server port number must be in range [1, 65535].\n");
 		exit(1);
 	}
 	*sp = (unsigned short)parsed_value;
 	if (!inet_pton(AF_INET, argv[1], ma)) {
 		printf("Invalid local IP address format.\n");
 		exit(1);
 	}
 	if (!inet_pton(AF_INET, argv[3], sa)) {
 		printf("Invalid IP address format for server.\n");
 		exit(1);
 	}
 }


void showconnection(struct in_addr sa, int sp, int mp) {
	char ipbuf[15];
	inet_ntop(AF_INET, &sa, ipbuf, 15);
	printf("Connessione avvenuta con successo al server %s (porta %d).\n"
			"RRicevo instant messaging alla porta %d\n\n",
			ipbuf, sp, mp);
}

int read_execute_command(){
	char command[MAX_LENGTH_COMMAND];
	scanf("%s", command);
	if(!strcmp(command,"!help")){
		showcommands();
		return 0;
	}
	else if(!strcmp(command,"!register")){
		return _register();
	}
	else if(!strcmp(command,"!deregister")){
		return _deregister();
	}
	else if(!strcmp(command,"!who")){
		return _who();
	}
	else if(!strcmp(command,"!send")){
		return _send();
	}
	else if(!strcmp(command,"!quit")){
		return _quit();
	}else{
    printf("Comando inesistente\n");
    return -1;
  }
	return 0;
}

void showcommands(){
	printf("\n\nsono disponibili i seguenti comandi:\n");
	printf("!help -> mostra i comandi\n");
	printf("!register username --> registra il client presso il server\n");
	printf("!deregister --> deregistra il client presso il server\n");
	printf("!who --> mostra l'elenco degli utenti disponibili\n");
	printf("!send username --> invia un messaggio ad un altro utente\n");
	printf("!quit --> disconnette il client dal server ed esce\n\n");

}

//funzione che registra il client:
//1. legge username
//2. chiede connessione al server che verifica se esisteva già e se è in uso
//3. Se tutto va a buon fine setta logged e salva l'user nella variabile globale current_user
int _register(){
	int ret = 0;
	uint16_t response;
	char username[MAX_LENGTH_USERNAME];
	if(logged){
		printf("Attenzione client già registrato\n");
		return -1;
	}
	scanf("%s", username);
	ret = send_cmd_arg(CMD_REGISTER,username);
	if(ret < 0){
		return -1;
	}
	ret = rcv_response(&response);
	if(ret < 0){
		return -1;
	}else if(response == REG_DOUBLE_LOGIN){//se il login è gia avvenuto in un altro client
		printf("Attenzione, l'user %s è già connesso in unaltro client\n",username );
		return -1;
	}else if(response == REG_NEW_USER){//se è un nuovo user
		ret = send_addr();
		if(ret < 0){
			printf("Errore nel mandare il proprio indirizzo al server\n");
			return -1;
		}
		printf("Benvenuto %s, ora sei connesso\n",username );
		ret = 0;
	}else if(response == REG_RECONNECTED){// se l'user esisteva già->provoa  a scaricare i messaggi
	ret = send_addr();
	if(ret < 0){
		printf("Errore nel mandare il proprio indirizzo al server\n");
		return -1;
	}
	printf("Welcome back %s!\n", username);
	ret = download_offline_msg();
	if(ret < 0){
		printf("Errore nello scaricare i messaggi offline\n");
		return -1;
	}
	ret = 0;
}else{
	printf("Errore non identificato\n");
	return -1;
}
if (ret == 0) {		//if no error occurred, login the user
		current_user = (char*)malloc(strlen(username) + 1);
		strcpy(current_user, username);
		logged = true;
	}
	return ret;
}


//funzione per deregistrare il client
//1. Se è effettivamente loggato chiede al server di deregistrare il client
//2. il server risponde con uno stato
int _deregister(){
	int ret = 0;
	uint16_t response;
	if(!logged){
		printf("Attenzione client non registrato, non ci si può deregistrare\n");
		return -1;
	}
	ret = send_cmd(CMD_DEREGISTER);
	if(ret < 0){
		printf("errore nella richiesta di deregistrazione\n");
		return -1;
	}
	ret = rcv_response(&response);
	if(ret < 0){
		printf("Errore nel ricevere la risposta\n");
	}
	if(response == DEREG_DONE){
		logged = false;
		free(current_user);
		return ret;
	}else if(response == DEREG_FAILED){
		printf("Attenzione, deregister fallita\n");
		return -1;
	}else{
		printf("Attenzione, risposta insensata\n");
		return -1;
	}
}


//funzione che disconnette il client
int _quit(){
	int ret;
	ret = send_cmd(CMD_QUIT);
	if(ret < 0){
		printf("Errore nel segnalare la disconnessione al server\n");
	}
	return -2;
}


//funzione che mostra gli user online.
//1. Manda al server il comando
//2. Legge la risposta
int _who(){
	uint16_t response;
	int ret;
	if(!logged){
		printf("impossibile chiamare la !who se non si è connessi");
		return -1;
	}
	ret = send_cmd(CMD_WHO);
	if(ret < 0){
		printf("errore nella richiesta degli utenti online\n");
		return -1;
	}
	ret = rcv_response(&response);
	if(ret < 0){
		printf("Errore nel ricevere la risposta\n");
		return -1;
	}
	if(response == WHO_EMPTY){
		printf("Nessun utente online\n\n");
		return 0;
	}else if(WHO_READY){
		ret = rcv_users();
		if(ret < 0){
			printf("Errore nel ricevere la lista degli utenti online");
			return -1;
		}
		return 0;
	}else{
		printf("Risposta inaspettata\n");
		return -1;
	}
}


//funzione per mandare un messaggio:
//1. chiede al server di mandare un messaggio all'user destination_user
//2. legge la risposta del server e scopre se mandarlo a lui via tcp o all'altro clinet via udp
//3. legge il messagggio e lo manda nel modo stabilito
int _send(){
	char destination_user[MAX_LENGTH_USERNAME];
	char msg[MAX_LENGTH_MESSAGE];
	int msglen = 0;
	char new;
    struct sockaddr_in destination_addr;
	uint16_t response;
	int ret = 0;
	if(!logged){
		printf("Attenzione client non registrato, non si possono mandare messaggi\n");
		return -1;
	}
	scanf("%s", destination_user);
	//connessioni
	//mando al server comando e user
	ret = send_cmd_arg(CMD_SEND, destination_user);
	if(ret < 0){
		printf("Errore nel mandare il comando al server\n");
		return -1;
	}
	ret = rcv_response(&response);
	if(ret < 0){
		printf("Errore nel ricevere la risposta\n");
		return -1;
	}
  // 3 casi: user inesistente, user ofline->tcp user online ->udp
	if(response == SEND_NONEXIST_USER){
    printf("Attenzione, l'user %s è inesistente\n\n", destination_user);
    return -1;
  }else{
    //lettura messaggio
    printf("\nScrivere il messaggio per %s\n\n", destination_user);
      while(1){
        new = getchar();
        if((msglen>0 && msg[msglen-1] == '\n' && new == '.')||(msglen >= MAX_LENGTH_MESSAGE -2)){
          break;
        }
        msg[msglen] = new;
        msglen++;
      }
      msg[msglen] = '\0';
      if(msglen >= MAX_LENGTH_MESSAGE - 2){
        printf("Attenzione, messaggio troppo lungo, è stato troncato\n");
      }
      //invio messaggio
      if(response == SEND_ONLINE_USER){
        ret = rcv_destination_addres(&destination_addr);
        if(ret < 0){
          printf("Errore nel ricvere l'indirizzo del destinatario");
          return -1;
        }
        destination_addr.sin_family = AF_INET;
        ret = send_udp_msg(msg, destination_addr);
  			if(ret < 0){
          perror("Errore, impossibile mandare il messaggio");
          return -1;
        }else{
          printf("Messaggio mandato!\n\n");
        }
  			return ret;
      }else if(response == SEND_OFFLINE_USER){
          ret = send_tcp_msg(msg, msglen);
          if (ret < 0) {
            printf("Errore nel mandare il messaggio\n\n");
            return -1;
          }
          return 0;
      }else{
        printf("Risposta insapettata dal server, messaggio non mandato");
        return -1;
      }
  }
}
