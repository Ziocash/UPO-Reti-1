#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> // isspace()
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()

#define MAX 256

#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_RESET "\x1b[0m"

#define PLAY_GAME 1
#define QUIT_GAME 2

#define CLOSE_EXECUTION 1
#define CONTINUE_EXECUTION 0

typedef enum enum_commands
{
    OK,      // received message
    QUIT,    // leave execution
    PERFECT, // guessed word
    END,     // no more attempts
    ERR      // error
} commands;

typedef struct sockaddr_in *sockaddr_t;

/// @brief prints red string
void print_error(const char *string)
{
    fprintf(stderr, COLOR_RED "%s\n" COLOR_RESET, string);
    fflush(stderr);
}

/// @brief prints green string
void print_success(const char *string)
{
    fprintf(stderr, COLOR_GREEN "%s\n" COLOR_RESET, string);
    fflush(stderr);
}

/// @brief create server and assign domain, address and port
/// @return resulting server
sockaddr_t init_server(int domain, in_addr_t address, int port)
{
    sockaddr_t myServer = malloc(sizeof(struct sockaddr_in));
    bzero(myServer, sizeof(myServer));

    (*myServer).sin_family = domain;       // domain assignment
    (*myServer).sin_addr.s_addr = address; // ip_address assignment
    (*myServer).sin_port = port;           // port assignment

    return myServer; // resulting server
}

/// @brief create socket
/// @return resulting socket
int create_socket()
{
    int mySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (mySocket < 0)
    {
        print_error("Socket creation failed...\n");
        exit(EXIT_FAILURE);
    }
    else
        print_success("Socket succesfully created..\n");

    return mySocket;
}

/// @brief connect client socket to server socket
void connect_socket_to_server(int clientSocket, struct sockaddr *serverSocket)
{
    int responseStatus = connect(clientSocket, serverSocket, sizeof(*serverSocket));

    if (responseStatus < 0)
    {
        print_error("Connection with the server failed...\n");
        close(clientSocket);
        free(serverSocket);
        exit(EXIT_FAILURE);
    }
    else
        print_success("Connected to the server..\n");
}

/// @brief search space position in given string
/// @return first space position
int search_space_pos(const char *string)
{
    int space_pos = 0;
    for (int i = 0; string[i] != ' '; i++)
        space_pos = i + 1;
    return space_pos;
}

/// @brief checks given string for errors and retrieves message
/// @return 1 if no errors, 0 otherwise
int get_message(const char *string, char **msg)
{
    int space_pos = search_space_pos(string);
    int needle = space_pos;
    int err = 1;

    if (isspace(string[space_pos]))
        needle++;
    else
        err &= 0;

    // get message
    *msg = malloc(strlen(string + needle + 1) + 1);
    strcpy(*msg, string + needle);
    if (string[strlen(string) - 1] != '\n')
        err &= 0;

    if (!err) // if there's an error
        print_error("Malformed message...");

    return err;
}

/// @brief checks given string for errors and retrieves attempts and message
/// @return 1 if no errors, 0 otherwise
int get_ok_message(const char *string, int *attempts, char **msg)
{
    int space_pos = search_space_pos(string);
    int needle = space_pos;
    int err = 1;

    if (isspace(string[space_pos]))
        needle++;
    else
        err &= 0;
    if (isdigit(string[needle]))
        sscanf(string + needle, "%d", attempts);
    else
        err &= 0;
    if (*attempts < 9)
        needle++;
    return get_message(string + needle, msg);
}

/// @brief compare command and value
/// @param string message to compare
/// @param cmd expected command
/// @param val given value
/// @return value if equal, -1 otherwise
commands cmdcmp(const char *string, const char *cmd, commands val)
{
    size_t size = strlen(cmd);
    int result = strncmp(string, cmd, size);
    if (!result)
        return val;
    return -1;
}

/// @brief retrieves command, attempts and message from the given string
int retrieve_message(const char *string, commands *cmd, int *attempts, char **msg)
{
    if (cmdcmp(string, "OK", OK) == OK)
    {
        if (cmdcmp(string + 3, "PERFECT\n", PERFECT) == PERFECT)
        {
            *cmd = PERFECT;
            return 1;
        }
        else if (strcmp(string + (strlen(string) - 1), "\n") != 0)
        {
            print_error("Malformed string...\n");
            return 0;
        }

        *cmd = OK;
        get_ok_message(string, attempts, msg);
        return 1;
    }

    if (cmdcmp(string, "END", END) == END)
    {
        *cmd = END;
        get_ok_message(string, attempts, msg);
        return 1;
    }
    else if (cmdcmp(string, "QUIT", QUIT) == QUIT)
        *cmd = QUIT;
    else if (cmdcmp(string, "ERR", ERR) == ERR)
        *cmd = ERR;

    get_message(string, msg);
    return 1;
}

/// @brief exchange messages between client and server
/// @param mySocket given socket
void socket_chat(int mySocket)
{
    char buffer[MAX] = "";
    bzero(buffer, sizeof(buffer));      // set buffer to 0
    commands cmd;                       // command received
    int attempts = 0, max_attempts = 0; // number of attempts
    char *message = NULL;               // received message

    while (1)
    {
        // int readStatus = read(mySocket, buffer, sizeof(buffer));
        // retrieve_message(buffer, &cmd, &attempts, &message);
        // bzero(buffer, sizeof(buffer)); // set buffer to 0

        fprintf(stderr, "Send to server: ");
        fscanf(stdin, " %[^\n]s", buffer);
        buffer[strlen(buffer)] = '\n';
        fflush(stdin);
        write(mySocket, buffer, strlen(buffer));
        bzero(buffer, sizeof(buffer)); // set buffer to 0
    }
}

int main(int argc, char *argv[])
{
    // check arguments
    if (argc < 3)
    {
        fprintf(stderr, COLOR_RED "Incorrect arguments. Usage: %s <server> <port>\n" COLOR_RESET, argv[0]);
        exit(EXIT_FAILURE);
    }

    // verify if server address is valid
    if (inet_addr(argv[1]) == INADDR_NONE)
    {
        print_error("Server address is invalid!\n");
    }

    // create and verify streaming socket
    int mySocket = create_socket();

    // server setup and assign
    sockaddr_t myServer = init_server(AF_INET, inet_addr(argv[1]), htons(atoi(argv[2])));

    // connect the client socket to server socket
    connect_socket_to_server(mySocket, (struct sockaddr *)myServer);

    // function for chat
    socket_chat(mySocket);

    // close the socket
    close(mySocket);
    free(myServer);
    return 0;
}