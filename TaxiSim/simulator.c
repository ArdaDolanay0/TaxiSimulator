#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "simulator.h"
#include <sys/wait.h>

#include "taxi.c"
#include "display.c"
#include "dispatchCenter.c"

void spawnNewTaxi(DispatchCenter* dispatch, int location, int plate){
  Taxi *taxi = NULL;
  taxi = malloc(sizeof(Taxi));
  dispatch->taxis[dispatch->numTaxis] = taxi;
  dispatch->taxis[dispatch->numTaxis]->x= AREA_X_LOCATIONS[location];
  dispatch->taxis[dispatch->numTaxis]->y= AREA_Y_LOCATIONS[location];
  dispatch->taxis[dispatch->numTaxis]->currentArea = location;
  dispatch->taxis[dispatch->numTaxis]->plateNumber= plate;
  dispatch->taxis[dispatch->numTaxis]->pickupArea = -1;
  dispatch->taxis[dispatch->numTaxis]->dropoffArea= -1;
  dispatch->taxis[dispatch->numTaxis]->status = 0;
  dispatch->numTaxis++;


}
int main() {
  int location[10], pid[10], status,parent;
  DispatchCenter     ottawaDispatch;


  parent = getpid();

  srand(time(NULL));
  ottawaDispatch.numTaxis = 0;
  ottawaDispatch.numRequests = 0;


  // Create a taxi in a random area of the city.
  // ...
  for(int i=0;i<10;i++){
    //generates a psudo radom location
    location[i] = rand()%6;
    //calls the fucntion to create a new taxi
    spawnNewTaxi(&ottawaDispatch,location[i],i);
    //generates a new process
    if(getpid()==parent){
      pid[i] = fork();
    }
    
    if(pid[i]==0){
      printf("%d\n",i);

      printf("%d\n",getpid());

      //runs this method
      runTaxi(ottawaDispatch.taxis[i]);
      exit(0);
    }else if(parent=getpid()){
      //stores process id in taxi's pID
      ottawaDispatch.taxis[i]->pID = pid[i];
    }
  }





  // Spawn a thread to handle incoming requests
  // ...
  if(getpid()==parent){

    pthread_t threadRequests, threadDisplay;
    //creates a new thread to handle requests
    pthread_create(&threadRequests, NULL, handleIncomingRequests, &ottawaDispatch);
    //creates a new thread to handle display
    pthread_create(&threadDisplay, NULL, showSimulation, &ottawaDispatch);

    //joins the thread so main fucntion will not done until dispatch center is shutdwon
    pthread_join(threadRequests, NULL);


    // Spawn a thread to handle display
    // ...

    // Wait for the dispatch center thread to complete, from a STOP command
    // ...

    // Kill all the taxi processes
    // ...
    for(int i=0;i<ottawaDispatch.numTaxis;i++){
      kill(ottawaDispatch.taxis[i]->pID, SIGKILL);
    }

    // Free the memory
    // ...
    for(int i=0;i<ottawaDispatch.numTaxis;i++){
      Taxi *taxi = NULL;
      taxi = ottawaDispatch.taxis[i];
      free(taxi);
    }
    printf("Simulation complete.\n");
    exit(0);
  }
}
