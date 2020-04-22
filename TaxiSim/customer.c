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

// This represents a customer request.  There should be two command line arguments representing the
// pickup location and dropoff location.  Each should be a number from 0 to the NUM_CITY_AREAS.
void main(int argc, char *argv[]) {
  int                 clientSocket;  // client socket id
  struct sockaddr_in  serverAddress; // client address
  int status,bytesRcv;
  char buffer[30];
  //stores the values to be sent to server
  buffer[0]=REQUEST_TAXI;
  buffer[1]=atoi(argv[1]);
  buffer[2]=atoi(argv[2]);

  //opes a socket
  clientSocket = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
  //stores internet address information
  memset(&serverAddress,0,sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
  serverAddress.sin_port=htons((unsigned short)SERVER_PORT);

  //makes connection with server
  status=connect(clientSocket,(struct sockaddr*)&serverAddress,sizeof(serverAddress));
  if(status<0){
    printf("Could not establish connection with server!\n");
  }
  //sends a buffer to server
  send(clientSocket,buffer,strlen(buffer),0);
  //gets a response from server
  bytesRcv = recv(clientSocket, buffer, strlen(buffer), 0);
  buffer[bytesRcv] = 0;
  //if the number of requests are higher than maximum
  if(buffer[0]==NO){
    printf("Failed Sending Client Request.\n");
  }

  close(clientSocket);


}
