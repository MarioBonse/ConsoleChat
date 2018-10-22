#include "client.h"

int tcp_socket;
int udp_socket;
uint16_t my_port;
struct in_addr my_ip;
bool logged = false;
char* current_user = NULL;


//Codice del thread che controlla l'eventuale presenza di messaggi udp.
void* recv_msg(void* arg) {
    int ret;
    char msg[MAX_LENGTH_USERNAME + MAX_LENGTH_MESSAGE + strlen(" (Messaggio)>\n\n") +1];
	sleep(1);
	while(1) {
    ret = rcv_udp_msg(msg);
    if(ret >= 0){//ho effettivamente ricevuto un messaggio
      printf("\nNuovo messaggio");
      printf("\n%s",msg);
      printf("%s>", current_user);
      fflush(stdout);
    }
	}
	pthread_exit(NULL);
}


int main(int argc, char* argv[]){
    pthread_t recv_routine_thr;	//thread che riceve i messaggi udp in background
    struct sockaddr_in server_addr, client_addres;
    uint16_t server_port; 
    struct in_addr server_ip;
    int ret;
    printf("\n");
    parse_arguments(argc, argv, &my_ip, &my_port, &server_ip, &server_port);
    //Inizializate tcp socket
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(tcp_socket<0){
            perror("socket fallita");
            exit(1);
    }
    memset(&server_addr, 0, sizeof(server_addr)); // Pulizia
    server_addr.sin_family = AF_INET ;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr = server_ip;
    //connection
    ret = connect(tcp_socket,(struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret<0){
            perror("Connection fallita");
            exit(1);
    }
    //inizializate udp socket
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_socket<0){
            perror("UDP socket creation fallita");
            exit(1);
    }
    memset(&client_addres,0,sizeof(client_addres));
    client_addres.sin_family = AF_INET;
    client_addres.sin_port = htons(my_port);
    client_addres.sin_addr = my_ip;
    ret = bind(udp_socket, (struct sockaddr*)&client_addres, sizeof(client_addres));
    if(ret<0){
          perror("Bind fallita");
          exit(1);
    }
    showconnection(server_ip, server_port, my_port);
    showcommands();
	  ret = pthread_create(&recv_routine_thr, NULL, recv_msg, NULL);
    if(ret < 0){
      perror("Errore nell'iniziare la routine che legge i messaggi udp");
      exit(1);
    }
    while(1){
        int exit;
        if(logged){
            printf("%s>", current_user);
        }else{
            printf(">");
        }

        exit = read_execute_command();
        if(exit == -2){
            break;
        }
    }
    close(udp_socket);
    close(tcp_socket);
    printf("Disconnected from server");
    return 0;
}
