#!/bin/python

#Kyle Dixon
#CS 372 Summer 2019
#Project 1 Client/Server Application
#Sources: https://realpython.com/python-sockets/
#http://floppsie.comp.glam.ac.uk/Glamorgan/gaius/wireless/5.html#1.%20Python%20Socket%20Programming
#https://docs.python.org/2/library/socket.html
#################

import sys
from socket import *


#Get Client Function
#pass the socket and the user name, returns the client name
def getClient(connection, user):
    clientName = connection.recv(4096)    #receive data with a max buffer of 4096 (should be a "relatively small power of two" -python.org)
    connection.send(user)   #send the user name to the socket
    return clientName

    
#Chat Function
#pass socket, client name, and user (server) name
def chat(newConnection, clientName, user):
    message = ""            
    while 1:              
        charIn = newConnection.recv(501)[0:-1]    #get all 500 characters (don't get the final character)    
        if charIn == "":    #test the incoming characters for client ending program
            print("Connection stopped")
            break
        print("{}> {}".format(clientName, charIn))   #print the message
        charOut = ""
        while len(charOut) > 500 or len(charOut) == 0:  #get outgoing characters
            charOut = raw_input("{}> ".format(user))
        if charOut == "\quit":  #test outgoing characters for server ending program
            print("Connection stopped")
            break
        newConnection.send(charOut) #send the message


#Start of Main Code
if __name__ == "__main__":    
    if len(sys.argv) != 2:                  #validate argument number
        print("Error: server arg number")
        exit(1)
    portNumber = sys.argv[1]                #get the passed in port number
    sock = socket(AF_INET, SOCK_STREAM)    #create socket (IPv4 and TCP)
    sock.bind(('', int(portNumber)))   #bind the socket
    sock.listen(1) #set the socket as a server socket
    user = ""   #get the user name (technically the server name)
    while len(user) > 10 or len(user) == 0:    #validate the length of the user name
        user = raw_input("Enter a user name 10 characters or less. ")
        print("Server ready.")
    while 1:    #while there is a client attempting to connect (main loop of server)
        connection, address = sock.accept()    #accept the socket connection         
        print("Connected!: {}".format(address)) #print connection message with address
        chat(connection, getClient(connection, user), user) #get the client name and initiate chat
        connection.close()  #close chat

