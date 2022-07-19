#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>

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
// void elapse_string(char *str)
// {
//     char delim[] = "|";
//     int length = strlen(str);
//     char response[length];
//     int flag = 0; 
//     bzero(response, sizeof(response));
//     int i = 0, j = 0;
//     while(i < length)
//     {
//         if(isalnum(str[i]))
//         {
//             response[j] = str[i];
//             j++;
//         }
//         else
//         {
//             response[j] = '|';
//             j++;
//         }
//         i++;
//     }
//     //splitta la stringa in modo da verificarne il contenuto
//     char *ptr = strtok(response, delim);
//     while(ptr != NULL)
//     {
//         if(strcmp(ptr, "OK") == 0)
//         {
//             ptr = strtok(NULL, delim); 
//             //comando START
//             if(strcmp(ptr, COMMANDS[1]) == 0)
//             {
//                 ptr = strtok(NULL, delim); 
//                 fprintf(stderr,  "Contatore (server): %s\n" , ptr);
//                 break;
//             }
//             //comandi HIST ed EXIT
//             else if(strcmp(ptr, COMMANDS[2]) == 0 || strcmp(ptr, COMMANDS[3]) == 0)
//             {
//                 char string[1024];
//                 if(strcmp(ptr, COMMANDS[3]) == 0)
//                     flag = 1;
//                 bzero(string, sizeof(string));
//                 strcat(string, "\nCalcolo istogramma in corso...\n");
//                 while(ptr != NULL)
//                 {
//                     if(strstr(ptr, "OK") == NULL && strstr(ptr, COMMANDS[2]) == NULL && strstr(ptr, "END") == NULL && strstr(ptr, COMMANDS[3]) == NULL)
//                     {                        
//                         strcat(string, ptr);
//                         if(isalpha(ptr[0]))
//                             strcat(string, "-");
//                         else
//                             strcat(string, " ");
//                         ptr = strtok(NULL, delim);
//                     }
//                     else if(strstr(ptr, "END") != NULL)
//                     {
//                         strcat(string, "Istogramma terminato\n");                        
//                         if(flag)
//                             break;
//                         else
//                             ptr = strtok(NULL, delim);
//                     }
//                     else 
//                     {
//                         ptr = strtok(NULL, delim);
//                         strcat(string, "\n");
//                     }
//                 }
//                 fprintf(stderr,  "%s" , string);
//                 fflush(stderr);
//             }
//             //comando QUIT
//             else 
//             {
//                 char string[512];
//                 memset(string, 0, sizeof(string));
//                 ptr = strtok(NULL, delim);
//                 while(ptr != NULL)
//                 {
//                     strcat(string, ptr);
//                     strcat(string, " ");
//                     ptr = strtok(NULL, delim);
//                 }
//                 fprintf(stderr,  "%s" , string);
//             }
//             fprintf(stderr, "\n");
//             break;
//         }
//         //gestione degli errori
//         else if(strcmp(ptr, "ERR") == 0)
//         {
//             char string[1024];
//             bzero(string, sizeof(string));
//             ptr = strtok(NULL, delim);
//             ptr = strtok(NULL, delim);
//             while(ptr != NULL)
//             {
//                 if(isalpha(ptr[0]) && !ispunct(ptr[0]))
//                 {
//                     strcat(string, ptr);
//                     strcat(string, " ");
//                 }
//                 ptr = strtok(NULL, delim);
//             }
//             fprintf(stderr,  "%s\n" , string);
//             fprintf(stderr,  "Server disconnesso\n" );
//             exit(0);
//         }
//     }
//     bzero(str, sizeof(str));
//     bzero(response, sizeof(response));
// }

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

    if (argc != 3) 
    {
        fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
        exit(1);
    }

    //create a streaming socket
    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1) 
    {
        fprintf(stderr,  "Could not create a socket!\n" );
        fflush(stderr);
        exit(1);
    }
    else 
    {
	    fprintf(stderr,  "Socket created!\n" );
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
	    fprintf(stderr,  "Connect successful! Server at %s:%d\n" , inet_ntoa(simpleServer.sin_addr), simplePort);
        bzero(buffer, sizeof(buffer));
        returnStatus = read(simpleSocket, buffer, sizeof(buffer));
    	//elapse_string(buffer);
        print_menu();        
        //fprintf(stderr, "%s\n", buffer);
        fflush(stderr);
        bzero(buffer, sizeof(buffer));
    }
    else 
    {
        fprintf(stderr,  "Could not connect to address!\n" );
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
            fprintf(stderr,  "Scelta %d non corretta, reinserimento necessario.\n" , choice);
        }
        while(choice <= 0 || choice > 5);        
        text[strlen(text)] = '\n';
        fflush(stderr);

        //Command identification
        switch (choice)
        {
            //TEXT
            case 1:
                //strcat(message, COMMANDS[1]);
                strcat(message, " ");
                strcat(message, text);
                write(simpleSocket, message, strlen(message));
                break;
            //HIST
            case 2:
                //strcat(message, COMMANDS[2]);
                strcat(message, "\n");
                write(simpleSocket, message, strlen(message));
                break;
            //EXIT
            case 3:
                //strcat(message, COMMANDS[3]);
                strcat(message, "\n");
                write(simpleSocket, message, strlen(message));
                flag = 0;
                break;
            //QUIT
            case 4:
                //strcat(message, COMMANDS[4]);
                strcat(message, "\n");
                write(simpleSocket, message, strlen(message));
                flag = 0;
                break;
            //Console clear
            case 5:
                system("clear");
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
            //elapse_string(buffer); 
            fflush(stderr);                           
        }
    	
	}
    while (flag);

    fprintf(stderr,  "A presto\n");
    fflush(stderr);
    close(simpleSocket);
    return 0;

}