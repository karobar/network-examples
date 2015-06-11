#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define DA_PORT 51717

//waits for the client to send a message and then responds if the message is 
//"hello", "goodbye", or "exit", then exits
void waitAndRespond(int socket) 
{
      struct sockaddr_in responseAddr;
      //Allocate a new socket when one connects
      int responseLen = sizeof(responseAddr);
      int connectSocket;
      if((connectSocket = accept(socket, (struct sockaddr *)&responseAddr, &responseLen)) < 0)
      {
	  printf("SERVER: accept() failed!\n");
      }
      char message[512];
      //receive the client's message
      recv(connectSocket, message, sizeof message, 0);
      printf("SERVER: RECEIVED %s\n",message);
      char *response;
      //choose a response to send
      if(strcmp(message, "hello")==0)
      {
	response = "world";
	printf("SERVER: SENDING %s\n",message);
	send(connectSocket, response, strlen(response), 0);
      }
      else if(strcmp(message, "goodbye")==0) 
      {
	response = "farewell";
	printf("SERVER: SENDING %s\n",message);
	send(connectSocket,  response, strlen(response), 0);
      }
      else if (strcmp(message, "exit")==0)
      {
	response = "ok";
	printf("SERVER: SENDING %s\n",message);
	send(connectSocket, response, strlen(response), 0);	
      }
      //Close client socket
      close(connectSocket);
}

main()
{
  //Allocate socket
  int Bsocket; 
  if((Bsocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  {
    printf("failed to create socket!\n");
  }  
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(DA_PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  //bind Bsocket to addr
  if(bind(Bsocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
      int errorNum = errno;
      printf("SERVER: bind() failed! Error number = %i\n", errorNum);      
  }
  //Wait for incoming connections
  if(listen(Bsocket, 32) < 0)
  {
      printf("listen failed!\n");
  }
  waitAndRespond(Bsocket);
  close(Bsocket);
}








