#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>  
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MIN_WORDS_COUNT 1
#define DEFAULT_WORDS_COUNT 10
#define MAX_WORDS_COUNT 30

void usage(int argc, char *executable)
{
    if (argc < 2 || argc > 3) 
    {
        fprintf(stderr, "Argomenti a riga di comando: %s <porta> [<max_parole>]\n", executable);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;

    usage(argc, argv[0]);

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1) 
    {
        fprintf(stderr, "Could not create a socket!\n");
        exit(EXIT_FAILURE);
    }
    else 
    {
	    fprintf(stderr, "Socket created!\n");
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
	    fprintf(stderr, "Bind completed!\n");
        fprintf(stderr, "Currently listening on %s:%d\n", inet_ntoa(simpleServer.sin_addr), simplePort);
    }
    else 
    {
        fprintf(stderr, "Could not bind to address! Status: %d\n", returnStatus);
        close(simpleSocket);
        exit(EXIT_FAILURE);
    }

    /* lets listen on the socket for connections      */
    returnStatus = listen(simpleSocket, 10);

    if (returnStatus == -1)
    {
        fprintf(stderr,  "Cannot listen on socket!\n");
        close(simpleSocket);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        struct sockaddr_in clientName = { 0 };
        int simpleChildSocket = 0;
        int clientNameLength = sizeof(clientName);
        simpleChildSocket = accept(simpleSocket, (struct sockaddr*)&clientName, &clientNameLength);

        if (simpleChildSocket == -1)
        {
            fprintf(stderr, "Cannot accept connections!\n");
            close(simpleSocket);
            exit(1);
        }

        write(simpleChildSocket, "WELCOME", strlen("WELCOME"));
        char buffer[512];
        bzero(buffer, strlen(buffer));
        ssize_t readvalue = read(simpleChildSocket, buffer, strlen(buffer));
        if(readvalue != 0)
            fprintf(stderr, "%s\n", buffer);
    }
    

    close(simpleSocket);
    return EXIT_SUCCESS;
}