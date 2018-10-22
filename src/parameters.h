#ifndef PARAMETERS_H_
#define PARAMETERS_H_


#define BACKLOG_SIZE 32
#define NUM_OF_COMMANDS 6
#define MAX_LENGTH_COMMAND 15
#define MAX_LENGTH_USERNAME 20
#define MAX_LENGTH_INPUT_STRING	(MAX_LENGTH_COMMAND + MAX_LENGTH_USERNAME + 1)
#define MAX_LENGTH_MESSAGE 1500
#define IN_MSG_BUFFER_SIZE 30
#define MAX_LENGHT_COMMAND_STRING 11

//comandi
#define CMD_HELP        0
#define CMD_REGISTER    1
#define CMD_DEREGISTER  2
#define CMD_WHO         3
#define CMD_SEND        4
#define CMD_QUIT        5
//risposte dal server
#define REG_DOUBLE_LOGIN    128
#define REG_NEW_USER        129
#define REG_RECONNECTED     130
#define WHO_EMPTY           131
#define WHO_READY           132
#define DEREG_FAILED        133
#define DEREG_DONE          134
#define SEND_NONEXIST_USER  135
#define SEND_ONLINE_USER    136
#define SEND_OFFLINE_USER   137

#endif
