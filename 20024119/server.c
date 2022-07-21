#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define CLOSE_ERROR -1
#define CONTINUE 0
#define CLOSE_NO_ERROR 1

typedef char *string;

enum commands_e
{
    QUIT,
    WORD
};

typedef enum commands_e commands;

const string WELCOME_MESSAGE = "Welcome to Wordle!\n";
const string ERROR_INCORRECT_MESSAGE = "ERR Incorrect format\n";
const string ERROR_INCORRECT_COMMAND_MESSAGE = "ERR Incorrect command\n";
const string ERROR_INCORRECT_WORD = "ERR Incorrect word format, must be 5 LETTERS long\n";
const string GUESSED_MESSAGE = "OK PERFECT\n";
const string values[] = {"tonno", "etere", "fagli", "rozzo", "stura"};

/*
Compare the given word
*/
int compare_word(string result, string compared_str, string user_str)
{
    for (int i = 0; i < 5; i++)
    {
        if (compared_str[i] == tolower(user_str[i]))
            strcat(result, "*");
        else if (strchr(compared_str, tolower(user_str[i])) != NULL)
            strcat(result, "+");
        else
            strcat(result, "-");
    }
    if (strcmp(result, "*****") == 0)
        return 1;
    else
        return 0;
}

// Check spaces to respect protocol
int check_spaces(string str)
{
    if (strstr(str, "  ") != NULL)
        return 0;
    else
        return 1;
}

// Check string if contains a command
commands check_command(string str)
{
    if (strstr(str, "WORD"))
        return WORD;
    else if (strstr(str, "QUIT\n"))
        return QUIT;
    else
        return -1;
}

/*
Check chars in request body
*/
int check_chars(string message)
{
    for (int i = 0; i < strlen(message) - 1; i++)
        if (!isalpha(message[i]))
            return 0;
    return 1;
}

/*
Check word in WORD command
*/
int check_word(string result, string message)
{
    if (strlen(message) != 11)
        return 0;
    for (int i = 5; i < strlen(message); i++)
    {
        char c[2] = "";
        c[0] = message[i];
        strcat(result, c);
    }
    if(!check_chars(result))
        return 0;
    if (strchr(result, ' ') != NULL)
        return 0;
    return 1;
}

int main(int argc, char *argv[])
{
    int attempts = 6;
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;

    if (argc != 2 && argc != 3)
    {
        fprintf(stderr, "Usage: %s <port> [<attempts>]\n", argv[0]);
        exit(1);
    }

    if (argc == 3)
    {
        if (atoi(argv[2]) >= 6 && atoi(argv[2]) <= 10)
            attempts = atoi(argv[2]);
        else
        {
            fprintf(stderr, "Usage: [<attempts>] value should be between 6 and 10\n");
            exit(1);
        }
    }

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1)
    {

        fprintf(stderr, "Could not create a socket!\n");
        perror("Socket creation");
        exit(1);
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
    returnStatus = bind(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

    if (returnStatus == 0)
    {
        fprintf(stderr, "Bind completed!\n");
    }
    else
    {
        fprintf(stderr, "Could not bind to address!\n");
        perror("Binding");
        close(simpleSocket);
        exit(1);
    }

    /* lets listen on the socket for connections      */
    returnStatus = listen(simpleSocket, 5);

    if (returnStatus == -1)
    {
        fprintf(stderr, "Cannot listen on socket!\n");
        perror("Listening");
        close(simpleSocket);
        exit(1);
    }

    while (1)
    {

        struct sockaddr_in clientName = {0};
        int simpleChildSocket = 0;
        int closeFlag = 0;
        int guessed = 0;
        uint clientNameLength = sizeof(clientName);

        fprintf(stdout, "Waiting for connection...\n");

        simpleChildSocket = accept(simpleSocket, (struct sockaddr *)&clientName, &clientNameLength);

        if (simpleChildSocket == -1)
        {
            fprintf(stderr, "Cannot accept connections!\n");
            perror("Accepting");
            close(simpleSocket);
            exit(1);
        }

        fprintf(stdout, "Client connected.\n");

        /* handle the new connection request  */
        /* write out our message to the client */
        char welcome[256] = "";
        string selectedWord = "";
        selectedWord = values[rand() % (sizeof(values) / sizeof(values[0]))];
        sprintf(welcome, "OK %d %s", attempts, WELCOME_MESSAGE);
        write(simpleChildSocket, welcome, strlen(welcome));

        int clientAttempts = 0;

        while (closeFlag == CONTINUE)
        {
            char buffer[256];
            bzero(buffer, sizeof(buffer));
            if (read(simpleChildSocket, buffer, sizeof(buffer)) > 0)
            {
                fprintf(stdout, "Client sends: %s", buffer);
                if (strchr(buffer, '\n') != NULL)
                {
                    if (!check_spaces(buffer))
                    {
                        write(simpleChildSocket, ERROR_INCORRECT_MESSAGE, strlen(ERROR_INCORRECT_MESSAGE));
                        closeFlag = CLOSE_ERROR;
                    }
                    else
                    {
                        char word[10];
                        char result[10] = "";
                        char message[256] = "";
                        bzero(word, sizeof(word));
                        switch (check_command(buffer))
                        {
                        case WORD:
                            if (check_word(word, buffer))
                            {
                                closeFlag = guessed = compare_word(result, selectedWord, word);
                                clientAttempts++;
                            }
                            else
                            {
                                write(simpleChildSocket, ERROR_INCORRECT_WORD, strlen(ERROR_INCORRECT_WORD));
                                closeFlag = CLOSE_ERROR;
                            }
                            break;
                        case QUIT:
                            sprintf(message, "QUIT You're leaving early. The word was \'%s\'\n", selectedWord);
                            write(simpleChildSocket, message, strlen(message));
                            closeFlag = CLOSE_NO_ERROR;
                            break;
                        default:
                            write(simpleChildSocket, ERROR_INCORRECT_COMMAND_MESSAGE, strlen(ERROR_INCORRECT_COMMAND_MESSAGE));
                            closeFlag = CLOSE_ERROR;
                        }
                        if (closeFlag == CONTINUE || closeFlag == CLOSE_NO_ERROR)
                        {
                            if (guessed)
                                write(simpleChildSocket, GUESSED_MESSAGE, strlen(GUESSED_MESSAGE));
                            else if (clientAttempts < attempts && !guessed && closeFlag != CLOSE_NO_ERROR)
                            {
                                char temp[256] = "";
                                sprintf(temp, "OK %d %s\n", clientAttempts, result);
                                write(simpleChildSocket, temp, strlen(temp));
                                closeFlag = CONTINUE;
                            }
                            else if(clientAttempts == attempts && !guessed)
                            {
                                char temp[256] = "";
                                sprintf(temp, "END %d %s\n", clientAttempts, selectedWord);
                                write(simpleChildSocket, temp, strlen(temp));
                                closeFlag = CLOSE_NO_ERROR;
                            }
                        }
                    }
                }
                else
                {
                    write(simpleChildSocket, ERROR_INCORRECT_MESSAGE, strlen(ERROR_INCORRECT_MESSAGE));
                    closeFlag = CLOSE_ERROR;
                }
            }
        }

        // read(simpleChildSocket, )
        if (close(simpleChildSocket) == 0)
            fprintf(stdout, "Client disconnected.\n");
        else
            perror("Error during child socket closure: ");
    }

    close(simpleSocket);
    return 0;
}
