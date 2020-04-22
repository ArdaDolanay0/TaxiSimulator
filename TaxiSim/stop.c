#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"


void main() {
  int clientSocket;
  struct sockaddr_in serverAddress;
  int status,bytesRcv;
  char buffer[30];
  buffer[0]=SHUTDOWN;


  clientSocket = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);

  memset(&serverAddress,0,sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
  serverAddress.sin_port=htons((unsigned short)SERVER_PORT);

  status=connect(clientSocket,(struct sockaddr*)&serverAddress,sizeof(serverAddress));
  if(status<0){
    printf("Could not establish connection with server!\n");
    exit(-1);
  }
  send(clientSocket,buffer,strlen(buffer),0);

  close(clientSocket);
  // Contact the dispatch center and ask it to shut down
  // Create socket

}
