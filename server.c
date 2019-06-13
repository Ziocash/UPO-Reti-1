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

const char WELCOME[] = "OK START Benvenuto\n";
const char ERROR[] = "ERR ";
const char *COMMANDS[] = {"START", "TEXT", "HIST", "QUIT", "EXIT"};
unsigned char TEXT[] = "string";


int text_assert(unsigned char *src)
{
    if(memcpy(TEXT, src, sizeof(src)) != NULL)
        return 1;
    else
        return 0;
        
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
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1) 
    {
        fprintf(stderr, "Could not create a socket!\n");
        exit(1);
    }
    else 
    {
	    fprintf(stderr, "Socket created!\n");
    }
    
    /* retrieve the port number for listening */
    simplePort = atoi(argv[1]);
    //fprintf(stderr, &simplePort);

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
        exit(1);
    }

    /* lets listen on the socket for connections      */
    returnStatus = listen(simpleSocket, 5);

    if (returnStatus == -1)
    {
        fprintf(stderr, "Cannot listen on socket!\n");
        close(simpleSocket);
        exit(1);
    }

    while (1)
    {

        struct sockaddr_in clientName = { 0 };
        int simpleChildSocket = 0;
        int clientNameLength = sizeof(clientName);
        char buffer[256] = "";

        //wait here
        simpleChildSocket = accept(simpleSocket, (struct sockaddr*)&clientName, &clientNameLength);

        if (simpleChildSocket == -1)
        {
            fprintf(stderr, "Cannot accept connections!\n");
            close(simpleSocket);
            exit(1);
        }
		else
        {
            fprintf(stdout, "\nClient connected\n");
            write(simpleChildSocket, WELCOME, strlen(WELCOME));
            while(simpleChildSocket != EMPTY_SOCKET)
            {
                while ((returnStatus = read(simpleSocket, buffer, sizeof(buffer))) >= 0)
                {
                    fprintf(stderr, "Internal error %s\n", buffer);
                }
                
                if(strstr(buffer, COMMANDS[1]) != NULL)
                    text_assert(buffer);
                else if(strstr(buffer, COMMANDS[2]) != NULL)
                    histogram();
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
            }
            fprintf(stdout, "\nClient disconnected\n");
        }
    }

    close(simpleSocket);
    return 0;
}