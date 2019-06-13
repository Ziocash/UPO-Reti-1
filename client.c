#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define VERSION "0.4.12"

const char *COMMANDS[] = {"START", "TEXT", "HIST", "QUIT", "EXIT"};

void print_title()
{
    fprintf(stdout, "TEXT Analyzer - %s\n", VERSION);
}

void print_menu()
{
    fprintf(stdout, "Scegliere un'operazione da fare\n");
    fprintf(stdout, "Eseguire le operazioni in ordine: non è possibile analizzare un testo non fornito!\n");
    fprintf(stdout, "1- Inviare il testo con la relativa lunghezza (es.: \"ciao come stai\" 14)\n");
    fprintf(stdout, "2- Visualizzare la frequenza dei caratteri contenuti nella stringa\n");
    fprintf(stdout, "3- ");
    fprintf(stdout, "4- ");
}

int main(int argc, char *argv[]) 
{

    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    char buffer[256] = "";
    struct sockaddr_in simpleServer;

    print_title();

    if (argc != 3) 
    {
        fprintf(stdout, "Usage: %s <server> <port>\n", argv[0]);
        exit(1);
    }

    //create a streaming socket
    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1) 
    {
        fprintf(stdout, "Could not create a socket!\n");
        exit(1);
    }
    else 
    {
	    fprintf(stdout, "Socket created!\n");
    }

    //retrieve the port number for connecting
    simplePort = atoi(argv[2]);

    //setup the address structure
    //use the IP address sent as an argument for the server address
    bzero(&simpleServer, sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    //inet_addr(argv[2], &simpleServer.sin_addr.s_addr);
    simpleServer.sin_addr.s_addr=inet_addr(argv[1]);
    simpleServer.sin_port = htons(simplePort);

    //connect to the address and port with our socket
    returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

    if (returnStatus == 0) 
    {
	    fprintf(stdout, "Connect successful!\n");
        //write(simpleSocket, COMMANDS[0], sizeof(COMMANDS[0]));                
    }
    else 
    {
        fprintf(stdout, "Could not connect to address!\n");
        close(simpleSocket);
        exit(1);
    }

    bzero(buffer, sizeof(buffer));

    int flag = 1;	
	do
    {       
        //get the message from the server   
	    returnStatus = read(simpleSocket, buffer, sizeof(buffer));
    	printf("%s", buffer);
        bzero(buffer, sizeof(buffer));

		char  message[256] = "";
        fprintf(stdout, "Command: ");
        scanf(" %s", message); 
		write(simpleSocket, message, strlen(message));
        read(simpleSocket, buffer, sizeof(buffer));
		fprintf(stdout, "%s", buffer);

        
        //command_validation(message);

        if(strstr(message, "QUIT") != NULL)
            flag = 0;

        bzero(buffer, sizeof(buffer));

	}while (flag);

    close(simpleSocket);
    return 0;

}