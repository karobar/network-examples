/////////////////////////////////////////////////////////////////////////////////
/*  SERVER
/   CREATED: 9/9/2014
/   MODIFIED: 9/10/2014
/   Receives UDP datagrams, and can send a response if the message text is 
/   "hello", "goodbye", or "exit"
/   INPUTS: IP ADDRESS (string), PORT (int)
*/
/////////////////////////////////////////////////////////////////////////////////

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "header.h"

#define DA_PORT 50003
#define PACKET_LENGTH 32

void printPacket(int * packet, int packetLength)
{
  int lengthINT = packetLength / sizeof(int);
  int j;
  for(j = 0; j < lengthINT-3; j++) {
    printf("%c ", (char)packet[j+3]);
  }
}

void sendACK(int socket, int sequenceNum,  struct sockaddr_in * respSocket) 
{
  //Allocate a new socket when one connects
  int respBufSize = sizeof(int);
  int * respBuf = malloc(respBufSize);
  respBuf[0] = sequenceNum; 
    
  //ssize_t sendto(int socket, const void *message, size_t length,
  //		 int flags, const struct sockaddr *dest_addr,
  //		 socklen_t dest_len);
  if(sendto(socket, respBuf, respBufSize, 0, (struct sockaddr *)respSocket, sizeof(*respSocket)) < 0) 
    {
      printf("SERVER: error in sendto, error %s\n", strerror(errno));
    }
  else 
    {
      //      printf("SERVER: SENDING ACK %d\n",respBuf[0]); 
    }
}

header parsePacket(int * packet) 
{
  header daHeader;
  daHeader.sequenceNo = packet[0];
  daHeader.flags = packet[1];
  daHeader.payloadLength = packet[2];
  return daHeader;
}

void initializeSocket(int * sockNum)
{
  if((*sockNum = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
      printf("failed to create socket!\n");
    }
} 

main()
{
  int Bsocket; 
  initializeSocket(&Bsocket);

  int buf[PACKET_LENGTH];
  //addr is this server's address,
  struct sockaddr_in addr, remAddress;
  int slen = sizeof(remAddress);
  
  memset((char *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(DA_PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  remAddress.sin_family = AF_INET;

  //bind Bsocket to addr
  if(bind(Bsocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    { 
      int errorNum = errno;
      printf("SERVER: bind() failed! Error number = %i\n", errorNum);      
    }
  int lastGoodPacket = -1;

  time_t t;
  srand((unsigned)time(&t));
  //Wait for incoming connections
  while(1)  
  {
    if(recvfrom(Bsocket,buf,PACKET_LENGTH, 0, (struct sockaddr *)&remAddress, &slen) < 0)
    {
	printf("SERVER: recvfrom failed!\n");
    }
    header packetHeader = parsePacket(buf);
    if(packetHeader.sequenceNo == 0)   //New client-server interaction
      {
	lastGoodPacket = -1;
	//printf("resetting expected sequence number!\n");
      }
    //simulate dropped packets with random numbers
    int randomNum = rand() % 100;
    //printf("randomNum = %d\n", randomNum);
    if(lastGoodPacket+1 == packetHeader.sequenceNo && randomNum > 10) 
      {
	printf("SERVER RECEIVED: ");
	printPacket(buf, PACKET_LENGTH);
	printf("\n");
	lastGoodPacket++;
	sendACK(Bsocket, lastGoodPacket, &remAddress);
      }
    else if(randomNum > 5)
      {
	//printf(".");
	sendACK(Bsocket, lastGoodPacket, &remAddress);
      }
  }
  close(Bsocket);
}








