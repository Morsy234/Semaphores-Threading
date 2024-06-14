#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define BUFFER_SIZE 5

int counter = 0; //our integer counter which is increamented


//semaphores
sem_t sem_counter;
sem_t empty;
sem_t full;
sem_t singlebuffer;



// generate random number between 2 numbers
int rand_num(int min, int max)
{
    return min + rand() % (max - min + 1);
}


void *mCounter(void *arg) {
  int threadId = *(int *)arg;

  // sleep for random interval of time between 1 and 100

  while (1) {
    sleep(rand_num(1,40));
    printf("Counter thread %d: received a message\n", threadId);

    int sem_counter_value;
    sem_getvalue(&sem_counter, &sem_counter_value);
    if (sem_counter_value <= 0)
        {
         printf("Counter thread %d: waiting to write\n", threadId);
        }

    //printf("Counter thread %d: waiting to write\n", threadId);
    sem_wait(&sem_counter);//waiting

    //critical section
    counter++;


    printf("Counter thread %d: now adding to counter, counter value=%d\n", threadId, counter);
    sem_post(&sem_counter);
  }

  return NULL;
}





// Functions to manipulate the buffer

typedef struct {
    int buffer[BUFFER_SIZE];
    int front, rear;
} Bounded_Buffer;


void initialize_buffer(Bounded_Buffer *buffer) {
    buffer->front =0;
    buffer->rear = 0;

}

void insert_into_buffer(Bounded_Buffer *buffer, int value) {
    buffer->buffer[buffer->rear] = value;
    buffer->rear = (buffer->rear + 1) % BUFFER_SIZE;

}

int extract_from_buffer(Bounded_Buffer *buffer) {
    int value = buffer->buffer[buffer->front];
    buffer->front = (buffer->front + 1) % BUFFER_SIZE;

    return value;
}

//our buffer
Bounded_Buffer buffer_shared;
























/*// Functions to manipulate the buffer
int in_index = -1;
int out_index = -1;


void insert_into_buffer(int value) {
    if (out_index == BUFFER_SIZE - 1 ) // buffer is full (out_index + 1) % BUFFER_SIZE == in_index; or (in_index + 1) % BUFFER_SIZE == out_index;
    {
        printf("Buffer is full (overflow)\n");
    }
    else
    {
        if(in_index==-1)
        {
            in_index=0;
            out_index++;
            buffer[in_index] = value;
            //in_index = (in_index + 1) % BUFFER_SIZE;
        }

    }
}

int extract_from_buffer() {
    if (in_index=-1 || in_index > out_index ) // buffer is empty  in_index == out_index
    {
        printf("Underflow\n");
        return -1; // Return a special value or handle the underflow case appropriately
    }
    else
    {
        /*int value = buffer[out_index];
        out_index = (out_index + 1) % BUFFER_SIZE;
        return value;*/


        //in_index++;
    //}
//}


//produce
void *mMonitor() {

  while (1) {
    sleep(rand_num(1,40));

    //produce (counter-monitor part)
    int sem_counter_value_monitor;
    sem_getvalue(&sem_counter, &sem_counter_value_monitor);
    if (sem_counter_value_monitor <= 0)
    {
        printf("Monitor thread: waiting to read counter\n");
    }

    //printf("monitor thread: waiting to read counter\n");
    sem_wait(&sem_counter);
    //critical section
    int counter_value = counter;
    printf("monitor thread: reading a count value of %d\n", counter_value);//or counter
    counter = 0;//reset the counter
    sem_post(&sem_counter);

    //--------------------------------------------------------------------------------------------------//


    int empty_value;
    sem_getvalue(&empty, &empty_value);
    if (empty_value <= 0)
    {
        printf("monitor thread: buffer full!!\n");
    }


    sem_wait(&empty);
    sem_wait(&singlebuffer);
    //critical section
    insert_into_buffer(&buffer_shared,counter_value);


    printf("monitor thread: writing to buffer at position %d\n", buffer_shared.rear);



    sem_post(&singlebuffer);
    sem_post(&full);
  }

  return NULL;
}


//consume
void *mCollector()
{
    while (1)
    {
        sleep(rand_num(1,40)+2);//!!!!

        int full_value;
        sem_getvalue(&full, &full_value);
        if (full_value <= 0) //if the semaphore is zero or negative then nothing is in buffer
        {
            printf("collector thread:nothing in the buffer!!\n");
        }

        sem_wait(&full);
        sem_wait(&singlebuffer);

        //critical section

        printf("collector thread: reading from the buffer at position %d\n", buffer_shared.rear);
        extract_from_buffer(&buffer_shared);

        sem_post(&singlebuffer);
        sem_post(&empty);
    }
}






int main()
{

  // Initialize semaphores
  sem_init(&sem_counter, 0, 1);
  sem_init(&singlebuffer, 0, 1);
  sem_init(&full, 0, 0);
  sem_init(&empty, 0, BUFFER_SIZE);



  printf("Enter number of threads (between 5 and 10): ");
  const int counters_num;
  scanf("%d", &counters_num);

  //const int counters_num = 5;

  // Create mCounter threads

  //array of threads for counter thread
  pthread_t counter_threads[counters_num];
  int counterThreadIds[counters_num];//the thread number that will be passed as argument
  for (int i = 0; i < counters_num; i++) {
    counterThreadIds[i] = i;
    pthread_create(&counter_threads[i], NULL, mCounter, &counterThreadIds[i]);
  }


  // Create mMonitor thread
  pthread_t monitor_thread;
  pthread_create(&monitor_thread, NULL, mMonitor, NULL);

  // Create mCollector thread
  pthread_t collector_thread;
  pthread_create(&collector_thread, NULL, mCollector, NULL);





  // Wait for all threads to finish
  for (int i = 0; i < counters_num; i++) {
    pthread_join(counter_threads[i], NULL);
  }
  pthread_join(monitor_thread, NULL);
  pthread_join(collector_thread, NULL);



  // Destroy semaphores
  sem_destroy(&sem_counter);
  sem_destroy(&singlebuffer);
  sem_destroy(&full);
  sem_destroy(&empty);

    return 0;
}
