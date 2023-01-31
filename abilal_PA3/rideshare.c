
// Ahmet Bilal Yildiz

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>

// global variables

int waitingFanA, waitingFanB;
sem_t sem_A, sem_B;
pthread_mutex_t Mutex;
pthread_barrier_t Barrier;


void *fan_method_A (void *arg){

  pthread_mutex_lock (&Mutex);	// locking the mutex

  printf ("Thread ID: %ld, Team: A, I am looking for a car.\n", pthread_self ()); //display init message

  waitingFanA++; // update the waiting info
  bool isCaptain; // initialize the isCaptain that is true if the current fan is the captain and false otherwise

  if (waitingFanA == 4){  // 4 team A fan condition

      isCaptain = true;
      waitingFanA -= 4;	// updating the waiting fan information
      
      // opening the waiting sems
      sem_post (&sem_A);
      sem_post (&sem_A);
      sem_post (&sem_A);

    }
  else if ((waitingFanA == 3 || waitingFanA == 2) && (waitingFanB >= 2)){ // 2 team A and 2 team B condition

      isCaptain = true;
      waitingFanA -= 2;
      waitingFanB -= 2;
      
     // opening the waiting sems
      sem_post(&sem_A);
      sem_post(&sem_B);
      sem_post(&sem_B);

    }
    else { // if the current fan must wait
        
        isCaptain = false;
        pthread_mutex_unlock(&Mutex);
        
        sem_wait(&sem_A);
    }
    
    printf("Thread ID: %ld, Team: A, I have found a spot in a car\n", pthread_self()); // display mid message
    
    pthread_barrier_wait(&Barrier); // ensure all the fan threads write their message
    
    if(isCaptain){ // if the fan is captain give the message
        
        printf("Thread ID: %ld, Team: A, I am the captain and driving the car\n", pthread_self()); // display the end message
        isCaptain = false;
        pthread_mutex_unlock(&Mutex); // unlock the mutex
        
    }

  return NULL;
}


void *fan_method_B (void *arg){

  pthread_mutex_lock (&Mutex);	// locking the mutex

  printf ("Thread ID: %ld, Team: B, I am looking for a car.\n", pthread_self ()); // display init message

  waitingFanB++; // update the waiting info
  bool isCaptain;// initialize the isCaptain that is true if the current fan is the captain

  if (waitingFanB == 4){ // 4 team B fan condition

      isCaptain = true;
      waitingFanB -= 4;	// updating the waiting fan information
      
      // opening the waiting sems
      sem_post (&sem_B);
      sem_post (&sem_B);
      sem_post (&sem_B);

    }
  else if ((waitingFanB == 3 || waitingFanB == 2) && (waitingFanA >= 2)){ // 2 team A and 2 team B condition

      isCaptain = true;
      waitingFanB -= 2;
      waitingFanA -= 2;
      
     // opening the waiting sems
      sem_post(&sem_B);
      sem_post(&sem_A);
      sem_post(&sem_A);

    }
    else { // if the current fan must wait
        
        isCaptain = false;
        pthread_mutex_unlock(&Mutex); 
        
        sem_wait(&sem_B);
    }
    
    printf("Thread ID: %ld, Team: B, I have found a spot in a car\n", pthread_self()); // display mid message
    
    pthread_barrier_wait(&Barrier); // ensure all the fan threads write their message
    
    if(isCaptain){ // if the fan is captain give the message
        
        printf("Thread ID: %ld, Team: B, I am the captain and driving the car\n", pthread_self()); // display end message
        isCaptain = false;
        pthread_mutex_unlock(&Mutex); // unlock the mutex
        
    }

  return NULL;
}


int main (int argc, char *argv[]){

  int numA, numB; // initializing numA and numB
  
  // getting numA and numB from the command line so the function
  numA = atoi(argv[1]);
  numB = atoi(argv[2]);

  if ((numA % 2 != 0) || (numB % 2 != 0) || ((numA + numB) % 4 != 0)){ // if the numbers are invalid the main terminates

     printf ("The main terminates...\n");
  }

  else{	// if numbers are valid main thread continues

      // initializing the semaphores
      sem_init (&sem_A, 0, 0);
      sem_init (&sem_B, 0, 0);

      // initializing the barrier for 4 threads
      pthread_barrier_init (&Barrier, NULL, 4);

      // initialize a thread array and all child threads
      pthread_t *thread_list = (pthread_t *) malloc ((numA + numB) * sizeof (pthread_t));

      // creating all child threads

      for (int i = 0; i < numA; i++){

	    pthread_create (&thread_list[i], NULL, fan_method_A, NULL);

	  }
      for (int i = numA; i < (numA + numB); i++){

	    pthread_create (&thread_list[i], NULL, fan_method_B, NULL);

	  }

      // join all threads

      for (int i = 0; i < (numA + numB); i++){

	    pthread_join (thread_list[i], NULL);

	  }

      free (thread_list);	// free the dynamically allocated memory
      printf ("The main terminates...\n");

  }

  return 0;
}

