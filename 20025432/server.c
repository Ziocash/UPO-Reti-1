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

#define EMPTY_SOCKET (-1)
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

const char WELCOME[] = "OK START Benvenuto\n";
const char ERROR[] = "ERR SYNTAX Comando non corretto\n";
const char *COMMANDS[] = {"START", "TEXT", "HIST", "QUIT", "EXIT"};
const char HISTEND[] = "OK HIST END\n";
const char EXITEND[] = "OK EXIT Sessione terminata\n";
unsigned char TEXT[4096];


void text_assert(unsigned char *src)
{
    int length = strlen(src);
    int index = 0;
    if(strlen(TEXT) != 0)
        index = strlen(TEXT);

    for(int i = 0; i < length; i++)
        if(i > 4 && (isalpha(src[i]) || ispunct(src[i]) || src[i] == ' '))
            TEXT[index++] = src[i];
}

int count_string(char str[])
{
    int counter = 0;
    for(int i = 0; i < strlen(str); i++)
        if(isalnum(str[i]) || ispunct(str[i]))
            counter++;
    return counter;
}

void clear()
{
    for(int i = 0; i < sizeof(TEXT); i++)
        TEXT[i] = 0;
}

void histogram(char out[])
{
    char value[512];
    char freq[26];
    bzero(value, sizeof(value));
    bzero(freq, sizeof(freq)); 
    if(TEXT == NULL)
        return;
    else
    {
        for(int i = 0; i < strlen(TEXT); i++)
            if(isalnum(TEXT[i]))
                freq[TEXT[i] - 'a']++;
        
        // traverse 'str' from left to right 
        for (int i = 0; i < strlen(TEXT); i++) 
        { 
            if (freq[TEXT[i] - 'a'] != 0) 
            {
                char appo[60];
                bzero(appo, sizeof(appo));
                sprintf(appo, "%c", TEXT[i]);
                strcat(value, appo);
                strcat(value, ":");
                bzero(appo, sizeof(appo));
                sprintf(appo, "%d", freq[TEXT[i] - 'a']);
                strcat(value, appo); 
                strcat(value, " ");
                freq[TEXT[i] - 'a'] = 0;
            } 
        } 
    }
    strcat(value, "\n");
    strncpy(out, value, strlen(value));
} 

int string_length(char *buffer)
{
    int size = 0;
    int bufflen = strlen(buffer);
    char *ptr = buffer;
    while(*ptr++ != '\n')
    {
        if(strlen(ptr) <= 4 && isdigit(*ptr))
        {
            size = atoi(ptr);
            break;
        }
    }
    return size;
}

int compare_length(int length, char *buffer)
{
    char string[strlen(buffer)];
    bzero(string, sizeof(string));
    int index = 0;
    int offset = 0;
    if(strlen(buffer) >= 100)
        offset = 4;
    else
        offset = 3;
    for(int i = 5; i < strlen(buffer) - offset; i++)
        string[index++] = buffer[i];

    int count = count_string(string);
    if(count == length)
        return 1;
    return 0;
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
            int flag = 1;
            char string;
            do
            {
                bzero(buffer, sizeof(buffer));
                returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));
                if(strstr(buffer, COMMANDS[1]) != NULL)
                {
                    int len = string_length(buffer);
                    if(compare_length(len, buffer))
                    {
                        char value[512];
                        bzero(value, sizeof(value));
                        strcat(value, "OK ");
                        strcat(value, COMMANDS[1]);
                        strcat(value, " ");
                        char len_s[10];
                        bzero(len_s, sizeof(len_s));
                        sprintf(len_s, "%d", len);
                        strcat(value, len_s);
                        strcat(value, "\n");
                        write(simpleChildSocket, value, strlen(value));
                        text_assert(buffer);
                    }
                    else
                    {
                        char value[] = "ERR TEXT Numero di caratteri non corretto\n";
                        write(simpleChildSocket, value, strlen(value));
                        flag = 0;
                        close(simpleChildSocket);
                    }                    
                }
                else if(strstr(buffer, COMMANDS[2]) != NULL)
                {
                    if(strlen(TEXT) > 0)
                    {
                        char str[512];
                        char message[512];
                        bzero(message, sizeof(message));
                        bzero(str, sizeof(str));
                        histogram(str);
                        if(strlen(str) <= 503)
                        {
                            strcat(message, "OK ");
                            strcat(message, COMMANDS[2]);
                            strcat(message, " ");
                            strcat(message, str);
                            strcat(message, "\n");
                        }

                        write(simpleChildSocket, message, strlen(message));
                        write(simpleChildSocket, HISTEND, strlen(HISTEND));
                    }
                    else
                    {
                        char message[] = "ERR TEXT Nessun testo inviato\n";
                        write(simpleChildSocket, message, strlen(message));
                        flag = 0;
                        close(simpleChildSocket);
                    }
                    
                }
                else if(strstr(buffer, COMMANDS[4]) != NULL)
                {
                    char str[512];
                    char message[512];
                    bzero(message, sizeof(message));
                    bzero(str, sizeof(str));
                    histogram(str);
                    if(strlen(str) <= 503)
                    {
                        strcat(message, "OK ");
                        strcat(message, COMMANDS[2]);
                        strcat(message, " ");
                        strcat(message, str);
                        strcat(message, "\n");
                    }

                    write(simpleChildSocket, message, strlen(message));
                    write(simpleChildSocket, HISTEND, strlen(HISTEND));
                    write(simpleChildSocket, EXITEND, strlen(EXITEND));
                    flag = 0;
                    close(simpleChildSocket);
                }
                else if(strstr(buffer, COMMANDS[3]) != NULL)
                {
                    char message[] = "OK QUIT Chiusura connessione\n";
                    write(simpleChildSocket, message, strlen(message));
                    flag = 0;
                    close(simpleChildSocket);
                }
                else if(buffer != NULL)
                {
                    write(simpleChildSocket, ERROR, sizeof(ERROR));
                    flag = 0;
                    close(simpleChildSocket);
                }
            } while(flag);

            fprintf(stdout, ANSI_COLOR_GREEN "\nClient disconnected\n" ANSI_COLOR_RESET);
            bzero(TEXT, sizeof(TEXT));
        }
    }
    close(simpleSocket);
    return 0;
}