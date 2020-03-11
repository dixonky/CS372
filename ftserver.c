/*******************************************************************************
* ** Author: Kyle Dixon
* ** Date: 8/06/2019-
* ** Descriptions: ftserver.c (Project 2 CS 372)
* ** 	Client/Server Network Application
* **	General Sources: Beej's guide (heavily followed for server/socket setup) and CS 341
* **	http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
* ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>

#define BUFFER 500
#define MAXDATASIZE 100 //"max number of bytes we can get at once" -Beej 
#define BACKLOG 10     //"how many pending connections queue will hold" -Beej 


//Prep Address Function (Beej)
	//"Prep the socket address structures for subsequent use"-Beej
	//pass in the port number and set up socket using address of the local host
struct addrinfo* prepAddress(char* port){             
	struct addrinfo hints;	
    struct addrinfo* address;
    int stat;
	memset(&hints, 0, sizeof hints);	//set up holders
	hints.ai_family = AF_INET;		//IPv4         
	hints.ai_socktype = SOCK_STREAM;  	//TCP
	hints.ai_flags = AI_PASSIVE;	//assign the address of the local host
	if((stat = getaddrinfo(NULL, port, &hints, &address)) != 0){	//create and validate the socket address structure
		fprintf(stderr,"Error: server port number %s\n", gai_strerror(stat));	//https://linux.die.net/man/3/gai_strerror
		exit(1);
	}
	return address;
}


//Prep IP Address Function (Beej)
	//"Prep the socket address structures for subsequent use"-Beej
	//pass in the address and port number of the desired socket
struct addrinfo* prepIPAddress(char* node, char* port){             
	struct addrinfo hints;	
    struct addrinfo* address;
    int stat;
	memset(&hints, 0, sizeof hints);	//set up holders
	hints.ai_family = AF_INET;		//IPv4         
	hints.ai_socktype = SOCK_STREAM;  	//TCP
	if((stat = getaddrinfo(node, port, &hints, &address)) != 0){	//create and validate the socket address structure
		fprintf(stderr,"Error: server port number %s\n", gai_strerror(stat));	//https://linux.die.net/man/3/gai_strerror
		exit(1);
	}
	return address;
}


//Create Socket Function (Beej)
	//pass in socket address struct
	//creates a socket and validates
int createSocket(struct addrinfo* address){           
	int sockfd;       //holds the socket file descriptor                                     
	if ((sockfd = socket(address->ai_family, address->ai_socktype, address->ai_protocol)) == -1){	//create the socket, save the returned desciptor, and validate the socket via address info of struct
		fprintf(stderr, "Error: server socket creation\n");
		exit(1);
	}
	return sockfd;
}


//Connect Socket Function (Beej)
	//pass in socket file descriptor and socket address struct
	//connects the socket and validates
void connectSocket(int sockfd, struct addrinfo* address){
	int stat;
	if ((stat = connect(sockfd, address->ai_addr, address->ai_addrlen)) == -1){	//connect socket and validate via address info of struct
		fprintf(stderr, "Error: server socket connection\n");
		exit(1);
	}
}


//Bind Socket Function (Beej)
	//pass in socket file descriptor and socket address struct
	//assigns the address to the socket
void bindSocket(int sockfd, struct addrinfo* address){                
	if (bind(sockfd, address->ai_addr, address->ai_addrlen) == -1) {	//assign address to the socket and validate
		close(sockfd);
		fprintf(stderr, "Error: server binding socket\n");
		exit(1);
	}
}


//Listen Socket Function (Beej)
	//pass in the socket file descriptor
	//sets up 10 incoming connections on the socket (10 chosen randomly from beej)
void listenSocket(int sockfd){         
	if(listen(sockfd, BACKLOG) == -1){	//listen to the socket and validate
		fprintf(stderr, "Error: server listen\n");
		close(sockfd);	//make sure to close the socket before exiting the program
		exit(1);
	}
}


//Download Directory List Function
	//Pass in pointer to structure used to hold the files, all the files need to be sent as one
	//return the number of files added to the structure
	// General Source: "The C Programming Language" Kernighan & Ritchie section 8
int downloadDirectoryList(char** package){    
	int counter = 0;	//file counter     
	DIR* dfd;  //dfd = directory for download                               
	struct dirent* dp;	//structure with (unique type) file serial number and (char) name of file
	dfd = opendir(".");	//open current working directory
	if (dfd){
		while ((dp = readdir(dfd)) != NULL){    //loop through all of the files in the directory 
			if (dp->d_type == DT_REG){	//if the file type of the active file is a regular file
				strcpy(package[counter], dp->d_name);	//add the file and the file name to the file structure
				counter++;	//increment counter to indicate a file was added and get ready to add another
			}
		}
		closedir(dfd);
	}
	return counter;
}


//Create Package function
	//creates null ptr (of ptr) of passed in size to hold the file descriptor list to be sent
	//"package" serves as the name of the "packaged" directory list
char **createPackage(int size){ 
	char **package = malloc(size*sizeof(char *));	//the package will point to the target list being sent
	int i;
	for(i = 0; i < size; i++){	//loop through and initialize/clear the package
		package[i] = malloc(MAXDATASIZE*sizeof(char));
		memset(package[i],0,sizeof(package[i]));
	}
	return package;
}


//Check Package for file function
	//pass in package, number of files in the package, and the file name to search for
	//looks for the file name in the passed in package
int checkPackage(char** package, int fileNumber, char* fileName){     
	int foundFlag = 0;                
	int i;
	for (i = 0; i < fileNumber; i++){	//loop through the package and search for the target file    
		if(strcmp(package[i], fileName) == 0){
			foundFlag = 1;	//set flag if found
		}
	}
	return foundFlag; 	//flag is an int but will serve as a boolean check  
}


//Send File Function
	//pass in client ipAddress, client port, and target file name
	//attempts to send desired data to client
void sendFile(char* ipAddress, char* port, char* fileName){  
	char data[2000];         //holder for data sent between server and client                                                             
	memset(data, 0, sizeof(data));          //clear holder
	int fd = open(fileName, O_RDONLY);    //open the target file (read only, no need to edit) to get the data  
	struct addrinfo* address = prepIPAddress(ipAddress, port);      //create the socket to send the data
	int packageSocket = createSocket(address);                                             
	connectSocket(packageSocket, address);	//connect the data socket                                           
	while (1) {
		int byteNumber = read(fd, data, sizeof(data) - 1);  //see if the file can fit in the data holder           
		if (byteNumber == 0){
			break;
		}
		if (byteNumber < 0) {
			fprintf(stderr, "Error: server read file\n");
			return;
		}
		void* dataPtr = data;	//pointer used to send data
		while (byteNumber > 0) {  //Beej referenced here, loop that "walks through" the data sending each byte
			int byteSent = send(packageSocket, dataPtr, sizeof(data),0);
			if (byteSent < 0) {	//validate that all of the data was sent
				fprintf(stderr, "Error: server writing\n");
				return;
			}
			byteNumber -= byteSent;
			dataPtr += byteSent;
		}
		memset(data, 0, sizeof(data)); 	//clear holder when done
	}
	
	memset(data, 0, sizeof(data));
	strcpy(data, "complete");	//send message to client to signal all the data was sent
	send(packageSocket, data, sizeof(data),0);
	close(packageSocket);    //close socket
	freeaddrinfo(address);
}


//Send Package (Directory List) Function
	//pass in client ipAddress, client port, package, and number of files in the directory
void sendPackage(char* ipAddress, char* port, char** package, int listNumber){    
	struct addrinfo* address = prepIPAddress(ipAddress, port);       //create the socket to send the data      
	int directorySocket = createSocket(address);
	connectSocket(directorySocket, address);	//connect the socket
	int i ;
	for (i = 0; i < listNumber; i++){        //loop through the package, sending each file name             
		send(directorySocket, package[i], MAXDATASIZE, 0);                
	}
	char* completed = "complete";	//send message to client to signal all the data was sent
	send(directorySocket, completed, strlen(completed), 0);
	close(directorySocket);		//close socket
	freeaddrinfo(address);
}


//Accept Connection Function (Beej)
	//pass in a file descriptor of a socket
	//function listens to commands from the client on the socket
void acceptConnection(int new_fd){	  
	char* pass = "pass"; 	//connection messages to client
	char* fail = "fail"; 
	char port[MAXDATASIZE];	//holders
	char ipAddress[MAXDATASIZE];
    char command[BUFFER];
	memset(port, 0, sizeof(port));	//clear holders
	memset(command, 0, sizeof(command));
	memset(ipAddress, 0, sizeof(ipAddress));              
	recv(new_fd, port, sizeof(port)-1, 0);	//get the port number
	send(new_fd, pass, strlen(pass), 0);	//send ack 
	recv(new_fd, command, sizeof(command)-1, 0);	//get the client command
	send(new_fd, pass, strlen(pass),0);		//send ack
	recv(new_fd, ipAddress, sizeof(ipAddress)-1,0);	//get the client ip address
	printf("New connection: %s\n", ipAddress);		//display successful connection
    
    if(strcmp(command, "g") == 0){    //client command to get a file         
		char** package = createPackage(BUFFER);	//create package of the directory list
		int fileNumber = downloadDirectoryList(package);  
		char fileName[MAXDATASIZE];
		memset(fileName, 0, sizeof(fileName));
		send(new_fd, pass, strlen(pass), 0);
		recv(new_fd, fileName, sizeof(fileName) - 1, 0);	//get the target file name from the client
		printf("File: %s requested \n", fileName);       
		int fileFlag = checkPackage(package, fileNumber, fileName);	//check the package for the target file name
		if(fileFlag){
			char* fileResponse = "found";	//if found let user and client know
			printf("Sending %s to client\n", fileName);
			send(new_fd, fileResponse, strlen(fileResponse), 0);
			char fileTarget[MAXDATASIZE];	//holder for target file name, needed to send the package
			memset(fileTarget, 0, sizeof(fileTarget));
			strcpy(fileTarget, "./");	//add path to start of holder
			char* filePtr = fileTarget + strlen(fileTarget);	//point to the holder
			filePtr += sprintf(filePtr, "%s", fileName);	//add the target file name to the holder via the pointer
			sendFile(ipAddress, port, fileTarget);	//initiate the function to send the file
		}        
        	else{		
			char* fileResponse = "File not found";	//if not found let the user and client know
			printf("Server failed to find file. Sending error...\n");
			send(new_fd, fileResponse, MAXDATASIZE, 0);
		}
		deletePackage(package, BUFFER);
	}
	
	else if(strcmp(command,"l") == 0){      //client command to get a list of the directory          
		char** package = createPackage(BUFFER); //create package of the directory list
		int fileNumber = downloadDirectoryList(package);
		send(new_fd, pass, strlen(pass),0);
		printf("Directory List Requested\n");
		printf("Sending directory list to %s \n", ipAddress);
		sendPackage(ipAddress, port, package, fileNumber);	//send the package
		deletePackage(package,BUFFER);
	 }	
	 
	else{	//else client command was invalid
		send(new_fd, fail, strlen(fail), 0);
		printf("Error: invalid client message\n");
	}

	printf("Server waiting... (Ctrl C to Quit)\n");
}


//Wait Function (Beej)
	//pass in socket file descriptor listening for connections
	//new socket connection created for all send and recv with connecting client
void wait(int sockfd){	               
	struct sockaddr_storage their_addr;		//client address
    socklen_t addr_size;	
	int new_fd;	//new socket file descriptor of connection for send and recv
	
	while(1){	//accept incoming connection (server loop until SIGINT)
		addr_size = sizeof(their_addr);		
		new_fd = accept(sockfd, (struct addrinfo *)&their_addr, &addr_size);
		if(new_fd == -1){			
			fprintf(stderr, "Error: server/client socket\n");	//validate but continue to let client know/ dont shut down entire server
			continue;
		}
		acceptConnection(new_fd);	//function to accept and relay message with client
		close(new_fd);	//be sure to close the newly created socket
	}
}


//Delete Package Function
	//pass in package and size of package
	//loops through the package, freeing memory
void deletePackage(char** package, int size){ 
	int i;
	for (i = 0; i < size; i++){
		free(package[i]);
	}
	free(package);
}


//Main Function
	//turns on the server and waits for a connection
int main(int argc, char *argv[]){
	if(argc != 2){                                                    
		fprintf(stderr, "Error: server arg number\n");	//validate argument number
		exit(1);
	}
	struct addrinfo* address = prepAddress(argv[1]);
	int sockfd = createSocket(address);
	bindSocket(sockfd, address);
	listenSocket(sockfd);
	printf("Server on port: %s\n", argv[1]);
	wait(sockfd);
	freeaddrinfo(address);
}

