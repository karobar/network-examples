/////////////////////////////////////////////////////////////////////////////////
/*  CLIENT
/   CREATED: 9/9/2014
/   MODIFIED: 11/9/2014
/   Sends UDP datagrams to an IP ADDRESS and PORT, and receives any response ACKs
/   INPUTS: IP ADDRESS (string), PORT (int)
*/
/////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "header.h"

#define PACKET_LENGTH_IN_INTS 5
#define MY_PORT 40013
#define message "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."

void allocateSocket(int * daSocket) 
{
  if((*daSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP))<0)
  {
      printf("client failed to create socket...\n");
  }
  else 
    {
      printf("Socket %d successfully created!\n", *daSocket);
    }
  struct timeval tv;
  tv.tv_sec = 5; 
  tv.tv_usec = 0; 
  setsockopt(*daSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
}

void mapAddrToSocket(struct sockaddr_in* addr, int *  socket) 
{
   if (bind(*socket, (struct sockaddr *)addr, sizeof(struct sockaddr_in)) < 0) 
   {
     printf("CLIENT: failed to bind socket %d, error %s\n", *socket, strerror(errno));
   }
}

//First is the IP address of the machine your server is running on.
//Second is the ("listening") TCP port the server is using. 
void parseArgs(int argc, char *argv[], int * port, char ** ip) 
{
  if (argc >= 2) 
  {
        *ip = argv[1];
	*port = atoi(argv[2]);
  }
 
}

void initializeMyAddress(struct sockaddr_in addr) 
{
}

void printPacket(int * packet, int packetLength) 
{
  int lengthINT = packetLength / sizeof(int);  
  int j;
  for(j = 0; j < lengthINT; j++) {
    printf("%c ", (char)packet[j]);
  }
}

//Transfer data
void sendPackets(int socket, struct sockaddr_in *dest_addr,int sequenceIter, int window) 
{
  int endOfWave = window + sequenceIter;
  while(sequenceIter < endOfWave)
    {
      //printf("creating header...\n");
      header * daHeader;
      daHeader = (header *) malloc(sizeof (header));
      daHeader->sequenceNo = sequenceIter;
      daHeader->flags = 0;
      daHeader->payloadLength = 20;

      int totalPacketLength = sizeof(header) + daHeader->payloadLength;
      int * packet = malloc(totalPacketLength);
      packet[0] = daHeader->sequenceNo;
      packet[1] = daHeader->flags;
      packet[2] = daHeader->payloadLength;
      int j;
      int remainingInts = daHeader->payloadLength / sizeof(int);
      //      printf("remainingInts = %d\n", remainingInts);
      
      int messageSize = sizeof(message) / sizeof(message[0]);
      for(j = 0; j < remainingInts; j++) 
	{
	  if(sequenceIter*remainingInts+j < messageSize) {
	    packet[j+3] = (int)message[sequenceIter*remainingInts+j];
	  }
	  else {
	    packet[j+3] = 8;
	  }
      }

      //printf("packet created of size %d!\n", totalPacketLength);

      if (sendto(socket, packet, totalPacketLength, 0, (struct sockaddr *)dest_addr, sizeof(*dest_addr)) < 0)
	{
	  printf("CLIENT: packet failed to send...\n");
	}
      else
	{
	  //	  printf("\nCLIENT SENT: ");
	  //printPacket(packet, totalPacketLength);
	  //printf("\n");
	}
      sequenceIter = sequenceIter + 1;
    }
}

main(int argc, char *argv[])
{
  int port;
  char* ip;
  parseArgs(argc, argv, &port, &ip);

  struct sockaddr_in addr, remAddr;
  int socketLength = sizeof(addr);
  memset((char*)&addr, 0, socketLength);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(MY_PORT);  

  memset((char*)&remAddr, 0, socketLength);
  remAddr.sin_family = AF_INET;
  remAddr.sin_port = htons(port);  
  remAddr.sin_addr.s_addr = inet_addr(ip);

  int Asocket;
  allocateSocket(&Asocket);
  mapAddrToSocket(&addr, &Asocket);

  int sequenceIter = 0;
  int windowSize = 1;
  int messageSize = sizeof(message) / sizeof(message[0]);
  int msgSizeInPackets = (messageSize + PACKET_LENGTH_IN_INTS - 1) / PACKET_LENGTH_IN_INTS;
  printf("%d packets\n", msgSizeInPackets);
  int lastACK = 0;
  while (sequenceIter < msgSizeInPackets)
    {
      sendPackets(Asocket, &remAddr, sequenceIter, windowSize);      
      //printf("Packet wave ending on %d, waiting for ACKs\n", sequenceIter);
      //wait for ACKS
      int ACKbufLen = sizeof(int);
      int * ACKbuf = malloc(ACKbufLen);
      int i;
      int remAddrLen = sizeof(remAddr);
      for(i = 0; i < windowSize; i++) 
	{
	  if (recvfrom(Asocket, ACKbuf, ACKbufLen, 0,(struct sockaddr *) &remAddr, &remAddrLen) < 0) 
	    {
	      //printf("CLIENT: recvfrom failed!\n");
	    }
	  else 
	    {
	      //	      printf("CLIENT: RECEIVED ACK %d\n", ACKbuf[0]);
	      // received ack, but is it the one we want?
	      if (ACKbuf[0] != lastACK) 
		{
		  windowSize = windowSize / 2;
		  sequenceIter = lastACK + 1;
		}
	      lastACK = ACKbuf[0];
	    }
	}
      sequenceIter = lastACK + 1;
      windowSize++;
      //printf("window size now %d\n", windowSize);
    }
  close(Asocket);
}
