#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define VERSION "0.4.125"

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
    fprintf(stdout, "3- \n");
    fprintf(stdout, "4- \n");
}

void elapse_string(char str[])
{
    char delim[] = " ";
    char *ptr = strtok(str, delim);

	while(ptr != NULL)
	{
        if(strstr(ptr, "OK") != NULL || strstr(ptr, "ERR") != NULL)
		    fprintf(stdout, "'%s'\n", ptr);
		    ptr = strtok(NULL, delim);
	}
}

int main(int argc, char *argv[]) 
{

    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    char buffer[512] = "";
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
	    fprintf(stdout, "Connect successful! Server at %s:%d\n", inet_ntoa(simpleServer.sin_addr), simplePort);
        returnStatus = read(simpleSocket, buffer, sizeof(buffer));
    	print_menu();
        elapse_string(buffer);
        //fprintf(stdout, "%s\n", buffer);
        bzero(buffer, sizeof(buffer));
    }
    else 
    {
        fprintf(stdout, "Could not connect to address!\n");
        close(simpleSocket);
        exit(1);
    }

    int flag = 1;	
	do
    {
		char  message[512] = "";
        fprintf(stdout, "Command: ");
        fscanf(stdin, " %[^\n]s\n", message);
        message[strlen(message)] = '\n';

        if(strstr(message, "QUIT") != NULL)
        {
            flag = 0;
            write(simpleSocket, message, strlen(message));
        }
        else
		{
            write(simpleSocket, message, strlen(message));
        }
        bzero(buffer, sizeof(buffer));
        bzero(message, sizeof(message));
        //while(returnStatus > 0)
            returnStatus = read(simpleSocket, buffer, sizeof(buffer));
    	fprintf(stdout, "%s\n", buffer);

        
        //command_validation(message);

       

	}while (flag);

    close(simpleSocket);
    return 0;

}