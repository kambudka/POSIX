#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>

#define N 10
#define L 20

pthread_t myThread[N];
pthread_mutex_t mutex, mAC, mBC;

int counter = 0;
int ACount = 0, BCount = 0, MACount = 0, MBCount = 0;
int temp[N];

void *Crossing(void *arg)
{
        while(1){
while(!wejscie)
wait(zmienna,mut)
        
        pthread_mutex_unlock(&mutex);
        int r = rand()%3;
        sleep(r);
        }

        pthread_exit((void*) 0);
}

int main ()
{
        void *status;
        pthread_mutex_init(&mutex, NULL);
        srand(time(NULL)); 
        for(int i=0; i<N;i++){
            int r = rand()%100;
            if(r<55){ 
                temp[i]=0;
                ACount++;
            }
            else{
                temp[i]=1;
                BCount++;
            }
        }
for(int i=0; i<N;i++){
    printf("%d-%d  ",i, temp[i]);
}
        /* Each thread has its own  set of data to work on. */
        for(long i=0; i < N; i++)
                pthread_create(&myThread[i], NULL, Crossing, (void *)i);

        /* Wait on child threads */
        while(1);

        /* Results and cleanup */

        pthread_mutex_destroy(&mutex); pthread_exit(NULL);
}