#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define CLOSE_ERROR -1
#define CONTINUE 0
#define CLOSE_NO_ERROR 1

#define MAX_DIM 256

typedef char *string;

enum commands_enum
{
    OK,
    END,
    ERR,
    QUIT,
    WORD,
    PERFECT
};

typedef enum commands_enum commands;

const string menu_string = "Menu:\n\n1- Guess the word\n2- Exit\nMake your choice:";

void process_message_attempts(string result, int attempts, string str, commands command)
{
    int len = 0;
    switch (command)
    {
    case OK:
        len = strlen(str);
        if (len <= 6)
            sprintf(result, "Word not guessed. Attempts: %d - Guessed letters: %s", attempts, str);
        else
            sprintf(result, "%s\nTotal attempts: %d\n", str, attempts);
        break;
    case END:
        sprintf(result, "No attempts remaining. Word was: %s\n", str);
        break;
    case ERR:
        sprintf(result, "Error received: %s", str);
        break;
    case QUIT:
    case PERFECT:
        strcpy(result, str);
        break;

    default:
        strcpy(result, "Invalid command.\n");
        break;
    }
}

commands retrieve_message(string result, string str)
{
    char temp[MAX_DIM] = "";
    commands command;
    if (strstr(str, "OK"))
    {
        command = OK;
        char ok[2];
        int t = 0;
        sscanf(str, "%s %d %[^\n]", ok, &t, temp);
        process_message_attempts(result, t, temp, command);
        if (strstr(str, "PERFECT"))
        {
            command = PERFECT;
            process_message_attempts(result, t, "You guessed the word!!\n", command);
        }
    }
    else if (strstr(str, "QUIT"))
    {
        char quit[4];
        command = QUIT;
        sscanf(str, "%s %[^\n]", quit, temp);
        process_message_attempts(result, 0, temp, command);
    }
    else if (strstr(str, "ERR"))
    {
        char err[3];
        command = ERR;
        sscanf(str, "%s %[^\n]", err, temp);
        process_message_attempts(result, 0, temp, command);
    }
    else if (strstr(str, "END"))
    {
        char end[3];
        int t = 0;
        command = END;
        sscanf(str, "%s %d %s", end, &t, temp);
        process_message_attempts(result, t, temp, command);
    }
    return command;
}

int main(int argc, char *argv[])
{

    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    char buffer[MAX_DIM] = "";
    struct sockaddr_in simpleServer;

    if (3 != argc)
    {

        fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
        exit(1);
    }

    /* create a streaming socket      */
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

    /* retrieve the port number for connecting */
    simplePort = atoi(argv[2]);

    /* setup the address structure */
    /* use the IP address sent as an argument for the server address  */
    // bzero(&simpleServer, sizeof(simpleServer));
    memset(&simpleServer, '\0', sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    // inet_addr(argv[2], &simpleServer.sin_addr.s_addr);
    simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
    simpleServer.sin_port = htons(simplePort);

    /*  connect to the address and port with our socket  */
    returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

    if (returnStatus == 0)
    {
        fprintf(stderr, "Connect successful!\n");
    }
    else
    {
        fprintf(stderr, "Could not connect to address!\n");
        close(simpleSocket);
        exit(1);
    }

    //-------------------------------------------------------
    char clientCommand[MAX_DIM] = "";
    char serverResult[MAX_DIM] = "";
    int closeFlag = 0;

    bzero(serverResult, sizeof(serverResult));
    bzero(buffer, sizeof(buffer));
    returnStatus = read(simpleSocket, buffer, sizeof(buffer));

    if (returnStatus > 0)
    {
        retrieve_message(serverResult, buffer);
        printf("%s\n", serverResult);
    }

    while (!closeFlag)
    {
        commands command;
        char word[10] = "";
        char string_choice[MAX_DIM] = "";
        int choice = 0;
        bzero(clientCommand, MAX_DIM);
        bzero(word, sizeof(word));
        do
        {
            printf("%s ", menu_string);
            choice = 0;
            scanf("%s", string_choice);
            if (strlen(string_choice) == 1)
                choice = atoi(string_choice);
        } while (choice != 1 && choice != 2);

        switch (choice)
        {
        case 1:
            printf("Guess the word <- selected\nInsert a 5-letter long word: ");
            scanf("%s", word);
            sprintf(clientCommand, "WORD %s\n", word);
            break;
        case 2:
            printf("Exit <- selected\n");
            strcpy(clientCommand, "QUIT\n");
        default:
            break;
        }
        write(simpleSocket, clientCommand, strlen(clientCommand));

        bzero(buffer, MAX_DIM);
        returnStatus = read(simpleSocket, buffer, MAX_DIM);

        if (returnStatus > 0)
        {
            bzero(serverResult, MAX_DIM);
            command = retrieve_message(serverResult, buffer);
            switch (command)
            {
            case QUIT:
            case END:
            case PERFECT:
                closeFlag = CLOSE_NO_ERROR;
                break;
            case OK:
                closeFlag = CONTINUE;
                break;
            case ERR:
                closeFlag = CLOSE_ERROR;
                break;
            default:
                closeFlag = CLOSE_ERROR;
                sprintf(serverResult, "Malformed message from server");
                break;
            }
            printf("%s\n", serverResult);
        }
    }
    close(simpleSocket);
    return 0;
}
