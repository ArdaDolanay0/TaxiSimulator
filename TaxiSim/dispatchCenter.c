#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>


// Initialize the dispatch center server by creating the server socket, setting up the server address,
// binding the server socket and setting up the server to listen for taxi and customer clients.
void initializeDispatchServer(int *serverSocket, struct sockaddr_in  *serverAddress) {

  int status;
  *serverSocket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
  if(*serverSocket<0){
    printf("Could not open server!\n");
    exit(-1);
  }
  memset(serverAddress,0,sizeof((*serverAddress)));
  (*serverAddress).sin_family=AF_INET;
  (*serverAddress).sin_addr.s_addr=htons(INADDR_ANY);
  (*serverAddress).sin_port=htons((unsigned short)SERVER_PORT);
  status= bind((*serverSocket),(struct sockaddr *)serverAddress,sizeof(*serverAddress));
  if(status<0){
    printf("Could not bind server socket!\n");
    exit(-1);
  }
  status= listen(*serverSocket,MAX_TAXIS);
  if(status<0){
    printf("Could not listen for taxis!\n");
    exit(-1);
  }

}




// Handle client requests coming in through the server socket.  This code should run
// indefinitiely.  It should wait for a client to send a request, process it, and then
// close the client connection and wait for another client.  The requests that may be
// handled are SHUTDOWN (from stop.c), REQUEST_TAXI (from request.c) and UPDATE or
// REQUEST_CUSTOMER (from taxi.c) as follows:

//   SHUTDOWN - causes the dispatch center to go offline.

//   REQUEST_TAXI - contains 2 additional bytes which are the index of the area to be
//                  picked up in and the index of the area to be dropped off in. If
//                  the maximum number of requests has not been reached, a single YES
//                  byte should be sent back, otherwise NO should be sent back.

//   REQUEST_CUSTOMER - contains no additional bytes.  If there are no pending customer
//                      requests, then NO should be sent back.   Otherwise a YES should
//                      be sent back followed by the pickup and dropoff values for the
//                      request that has been waiting the logest in the queue.

//   UPDATE - contains additional bytes representing the taxi's x, y plate, status and
//            dropoff area.  Nothing needs to be sent back to the taxi.

void *handleIncomingRequests(void *d) {
  DispatchCenter  *dispatchCenter = d;
  dispatchCenter->online=1;
  printf("Hello!");
  int  serverSocket, clientSocket, addressSize, bytesRcv;
  int numRequests =0;
  int shutDown =0;
  char buffer[30];
  int integerBuffer[2];
  char response[30];
  struct sockaddr_in  serverAddress,clientAddress;

  // Initialize the server
  initializeDispatchServer(&serverSocket, &serverAddress);

  // REPLACE THE CODE BELOW WITH YOUR OWN CODE
  printf("Making Server.\n");

  while (shutDown==0) {

    addressSize = sizeof(clientAddress);
    clientSocket = accept(serverSocket,(struct sockaddr *)&clientAddress, &addressSize);
    //if sevre rhas made connection with client
    if(clientSocket>0){
        //gets a message from the client
        bytesRcv = recv(clientSocket,buffer,sizeof(buffer),0);
        buffer[bytesRcv]=0;
        //printf("SERVER:Recived client connection.\n");
        //if its a shutdown request
        if(buffer[0]==SHUTDOWN){
          //closes down the server
          dispatchCenter->online =0;
          printf("Closing server.\n");
          close(clientSocket);
          close(serverSocket);
          shutDown =1;

          //if the customer has requested a taxi
        }else if(buffer[0]==REQUEST_TAXI){


          if(dispatchCenter->numRequests==MAX_REQUESTS){
            //send a no response if number of requests is too high
            response[0]=NO;
            send(clientSocket,response,strlen(response),0);
          }else{
            //store the request in the dispatch center
            response[0]=YES;
            send(clientSocket,response,strlen(response),0);
            dispatchCenter->requests[dispatchCenter->numRequests].pickupLocation= buffer[1];
            dispatchCenter->requests[dispatchCenter->numRequests].dropoffLocation= buffer[2];
            dispatchCenter->numRequests++;
          }
          close(clientSocket);

          // if a taxi has requested a customer
        }else if(buffer[0]==REQUEST_CUSTOMER){
          //if there are no requests respond with a NO
          if(dispatchCenter->numRequests==0){
            response[0]=NO;
            send(clientSocket,response,strlen(response),0);

          }else{
            //if there are requests send a yes to taxi with the oldest requests pickup and dropoff location
            response[0]=YES;
            response[1]=dispatchCenter->requests[0].pickupLocation;
            response[2]=dispatchCenter->requests[0].dropoffLocation;
            //decrease the number of requests in the dispatch center by one and move the all left by one
            for(int i=0;i<dispatchCenter->numRequests;i++){
              if(i+1<dispatchCenter->numRequests)
                dispatchCenter->requests[i]=dispatchCenter->requests[i+1];
            }
            dispatchCenter->numRequests--;
            send(clientSocket,response,strlen(response),0);
          }
          close(clientSocket);
          //if the taxi has requested an  update regarding its postion
        }else if(buffer[0]==UPDATE){

          //find the right taxi to update via its plateNumber
          for(int i=0;i<dispatchCenter->numTaxis;i++){

            if(dispatchCenter->taxis[i]->plateNumber==buffer[1]){
              //revice its x and y cordinates
              bytesRcv = recv(clientSocket,integerBuffer,sizeof(integerBuffer),0);
              //update the required information
              dispatchCenter->taxis[i]->x = integerBuffer[0];
              dispatchCenter->taxis[i]->y = integerBuffer[1];
              dispatchCenter->taxis[i]->status=buffer[2];
              dispatchCenter->taxis[i]->dropoffArea=buffer[3];
              break;
            }
          }
          close(clientSocket);

        }else{
          printf("Invalid Request: %d\n",buffer[0]);
          close(clientSocket);
        }
    }
  }
}
