#!/bin/python

#Kyle Dixon
#CS 372 Summer 2019
#Project 2 Client/Server File Transfer
#Arguments (if command is l): 1 <SERVER_HOST>, 2 <SERVER_PORT>, 3 <COMMAND>, 4 <DATA_PORT>
#Arguments (if command is g): 1 <SERVER_HOST>, 2 <SERVER_PORT>, 3 <COMMAND>, 4 <FILENAME>, 5 <DATA_PORT>
#Sources: https://realpython.com/python-sockets/
#http://floppsie.comp.glam.ac.uk/Glamorgan/gaius/wireless/5.html#1.%20Python%20Socket%20Programming
#https://docs.python.org/2/library/socket.html
#https://docs.python.org/2/howto/sockets.html
#################

import sys
from socket import *

#Create Socket Function
    #creates a data socket and accepts a connection
    #returns socket
def createSocket():    
    if sys.argv[3] == "-l":
        numArguments = 4
    elif sys.argv[3] == "-g":                               
        numArguments = 5
    portData = int(sys.argv[numArguments])    #get the data port number 
    socketServer = socket(AF_INET, SOCK_STREAM)    #default address family and socket type
    socketServer.bind(('', portData))     #Specify that the socket is reachable by any address on the server port
    socketServer.listen(5)  #queue up 5 connect requests (normal max)
    socketData, address = socketServer.accept() #accept connection from outside
    return socketData

#Connection Function
    #creates a socket and connects to a server
    #returns socket
def connect():                                            
    server = sys.argv[1]+".engr.oregonstate.edu"    #get the server name
    portServer = int(sys.argv[2])   #get the server port                                     
    socketServ = socket(AF_INET,SOCK_STREAM)    #default address family and socket type             
    socketServ.connect((server, portServer))    #connect          
    return socketServ
    
#Get File Function
    #writes passed data to the file name 
def getFile(socketData):                  
    fileTarget = open(sys.argv[4], "w") #create a new file with the passed in name
    buffer = socketData.recv(2048)  #get the data
    while "complete" not in buffer:     #complete signifies the end of the stream
        fileTarget.write(buffer)
        buffer = socketData.recv(2048)
        
#Get List Function    
        #prints passed data to the screen
def getList(socketData):
    fileName = socketData.recv(100)     #get the data
    while fileName != "complete":      #complete signifies the end of the stream       
        print fileName
        fileName = socketData.recv(100)

#Get IP Function Source:
    #https://stackoverflow.com/questions/166506/finding-local-ip-addresses-using-pythons-stdlib
    #creates a socket to google DNS service and returns the socket name which is the local IP address
def getClientIP():
    s = socket(AF_INET, SOCK_DGRAM)    #default address family and socket type
    s.connect(("8.8.8.8", 80))
    return s.getsockname()[0]

#Get Data Function
        #pass in client socket
        #creates data socket and calls appropriate function
def getData(socketServ): 
    if sys.argv[3] == "-g":  
        portD = 5
    elif sys.argv[3] == "-l":
        portD = 4
    socketServ.send(sys.argv[portD])   #send data port to server
    socketServ.recv(1024)
    if sys.argv[3] == "-g":
        socketServ.send("g")
    else:
        socketServ.send("l") 
    socketServ.recv(1024)
    socketServ.send(getClientIP())     #send client IP address to server
    response = socketServ.recv(1024)
    if response == "fail":          #react to server                          
        print "Error: Invalid command"
        exit(1)
    if sys.argv[3] == "-g":
        socketServ.send(sys.argv[4])
        response = socketServ.recv(1024)
        if response != "found":     #react to server            
            print "Error: FILE NOT FOUND"
            return
    socketData = createSocket()     #Socket to receive data from server
    if(sys.argv[3] == "-g"):    
        getFile(socketData)
    elif sys.argv[3] == "-l":     
        getList(socketData)
    socketData.close()          #close the data socket as transfer is complete


#Start of Main Code 
if __name__ == "__main__":    
    #Perform argument validation
    if len(sys.argv) < 5 or len(sys.argv) > 6:  #5 or 6 arguments depending on command
        print "Error: Argument Number"
        exit(1) 
    elif (int(sys.argv[2]) < 1024 or int(sys.argv[2]) > 65535): #validate server port number range    
        print "Error: Server Port"
        exit(1)
    elif (sys.argv[1] != "flip1" and sys.argv[1] != "flip2" and sys.argv[1] != "flip3"):    #server must be a flip variation
        print "Error: Server Name"
        exit(1)
    elif (sys.argv[3] != "-g" and sys.argv[3] != "-l"):     #2 commands possible
        print "Error: Command"
        exit(1)
    elif (sys.argv[3] == "-l" and (int(sys.argv[4]) < 1024 or int(sys.argv[4]) > 65535)):    #validate data port number range  
        print "Error: Data Port"
        exit(1)
    elif (sys.argv[3] == "-g" and (int(sys.argv[5]) < 1024 or int(sys.argv[5]) > 65535)):    #validate data port number range      
        print "Error: Data Port"
        exit(1)
    socketServ = connect()     #create socket and connect to srever
    getData(socketServ)        #get data from server