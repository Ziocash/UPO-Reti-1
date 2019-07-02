#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>

#define EXECUTABLE_NAME "TEXT Analyzer"
#define VERSION "0.4.5100.43"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

const char *COMMANDS[] = { "START", "TEXT", "HIST", "EXIT", "QUIT" };

void print_title()
{
    fprintf(stderr, "%s - %s\n", EXECUTABLE_NAME, VERSION);
}

void print_menu()
{
    fprintf(stderr, "Scegliere un'operazione da fare\n");
    fprintf(stderr, "Eseguire le operazioni in ordine: non è possibile analizzare un testo non fornito!\n");
    fprintf(stderr, "1- Inviare il testo con la relativa lunghezza (es.: \"ciao come stai\")\n");
    fprintf(stderr, "2- Visualizzare la frequenza dei caratteri contenuti nella stringa\n");
    fprintf(stderr, "3- Chiudere la connessione ricevendo risposta\n");
    fprintf(stderr, "4- Chiudere la connessione\n");
    fprintf(stderr, "5- Pulisce la schermata rimuovendo i caratteri\n\n");
    fflush(stderr);
}

void elapse_string(char *str)
{
    char delim[] = "|";
    int length = strlen(str);
    char response[length];
    bzero(response, sizeof(response));
    int i = 0, j = 0;
    while(i < length)
    {
        if(isalnum(str[i]) || ispunct(str[i]))
        {
            response[j] = str[i];
            j++;
        }
        else if(str[i] == 32)
        {
            response[j] = '|';
            j++;
        }
        i++;
    }
    response[j] = '\0';

    char *ptr = strtok(response, delim);

    while(ptr != NULL)
    {
        if(strcmp(ptr, "OK") == 0)
        {
            ptr = strtok(NULL, delim); 
            if(strcmp(ptr, COMMANDS[0]) == 0)
            {
                ptr = strtok(NULL, delim);
                fprintf(stderr, ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET, ptr);
            }
            else if(strcmp(ptr, COMMANDS[1]) == 0)
            {
                ptr = strtok(NULL, delim); 
                fprintf(stderr, "Contatore (server): %s\n", ptr);
                break;
            }
            else if(strcmp(ptr, COMMANDS[2]) == 0 || strcmp(ptr, COMMANDS[3]) == 0)
            {
                char string[512];
                int len = 0;
                memset(string, 0, sizeof(string));
                while(ptr != NULL)
                {
                    if(strstr(ptr, "OK") == NULL && strstr(ptr, COMMANDS[2]) == NULL && strstr(ptr, "END") == NULL)
                    {                        
                        strcat(string, ptr);
                        strcat(string, " ");
                        ptr = strtok(NULL, delim);
                    }
                    else if(strstr(ptr, "END") != NULL)
                    {
                        strcat(string, "Istogramma terminato\n");
                        break;
                    }
                    else 
                    {
                        ptr = strtok(NULL, delim);
                        strcat(string, "\n");
                    }
                }

                fprintf(stderr, ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET, string);
                fflush(stderr);
            }
            else 
            {
                ptr = strtok(NULL, delim);
                fprintf(stderr, ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET, ptr);
            }

            // while(ptr != NULL)
            // {
            //     fprintf(stderr, "%s ", ptr);
            //     ptr = strtok(NULL, delim);
            // }
            fprintf(stderr, "\n");
            break;
        }
        else if(strcmp(ptr, "ERR") == 0)
        {
            ptr = strtok(NULL, delim);
            ptr = strtok(NULL, delim);
            while(ptr != NULL)
            {
                fprintf(stderr, "%s ", ptr);
                ptr = strtok(NULL, delim);
            }
            fprintf(stderr, "\n");
        }
    }
    bzero(str, sizeof(&str));
    bzero(response, sizeof(response));
}

int count_string(char str[])
{
    int counter = 0;
    for(int i = 0; i < strlen(str); i++)
        if(isdigit(str[i]) || isalpha(str[i]))
            counter++;
    return counter;
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
        fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
        exit(1);
    }

    //create a streaming socket
    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1) 
    {
        fprintf(stderr, ANSI_COLOR_RED "Could not create a socket!\n" ANSI_COLOR_RESET);
        fflush(stderr);
        exit(1);
    }
    else 
    {
	    fprintf(stderr, ANSI_COLOR_GREEN "Socket created!\n" ANSI_COLOR_RESET);
        fflush(stderr);
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
	    fprintf(stderr, ANSI_COLOR_GREEN "Connect successful! Server at %s:%d\n" ANSI_COLOR_RESET, inet_ntoa(simpleServer.sin_addr), simplePort);
        returnStatus = read(simpleSocket, buffer, sizeof(buffer));
    	elapse_string(buffer);
        print_menu();        
        //fprintf(stderr, "%s\n", buffer);
        fflush(stderr);
        bzero(buffer, sizeof(buffer));
    }
    else 
    {
        fprintf(stderr, ANSI_COLOR_RED "Could not connect to address!\n" ANSI_COLOR_RESET);
        close(simpleSocket);
        exit(1);
    }

    int flag = 1;	
	do
    {
		char text[512] = "";
        char message[512] = "";
        int choice = 0;
        do
        {
            char choice_s[] = "";            
            fprintf(stderr, "Inserisci un comando: ");
            fscanf(stdin, "  %s", choice_s);
            choice = atoi(choice_s);
            if(choice == 1)
            {
                fprintf(stderr, "Inserire il testo da inviare: ");
                fscanf(stdin, " %[^\n]s\n", text);
                int length = count_string(text);
                char length_s[10];
                bzero(length_s, sizeof(length_s));
                strcat(text, " ");
                sprintf(length_s, "%d", length);
                strcat(text, length_s);
                break;
            }
            if(choice > 1 && choice < 6)
                break;
            fprintf(stderr, ANSI_COLOR_RED "Scelta %d non corretta, reinserimento necessario.\n" ANSI_COLOR_RESET, choice);
        }
        while(choice <= 0 || choice > 5);        
        text[strlen(text)] = '\n';
        fflush(stderr);

        switch (choice)
        {
        case 1:
            strcat(message, COMMANDS[1]);
            strcat(message, " ");
            strcat(message, text);
            write(simpleSocket, message, strlen(message));
            break;

        case 2:
            strcat(message, COMMANDS[2]);
            strcat(message, "\n");
            write(simpleSocket, message, strlen(message));
            break;

        case 3:
            strcat(message, COMMANDS[3]);
            strcat(message, "\n");
            write(simpleSocket, message, strlen(message));
            break;

        case 4:
            strcat(message, COMMANDS[4]);
            strcat(message, "\n");
            write(simpleSocket, message, strlen(message));
            flag = 0;
            break;
        case 5:
            system("clear");
            print_title();
            print_menu();
            break;

        default:
            break;
        }

        // if(strstr(message, "QUIT") != NULL)
        // {
        //     flag = 0;
        //     write(simpleSocket, message, strlen(message));
        // }
        // else
		// {
        //     write(simpleSocket, message, strlen(message));
        // }
        bzero(buffer, sizeof(buffer));
        bzero(message, sizeof(message));
        if(choice != 5)
        {
            // do
            // {
                returnStatus = read(simpleSocket, buffer, 512);
                elapse_string(buffer); 
                fflush(stderr);
            // } while ();           
                       
        }
    	
	}
    while (flag);

    fprintf(stderr, ANSI_COLOR_YELLOW "A presto da %s\n" ANSI_COLOR_RESET, EXECUTABLE_NAME);
    fflush(stderr);
    close(simpleSocket);
    return 0;

}