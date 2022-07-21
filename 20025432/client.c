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
#define VERSION "0.7.10.3"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET   "\x1b[0m"

const char *COMMANDS[] = { "START", "TEXT", "HIST", "EXIT", "QUIT" };

//stampa titolo
void print_title()
{
    fprintf(stderr, "%s - %s\n", EXECUTABLE_NAME, VERSION);
}

//stampa menu
void print_menu()
{
    fprintf(stderr, "Scegliere un'operazione da fare\n");
    fprintf(stderr, "Eseguire le operazioni in ordine: non è possibile analizzare un testo non fornito!\n");
    fprintf(stderr, "1- Inviare il testo (es.: \"ciao come stai\")\n");
    fprintf(stderr, "2- Visualizzare la frequenza dei caratteri contenuti nella stringa\n");
    fprintf(stderr, "3- Chiudere la connessione ricevendo risposta\n");
    fprintf(stderr, "4- Chiudere la connessione\n");
    fprintf(stderr, "5- Pulisce la schermata rimuovendo i caratteri\n\n");
    fflush(stderr);
}

//elaborazione della stringa ricevuta
void elapse_string(char *str)
{
    char delim[] = "|";
    int length = strlen(str);
    char response[length];
    int flag = 0; 

    bzero(response, sizeof(response));
    int i = 0, j = 0;
    while(i < length)
    {
        if(isalnum(str[i]))
        {
            response[j] = str[i];
            j++;
        }
        else
        {
            response[j] = '|';
            j++;
        }
        i++;
    }

    //splitta la stringa in modo da verificarne il contenuto
    char *ptr = strtok(response, delim);

    while(ptr != NULL)
    {
        if(strcmp(ptr, "OK") == 0)
        {
            ptr = strtok(NULL, delim); 
            //comando START
            if(strcmp(ptr, COMMANDS[1]) == 0)
            {
                ptr = strtok(NULL, delim); 
                fprintf(stderr, ANSI_COLOR_GREEN "Contatore (server): %s\n" ANSI_COLOR_RESET, ptr);
                break;
            }
            //comandi HIST ed EXIT
            else if(strcmp(ptr, COMMANDS[2]) == 0 || strcmp(ptr, COMMANDS[3]) == 0)
            {
                char string[1024];
                if(strcmp(ptr, COMMANDS[3]) == 0)
                    flag = 1;
                bzero(string, sizeof(string));
                strcat(string, "\nCalcolo istogramma in corso...\n");
                while(ptr != NULL)
                {
                    if(strstr(ptr, "OK") == NULL && strstr(ptr, COMMANDS[2]) == NULL && strstr(ptr, "END") == NULL && strstr(ptr, COMMANDS[3]) == NULL)
                    {                        
                        strcat(string, ptr);
                        if(isalpha(ptr[0]))
                            strcat(string, "-");
                        else
                            strcat(string, " ");
                        ptr = strtok(NULL, delim);
                    }
                    else if(strstr(ptr, "END") != NULL)
                    {
                        strcat(string, "Istogramma terminato\n");                        
                        if(flag)
                            break;
                        else
                            ptr = strtok(NULL, delim);
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
            //comando QUIT
            else 
            {
                char string[512];
                memset(string, 0, sizeof(string));
                ptr = strtok(NULL, delim);
                while(ptr != NULL)
                {
                    strcat(string, ptr);
                    strcat(string, " ");
                    ptr = strtok(NULL, delim);
                }
                fprintf(stderr, ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET, string);
            }
            fprintf(stderr, "\n");
            break;
        }
        //gestione degli errori
        else if(strcmp(ptr, "ERR") == 0)
        {
            char string[1024];
            bzero(string, sizeof(string));
            ptr = strtok(NULL, delim);
            ptr = strtok(NULL, delim);
            while(ptr != NULL)
            {
                if(isalpha(ptr[0]) && !ispunct(ptr[0]))
                {
                    strcat(string, ptr);
                    strcat(string, " ");
                }
                ptr = strtok(NULL, delim);
            }
            fprintf(stderr, ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, string);
            fprintf(stderr, ANSI_COLOR_RED "Server disconnesso\n" ANSI_COLOR_RESET);
            exit(0);
        }
    }
    bzero(str, sizeof(str));
    bzero(response, sizeof(response));
}

//string counter function
int count_string(char str[])
{
    int counter = 0;
    for(int i = 0; i < strlen(str); i++)
        if(isalnum(str[i]) || ispunct(str[i]))
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
    simpleServer.sin_addr.s_addr = inet_addr(argv[1]);
    simpleServer.sin_port = htons(simplePort);

    //connect to the address and port with our socket
    returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

    if (returnStatus == 0) 
    {
	    fprintf(stderr, ANSI_COLOR_GREEN "Connect successful! Server at %s:%d\n" ANSI_COLOR_RESET, inet_ntoa(simpleServer.sin_addr), simplePort);
        bzero(buffer, sizeof(buffer));
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

    //client cycle
	do
    {
		char text[4096] = "";
        char message[512] = "";
        int choice = 0;
        do
        {
            //traduzione comando
            char choice_s[] = "";            
            fprintf(stderr, "Inserisci un comando: ");
            fscanf(stdin, "  %s", choice_s);
            choice = atoi(choice_s);
            //richiesta inserimento del testo
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
            if(choice > 1 && choice < 6) //contollo di correttezza sulla scelta
                break;
            fprintf(stderr, ANSI_COLOR_RED "Scelta %d non corretta, reinserimento necessario.\n" ANSI_COLOR_RESET, choice);
        }
        while(choice <= 0 || choice > 5);        
        text[strlen(text)] = '\n';
        fflush(stderr);

        //Command identification
        switch (choice)
        {
            //TEXT
            case 1:
                strcat(message, COMMANDS[1]);
                strcat(message, " ");
                strcat(message, text);
                write(simpleSocket, message, strlen(message));
                break;
            //HIST
            case 2:
                strcat(message, COMMANDS[2]);
                strcat(message, "\n");
                write(simpleSocket, message, strlen(message));
                break;
            //EXIT
            case 3:
                strcat(message, COMMANDS[3]);
                strcat(message, "\n");
                write(simpleSocket, message, strlen(message));
                flag = 0;
                break;
            //QUIT
            case 4:
                strcat(message, COMMANDS[4]);
                strcat(message, "\n");
                write(simpleSocket, message, strlen(message));
                flag = 0;
                break;
            //Console clear
            case 5:
                system("clear");
                print_title();
                print_menu();
                break;
            //No selection
            default:
                choice = 0;
                break;
        }
        //Initializations
        bzero(buffer, sizeof(buffer));
        bzero(message, sizeof(message));
        //Socket reading if choice is not "cls" 
        if(choice > 0 && choice < 5)
        {
            usleep(300);
            returnStatus = read(simpleSocket, buffer, 512);
            elapse_string(buffer); 
            fflush(stderr);                           
        }
    	
	}
    while (flag);

    fprintf(stderr, ANSI_COLOR_YELLOW "A presto da %s\n" ANSI_COLOR_RESET, EXECUTABLE_NAME);
    fflush(stderr);
    close(simpleSocket);
    return 0;

}