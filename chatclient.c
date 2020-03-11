/*******************************************************************************
* ** Author: Kyle Dixon
* ** Date: 7/18/2019-
* ** Descriptions: charclient (Project 1 CS 372)
* ** 	Client/Server Chat Application
* **	General Sources: Beej's guide and CS 341
* **	http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
* ****************************************************************************/
#include <stdio.h>    
#include <stdlib.h>   
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#define BUFFER 500

//Prep Address Function
	//"Prep the socket address structures for subsequent use"-Beej
	//pass in the address and port number of the desired socket
struct addrinfo* prepAddress(char* node, char* port){             
	struct addrinfo hints;	
    struct addrinfo *address;
    int stat;
	memset(&hints, 0, sizeof hints);	//set up holders
	hints.ai_family = AF_INET;		//IPv4         
	hints.ai_socktype = SOCK_STREAM;  	//TCP
	if((stat = getaddrinfo(node, port, &hints, &address)) != 0){	//create and validate the socket address structure
		fprintf(stderr,"Error: client port number %s\n", gai_strerror(stat));	//https://linux.die.net/man/3/gai_strerror
		exit(1);
	}
	return address;
}


//Create Socket Function
	//pass in socket address struct
	//creates a socket and validates
int createSocket(struct addrinfo* address){           
	int sockfd;       //holds the socket file descriptor                                     
	if ((sockfd = socket(address->ai_family, address->ai_socktype, address->ai_protocol)) == -1){	//create the socket, save the returned desciptor, and validate the socket via address info of struct
		fprintf(stderr, "Error: client socket creation\n");
		exit(1);
	}
	return sockfd;
}


//Connect Socket Function
	//pass in socket file descriptor and socket address struct
	//connects the socket and validates
void connectSocket(int sockfd, struct addrinfo * address){
	int stat;
	if ((stat = connect(sockfd, address->ai_addr, address->ai_addrlen)) == -1){	//connect socket and validate via address info of struct
		fprintf(stderr, "Error: client socket connection\n");
		exit(1);
	}
}


//Send Name function
	//pass in socket, user name, and server name
	//sends the user name to the server and receives the server name
	//sets up the chat
void sendName(int sockfd, char* user, char* server){	        //A helper function that saves the descriptors of the user and server
	int sender = send(sockfd, user, strlen(user), 0);	
	int recer = recv(sockfd, server, 10, 0);
}


//Chat Function
	//pass in the socket descriptor, username, and servername
	//sends and receives messages from the server, validates messages
void chat(int sockfd, char * username, char * servername){  
    int errorCheck = 0;     
	int stat;
	char input[BUFFER];     //size per directions         
    memset(input, 0 ,sizeof(input));	//clear the holder
	char output[BUFFER];	//size per directions  
	memset(output, 0, sizeof(output));	//clear the holder
	fgets(input, BUFFER, stdin);	//read from stream and store in input
    
	while(1){		
		printf("%s> ", username);   //prompt
		fgets(input, BUFFER, stdin);	//read from stream and store in input
		if (strcmp(input, "\\quit\n") == 0){    //check input for client to end the program
			break;
		}
		errorCheck = send(sockfd, input, strlen(input), 0);  //send the input to the server and validate
		if(errorCheck == -1){
				fprintf(stderr, "Error: Client Data Sent\n");
				exit(1);
		}
		stat = recv(sockfd, output, BUFFER, 0);	//receive the output from the server, save the returned stat, and validate via the stat
		if (stat == -1){
			fprintf(stderr, "Error: Client Data Received\n");
			exit(1);
		}
		else if (stat == 0){	//check the stat for the server to end the program
			printf("Server ended program\n");
			break;
		}
		else{	//print the message if stat passes validation
			printf("%s> %s\n", servername, output);
		}
		memset(input, 0, sizeof(input));  //clear holders for next round    
		memset(output, 0, sizeof(output));
	}
	
	close(sockfd);	//close the socket                        
	printf("Client Connection Closed\n");
}


//Main Function
int main(int argc, char *argv[]){                             
	char user[10];       
    char server[10];    
	if(argc != 3){                                                    
		fprintf(stderr, "Error: client arg number\n");	//validate argument number
		exit(1);
	}
    printf("Enter a user name that is 10 characters or less.");
	scanf("%s", user);	
	struct addrinfo* address = prepAddress(argv[1], argv[2]);  //cast pointer to address struct created from passed in arguments
	int sockfd = createSocket(address);   //create socket and save the socket descriptor
	connectSocket(sockfd, address);		//connect the created socket to the address struct
	sendName(sockfd, user, server);	//pass user name to server to setup chat
	chat(sockfd, user, server);     //initiate the chat
	freeaddrinfo(address); 	//free linked list of address struct
}
