#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>  
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define EMPTY_SOCKET (-1)
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

const char WELCOME[] = "OK START Benvenuto\n";
const char ERROR[] = "ERR ";
const char *COMMANDS[] = {"START", "TEXT", "HIST", "QUIT", "EXIT"};
unsigned char TEXT[4096];


void text_assert(unsigned char *src)
{
    int length = strlen(src);
    int index = 0;
    for(int i = 0; i < length; i++)
        if(i > 3 && (isalpha(src[i]) || ispunct(src[i]) || src[i] == ' '))
            TEXT[index++] = src[i];
        
}

void * histogram()
{
    unsigned int *count[52]; 
    if(TEXT == NULL)
        abort();
    else
    {
        int i = 0;
        while (TEXT[i] != '\0') 
        {
            if (TEXT[i] >= 'a' && TEXT[i] <= 'z' || TEXT[i] >= 'A' && TEXT[i] <= 'Z') 
            {
                int x = TEXT[i] - 'a';
                count[x]++;
            }
            i++;
        }
    }
    return count;
} 

int main(int argc, char *argv[])
{
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;

    if (argc != 2) 
    {
        fprintf(stderr, ANSI_COLOR_RED "Usage: %s <port>\n" ANSI_COLOR_RESET, argv[0]);
        exit(1);
    }

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1) 
    {
        fprintf(stderr, ANSI_COLOR_RED "Could not create a socket!\n" ANSI_COLOR_RESET);
        exit(1);
    }
    else 
    {
	    fprintf(stderr, ANSI_COLOR_GREEN "Socket created!\n" ANSI_COLOR_RESET);
    }
    
    /* retrieve the port number for listening */
    simplePort = atoi(argv[1]);

    /* setup the address structure */
    /* use INADDR_ANY to bind to all local addresses  */
    memset(&simpleServer, '\0', sizeof(simpleServer)); 
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
    simpleServer.sin_port = htons(simplePort);

    /*  bind to the address and port with our socket  */
    returnStatus = bind(simpleSocket,(struct sockaddr *)&simpleServer,sizeof(simpleServer));

    if (returnStatus == 0) 
    {
	    fprintf(stderr, ANSI_COLOR_GREEN "Bind completed!\n");
        fprintf(stderr, "Currently listening on %s:%d\n" ANSI_COLOR_RESET, inet_ntoa(simpleServer.sin_addr), simplePort);
    }
    else 
    {
        fprintf(stderr, ANSI_COLOR_RED "Could not bind to address! Status: %d\n" ANSI_COLOR_RESET, returnStatus);
        close(simpleSocket);
        exit(1);
    }

    /* lets listen on the socket for connections      */
    returnStatus = listen(simpleSocket, 5);

    if (returnStatus == -1)
    {
        fprintf(stderr,  ANSI_COLOR_RED "Cannot listen on socket!\n" ANSI_COLOR_RESET);
        close(simpleSocket);
        exit(1);
    }

    //Server cycle

    while (1)
    {

        struct sockaddr_in clientName = { 0 };
        int simpleChildSocket = 0;
        int clientNameLength = sizeof(clientName);
        char buffer[512] = "";
        bzero(buffer, sizeof(buffer));

        //wait here
        simpleChildSocket = accept(simpleSocket, (struct sockaddr*)&clientName, &clientNameLength);

        if (simpleChildSocket == EMPTY_SOCKET)
        {
            fprintf(stderr, ANSI_COLOR_RED "Cannot accept connections!\n" ANSI_COLOR_RESET);
            close(simpleSocket);
            exit(1);
        }
		else
        {
            fprintf(stdout, ANSI_COLOR_GREEN "\nClient connected with address %s\n" ANSI_COLOR_RESET, inet_ntoa(clientName.sin_addr));
            write(simpleChildSocket, WELCOME, strlen(WELCOME));
            
            returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));
            if(strstr(buffer, COMMANDS[1]) != NULL)
                text_assert(buffer);
            else if(strstr(buffer, COMMANDS[2]) != NULL)
                histogram();
            else if(strstr(buffer, COMMANDS[4]) != NULL)
            {
                histogram();
                close(simpleChildSocket);
            }
            else if(strstr(buffer, COMMANDS[3]) != NULL)
            {
                write(simpleChildSocket, "Closing connection\n", sizeof("Closing connection\n"));
                close(simpleChildSocket);
            }
            else if(buffer != NULL)
            {
                write(simpleChildSocket, ERROR, sizeof(ERROR));
            }
            bzero(buffer, sizeof(buffer));
        
            fprintf(stdout, "\nClient disconnected\n");
        }
    }

    close(simpleSocket);
    return 0;
}