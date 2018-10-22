#include "server.h"

list_chat_user lcu;
pthread_mutex_t chat_user_list_mutex;

void *handler_connection(void *sock){
	int ret;
	int socket = *((int *)sock);
	printf("Nuovo host connesso, socket = %d.\n", socket);
	while(1){
		ret = get_command(socket);
		if (ret == -2) {
			close_sock(socket);
			pthread_exit(NULL);
		}
	}
}


int main(int argc, char* argv[]) {
	pthread_t ted;
	int newfd;			//new socket
	int listener;		//listening socket
	uint16_t sv_port;	//input port
	int ret,  addrlen;
	struct sockaddr_in sv_addr, cl_addr;	//server client addresses

	sv_port = parse_port(argc, argv);

	lcu = NULL;
	listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener < 0){
		perror("Errore nella sock");
		exit(1);
	}
	memset(&sv_addr, 0, sizeof(sv_addr));
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_addr.s_addr = INADDR_ANY;
	sv_addr.sin_port = htons(sv_port);

	//bind
	ret = bind(listener, (struct sockaddr*)& sv_addr, sizeof(sv_addr));
	if (ret < 0) {
        perror("bind fallita");
        exit(1);
    }
	//listen
	ret = listen(listener, BACKLOG_SIZE);
	if (ret < 0) {
        perror("listen fallita");
        exit(1);
    }
	printf("Creato listening socket. Porta = %hu, dimensione coda di backlog = %d\n", sv_port, BACKLOG_SIZE);
	pthread_mutex_init(&chat_user_list_mutex, NULL);
	while(1) {
		addrlen = sizeof(cl_addr);
		newfd = accept(listener, (struct sockaddr *)&cl_addr, (unsigned int*)&addrlen);
		if(newfd < 0){
				perror("errore nell'accept()");
		}
		pthread_create(&ted,NULL,handler_connection,(void*)&newfd );
	}
	close(listener);
	return 0;
}
