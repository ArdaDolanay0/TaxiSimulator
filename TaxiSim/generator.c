#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "simulator.h"

void main() {
  // Set up the random seed
  int dropLocation, pickLocation;
  char buff[30];
  srand(time(NULL));

  while(1) {
    //generates a pseudo random dropoff and pickup location
    dropLocation = rand()%6;
    pickLocation = rand()%6;
    //makes a system call to customer if these locations are no the same
    if(dropLocation!=pickLocation){
      //printf("%d=%d\n",pickLocation,dropLocation);
      snprintf(buff,sizeof(buff),"./customer %d, %d",pickLocation,dropLocation);

      system(buff);
    }

    // ...
    // add code here
    // ...

    usleep(250000);   /// do not remove this
  }
}
