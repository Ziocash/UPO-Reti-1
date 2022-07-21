#include <stdio.h>      
#include <sys/types.h>
#include <sys/socket.h>   
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <ctype.h>



int main(int argc, char *argv[]) {


	char serverMessage[512]="";
	char commandBuffer[523]=""; //523 = 5 + 512 + 4 + 2

    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;

    char command[523]="";
	char text[512]="";
	char clientCounter[512]="";

	int counter=0;

	char clientText[5000] = ""; //buffer grande perché memorizza tutti i testi inviati


    //Controlla che ci sia il numero di porta
    if (2 != argc) {

        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);

    }

    /////////////////////////// SOCKET \\\\\\\\\\\\\\\\\\\\\\\\\

    //Creazione streaming socket
    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1) {

        fprintf(stderr, "Could not create a socket!\n");
        exit(1);

    }
    else {
	    fprintf(stderr, "Socket created!\n");
    }


    //////////////////////////// BIND \\\\\\\\\\\\\\\\\\\\\\\\\\\\\

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

    if (returnStatus == 0) {
	    fprintf(stderr, "Bind completed!\n");
    }
    else {
        fprintf(stderr, "Could not bind to address!\n");
	close(simpleSocket);
	exit(1);
    }


    //////////////////////////// LISTEN \\\\\\\\\\\\\\\\\\\\\\\\\\\

    /*put the server socket in a passive mode, where it waits for 
    the client to approach the server to make a connection*/


    /* lets listen on the socket for connections      */
    returnStatus = listen(simpleSocket, 5);

    if (returnStatus == -1) {
        fprintf(stderr, "Cannot listen on socket!\n");
		close(simpleSocket);
        exit(1);
    }
    else{
    	fprintf(stderr, "Connected to the socket!\n");
    }


     //////////////////////////// ACCEPT \\\\\\\\\\\\\\\\\\\\\\\\\\\

    /* At this point, connection is established between client and server, 
    and they are ready to transfer data */

  while (1){



    	/* set up variables to handle client connections */
        struct sockaddr_in clientName = { 0 };
		int simpleChildSocket = 0;
		int clientNameLength = sizeof(clientName);

	/* wait here */


		/* block on accept function call */
        simpleChildSocket = accept(simpleSocket,(struct sockaddr *)&clientName, &clientNameLength);
       

		if (simpleChildSocket == -1) {

	        fprintf(stderr, "S: Cannot accept connections!\n");
		    close(simpleSocket);
		    exit(1);

		}
		else{
			char welcomeMessage[512] = "OK START benvenuto!\n";
			returnStatus = write(simpleChildSocket, welcomeMessage, strlen(welcomeMessage));
			if(returnStatus<0)
				fprintf(stderr, "S: Errore (write)\n");
			else
				fprintf(stderr, "\nS: %s\n", welcomeMessage);	
		}



	 //////////////////////////// READ \\\\\\\\\\\\\\\\\\\\\\\\\\\
	
while(1){ 
		
		//Svuoto i buffer prima di iniziare 
		bzero(commandBuffer, sizeof(commandBuffer)); 
		bzero(serverMessage, sizeof(serverMessage)); 
		
		//LETTURA
		returnStatus = read(simpleChildSocket, commandBuffer, 523);
		if(strlen(commandBuffer)==0){
			close(simpleChildSocket);
			break;
		}
	

		fprintf(stderr, "\nC: %s", commandBuffer);

		//IDENTIFICO IL COMANDO 
		char command[523]="";

		char text[4]="TEXT";
    	char hist[4]="HIST";
    	char exit[4]="EXIT";
    	char quit[4]="QUIT";

    	int t=0;
    	int h=0;
    	int e=0;
    	int q=0;
		
		//Estrapolo il comando
    	for(int i=0; i<4; i++){
        	command[i]=commandBuffer[i];
   		}


   		//Individuo il tipo di comando
   		for(int i=0; i<4; i++){
        	if(command[i]==text[i])
            	t++; 

        	if(command[i]==hist[i])
            	h++; 

        	if(command[i]==exit[i])
            	e++; 


       	 	if(command[i]==quit[i])
            	q++; 
    	}

    
	
		/* oldClientText e clientText mi servono per memorizzare i caratteri
			contenuti nel testo che dovranno essere analizzati con HIST */
			char oldClientText[512]="";
			
			

		//-->Text
		if(t==4){

			char clientCounter[3]="";
			memset(clientCounter, 0, sizeof(clientCounter));

			counter = 0;

			//Salvo ultima, penultima e terzultima posizione
			int ultimate = strlen(commandBuffer)-2;
			int penultimate = ultimate-1;
			int antepenult = ultimate-2;
			int counterIndex = 0;
			int startingCounterPos = 0;
			
			//ciclo in cui si estrapola il contatore
			if(isdigit(commandBuffer[ultimate])){
				if(isdigit(commandBuffer[penultimate])){
					if(isdigit(commandBuffer[antepenult])){
						clientCounter[0] = commandBuffer[antepenult];
						clientCounter[1] = commandBuffer[penultimate];
						clientCounter[2] = commandBuffer[ultimate];
						startingCounterPos = antepenult;
					}
					else {
						clientCounter[0] = commandBuffer[penultimate];
						clientCounter[1] = commandBuffer[ultimate];
						startingCounterPos = penultimate;
					}
				}
				else{
					clientCounter[0] = commandBuffer[ultimate];
					startingCounterPos = ultimate;
				}
			}


			for(int i=5; i<startingCounterPos; i++){ //5 perchè conto anche lo spazio dopo il comando TEXT

				//Memorizzo i caratteri in clientText
				if(isalpha(commandBuffer[i]) || isdigit(commandBuffer[i])){

					if(strlen(clientText)!=0){
						snprintf(oldClientText, 512, "%c", tolower(commandBuffer[i])); //tolower così non si confonde con le parole chiave
						strcat(clientText, oldClientText);
					}
					else{
						snprintf(clientText, 523, "%c", commandBuffer[i]);
					}
					counter++;
				}

			}

			//Confronto mio contatore con contatore del client
			if(counter==(atoi(clientCounter))){

				char concat[512]="";
				snprintf(concat, 512, "OK TEXT %d\n", counter); //per concatenare char[] con i
				strcpy(serverMessage, concat);
			}
			else{
				strcpy(serverMessage, "ERR TEXT Errore, numero di caratteri validi non corrispondente a quello reale\n"); 
				printf("S: %s\n", serverMessage);
				write(simpleChildSocket, serverMessage, strlen(serverMessage));
				memset(clientText, 0, sizeof(clientText));
				close(simpleChildSocket);
				break;
			}

		}

		//-->Hist
		else if(h==4){

			//Controllo che sia stato inviato del testo precedentemente
			if(strlen(clientText)==0){
				strcpy(serverMessage, "ERR HIST Errore, nessun testo da analizzare\n"); 
				printf("\nS: %s\n", serverMessage);
				write(simpleChildSocket, serverMessage, strlen(serverMessage));
				memset(clientText, 0, sizeof(clientText));
				close(simpleChildSocket);
				break;
			}

			char frequency[521] = "";
			memset(frequency, 0, sizeof(frequency));


			char concat2[521]="";
			memset(concat2, 0, sizeof(concat2));

	
			//Trovo le frequenze
			for(int i=0; i<(strlen(clientText)); i++){ 
				//Se trovo un doppione (o più)
				for(int j=0; j<(strlen(clientText)); j++){
					if(clientText[j]==clientText[i]){
						frequency[i]++;
					}

				}
			}

			//Scrivo il messaggio con le frequenze
			for(int i=0; i<strlen(clientText); i++){
				int j;
				for(j=0; j<i; j++){
					//ho già scritto quel carattere allora non faccio niente
					if (clientText[i]==clientText[j]){
						break;
					}
				}

				if(i==j){
					
					//Se sono già presenti delle distribuzioni concateno
					if(strlen(serverMessage)!=0){
						snprintf(concat2, 512, "%c:%d ", clientText[i], frequency[i]);
						strcat(serverMessage, concat2);
					}
					else{
						snprintf(serverMessage, 512, "OK HIST %c:%d ", clientText[i], frequency[i]);
					}

					
					if(strlen(serverMessage)>32){
						strcat(serverMessage, "\n");
						printf("\nS: %s\n", serverMessage);
						write(simpleChildSocket, serverMessage, strlen(serverMessage));
						memset(serverMessage, 0, sizeof(serverMessage));
					}
					
				}
				

			}

			//Se c'è ancora qualcosa da inviare
			if(strlen(serverMessage)!=0){
				strcat(serverMessage, "\n");
				printf("\nS: %s\n", serverMessage);
				write(simpleChildSocket, serverMessage, strlen(serverMessage));
				
			}
			else
				strcat(serverMessage, "\n");
			
			
			memset(serverMessage, 0, sizeof(serverMessage));
			strcpy(serverMessage, "OK HIST END\n");
			
			

		}

		//-->Exit
		else if(e==4){

			//Controllo che sia stato inviato del testo precedentemente
			if(strlen(clientText)==0){
				strcpy(serverMessage, "ERR HIST Errore, nessun testo da analizzare\n"); 
				printf("\nS: %s\n", serverMessage);
				write(simpleChildSocket, serverMessage, strlen(serverMessage));
				memset(clientText, 0, sizeof(clientText));
				close(simpleChildSocket);
				break;
			}

			char frequency[2000] = "";
			bzero(frequency, sizeof(frequency));


			char concat2[2000]="";
			bzero(concat2, sizeof(concat2));

			//Trovo le frequenze
			for(int i=0; i<(strlen(clientText)); i++){ 
				//Se trovo un doppione (o più)
				for(int j=0; j<(strlen(clientText)); j++){
					if(clientText[j]==clientText[i]){
						frequency[i]++;
					}

				}
			}

			//Scrivo il messaggio con le frequenze
			for(int i=0; i<strlen(clientText); i++){
				int j;
				for(j=0; j<i; j++){
					if (clientText[i]==clientText[j]){
						break;
					}
				}

				if(i==j){
					
	
					if(strlen(serverMessage)!=0){
						snprintf(concat2, 2000, "%c:%d ", clientText[i], frequency[i]);
						strcat(serverMessage, concat2);
					}
					else{
						snprintf(serverMessage, 512, "OK HIST %c:%d ", clientText[i], frequency[i]);
					}

					
					if(strlen(serverMessage)>34){
						strcat(serverMessage, "\n");
						printf("\nS: %s\n", serverMessage);
						write(simpleChildSocket, serverMessage, 512);
						memset(serverMessage, 0, sizeof(serverMessage));
					}
					
				}
				

			}

			//Se c'è ancora qualcosa da inviare
			if(strlen(serverMessage)!=0){
				strcat(serverMessage, "\n");
				printf("\nS: %s\n", serverMessage);
				write(simpleChildSocket, serverMessage, strlen(serverMessage));
				
			}
			else
				strcat(serverMessage, "\n");
			
			//Ultimo messaggio di HIST
			memset(serverMessage, 0, sizeof(serverMessage));
			strcpy(serverMessage, "OK HIST END");
			write(simpleChildSocket, serverMessage, strlen(serverMessage));
			printf("\nS: %s\n", serverMessage);
			
			//Evita la sovrapposizione delle write
			sleep(1);

			//Chiusura connessione
			memset(serverMessage, 0, sizeof(serverMessage));
			strcpy(serverMessage, "OK EXIT connessione chiusa\n");
			printf("\nS: %s\n", serverMessage);
			write(simpleChildSocket, serverMessage, strlen(serverMessage));
			memset(clientText, 0, sizeof(clientText));
			close(simpleChildSocket);
			break;


		}

		//-->Quit
		else if(q==4){
			strcpy(serverMessage, "OK QUIT connessione chiusa\n");
			printf("\nS: %s\n", serverMessage);
			write(simpleChildSocket, serverMessage, strlen(serverMessage));
			memset(clientText, 0, sizeof(clientText));
			close(simpleChildSocket);
			break;
		}

		 //////////////////////////// WRITE \\\\\\\\\\\\\\\\\\\\\\\\\\\


		returnStatus = write(simpleChildSocket, serverMessage, strlen(serverMessage));
		if(returnStatus>0){
			fprintf(stderr, "\nS: %s\n", serverMessage);
		}
	}
	
		//////////////////////////// CLOSE \\\\\\\\\\\\\\\\\\\\\\\\\\\
        
        close(simpleChildSocket);
        memset(clientText, 0, sizeof(clientText));

 	}

    close(simpleSocket);

}

