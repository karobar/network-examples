#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define message "hello"

main(int argc, char *argv[])
{
  int port;
  char* ip;
  //First is the IP address of the machine your server is running on.
  //Second is the ("listening") TCP port the server is using.
  if (argc >= 2) 
  {
        ip = argv[1];
	port = atoi(argv[2]);
  }
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);  
  addr.sin_addr.s_addr = inet_addr(ip);
  //Allocate socket
  int Asocket;
  if((Asocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
  {
      printf("client failed to create socket...\n");
  }
  //Connect to server
  if(connect(Asocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) 
  {
    int errorNum = errno;
    printf("connection failed! Error Number = %i\n", errorNum);
  }
  //Assign a specific local address to the given socket
  bind(Asocket, (struct sockaddr *)&addr, sizeof(addr));
  printf("CLIENT: SENDING %s\n", message);
  //Transfer data
  send(Asocket, message, strlen(message), 0);
  int stillWaiting = 1;
  char received[512];
  recv(Asocket, received, sizeof received, 0);
  printf("CLIENT: RECEIVED %s\n", received);
  //Close socket
  close(Asocket);
}
