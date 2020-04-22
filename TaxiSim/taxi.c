#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


// The dispatch center server connection ... when made
int                 clientSocket;  // client socket id
struct sockaddr_in  clientAddress; // client address



// Set up a client socket and connect to the dispatch center server.  Return -1 if there was an error.
int connectToDispatchCenter(int *sock,  struct sockaddr_in *address) {
    int status;
    char returnVal =0;
    *sock = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
    if(*sock<0){
    //  printf("Could not open a taxi socket!\n");
      returnVal=-1;
    }

    memset(address,0,sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = inet_addr(SERVER_IP);
    address->sin_port=htons((unsigned short)SERVER_PORT);

    status=connect(*sock,(struct sockaddr*)address,sizeof(*address));
    if(status<0){
      returnVal=-1;
      //printf("Taxi Could not establish connection with server!\n");
    }
    return returnVal;
}



//sends a requests to server for a customer
void sendTaxiRequest(){
  //printf("Sending a customer Request\n");
  char buffer[30];

  buffer[0]=REQUEST_CUSTOMER;
  send(clientSocket,buffer,strlen(buffer),0);

}

//Proccess the response from server to decide if taxi is in same location as customer and sets the taxi values accordingly
void ProcessServerResponse(Taxi *taxi){
  char buffer[30];
  int bytesRcv;
  //printf("Processing Response From Server\n");
  bytesRcv = recv(clientSocket, buffer, sizeof(buffer), 0);
  buffer[bytesRcv] = 0;
  if(buffer[0]==YES){
    if(taxi->currentArea==buffer[1]){
      //printf("Taxi is in same location as customer\n");
      taxi->pickupArea=buffer[1];
      taxi->dropoffArea=buffer[2];
      taxi->status = DROPPING_OFF;
      taxi->eta = TIME_ESTIMATES[taxi->pickupArea][taxi->dropoffArea];

    }else{
      //printf("Taxi is not in the same location as customer going to pick the customer\n");
      taxi->pickupArea=buffer[1];
      taxi->dropoffArea=buffer[2];
      taxi->status = PICKING_UP;
      taxi->eta = TIME_ESTIMATES[taxi->currentArea][taxi->dropoffArea];
    }
  }
}
//sends messages to the server regarding the postion of the taxi, its dropoffarea and status
void sendUpdate(Taxi *taxi){
  char buffer[30];
  buffer[0]=UPDATE;
  buffer[1] = taxi->plateNumber;
  buffer[2] = taxi->status;
  buffer[3] = taxi->dropoffArea;
  send(clientSocket,buffer,sizeof(buffer),0);
  int x[2];
  x[0]=taxi->x;
  x[1]=taxi->y;

  send(clientSocket,x,sizeof(x),0);

}
//moves the taxi to the desired location
void moveTaxi(Taxi *taxi){
//  printf("I am moving\n");
  int deltaX,deltaY;
  if(taxi->status==PICKING_UP){
    deltaX = (AREA_X_LOCATIONS[taxi->pickupArea]-AREA_X_LOCATIONS[taxi->currentArea])/TIME_ESTIMATES[taxi->pickupArea][taxi->currentArea];
    deltaY = (AREA_Y_LOCATIONS[taxi->pickupArea]-AREA_Y_LOCATIONS[taxi->currentArea])/TIME_ESTIMATES[taxi->pickupArea][taxi->currentArea];
  }else{
    deltaX = (AREA_X_LOCATIONS[taxi->dropoffArea]-AREA_X_LOCATIONS[taxi->pickupArea])/TIME_ESTIMATES[taxi->dropoffArea][taxi->pickupArea];
    deltaY = (AREA_Y_LOCATIONS[taxi->dropoffArea]-AREA_Y_LOCATIONS[taxi->pickupArea])/TIME_ESTIMATES[taxi->dropoffArea][taxi->pickupArea];
  }
  taxi->x=(taxi->x+deltaX);
  taxi->y=(taxi->y+deltaY);
  //decreases the estimated time
  taxi->eta = taxi->eta-0.16;
}
//Checks if the taxi has arrived at the desired location
void checkIfTaxiArrived(Taxi *taxi){
  if(taxi->eta<=0){
    //printf("I have arrived at Location to: ");

    //if taxi satus us dropping off become avalible again
    if(taxi->status==DROPPING_OFF){
      //printf("Drop\n");
      taxi->x= AREA_X_LOCATIONS[taxi->dropoffArea];
      taxi->y=AREA_Y_LOCATIONS[taxi->dropoffArea];
      taxi->status = AVAILABLE;
      taxi->pickupArea= UNKNOWN_AREA;
      taxi->currentArea= taxi->dropoffArea;
      taxi->dropoffArea= UNKNOWN_AREA;
    //if taxi status was picking up, go to drop off mode
    }else if(taxi->status==PICKING_UP){
      //printf("Pick\n");
      taxi->x=AREA_X_LOCATIONS[taxi->pickupArea];
      taxi->y=AREA_Y_LOCATIONS[taxi->pickupArea];
      taxi->status = DROPPING_OFF;
      taxi->eta = TIME_ESTIMATES[taxi->pickupArea][taxi->dropoffArea];
      taxi->currentArea= UNKNOWN_AREA;

    }
  }
}

// This code runs the taxi forever ... until the process is killed
void runTaxi(Taxi *taxi) {
  // Copy the data over from this Taxi to a local copy

  Taxi   thisTaxi;
  thisTaxi.plateNumber = taxi->plateNumber;
  thisTaxi.currentArea = taxi->currentArea;
  thisTaxi.x = taxi->x;
  thisTaxi.y = taxi->y;
  thisTaxi.status = AVAILABLE;
  thisTaxi.pickupArea = UNKNOWN_AREA;
  thisTaxi.dropoffArea = UNKNOWN_AREA;
  thisTaxi.eta = 0;

  // Go into an infinite loop to request customers from dispatch center when this taxi is available
  // as well as sending location updates to the dispatch center when this taxi is picking up or dropping off.

  while(1) {
    // WRITE SOME CODE HERE
    if(connectToDispatchCenter(&clientSocket, &clientAddress)==0){
      if(thisTaxi.status==AVAILABLE){
        //printf("Checked if Taxi is avliable!\n");
        //sends a customer request
        sendTaxiRequest();
        //proccess the reviced message from server
        ProcessServerResponse(&thisTaxi);

      }else if(thisTaxi.status==DROPPING_OFF||thisTaxi.status==PICKING_UP){
        //printf("Taxi is either Dropping Off or Picking Up\n");
        //sends an update to server
        sendUpdate(&thisTaxi);
        //changes taxi x and y
        moveTaxi(&thisTaxi);
        //checks if the taxi has arrived
        checkIfTaxiArrived(&thisTaxi);
      }
      // A delay to slow things down a little
    }
    usleep(50000);
  }
}
