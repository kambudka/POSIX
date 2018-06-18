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


//compile with command 
//gcc -o main *.c -lpthread -lrt

/*semaphores*/
sem_t rope;
sem_t east_mutex;
sem_t west_mutex;
sem_t deadlock_protection;
sem_t counter;

/*global variables*/
int east = 0;
int west = 0;
int travel_time = 1;    
int westBab =0;
int eastBab =0;
int eastcnt=0, westcnt=0;

/*function prototypes*/
void crossing(int x);
void* east_side(void*);
void* west_side(void*);

/*main function*/
int main(int argc, char *argv[])
{ 
srand(time(NULL));  
    char c;
    int baboonCnt=10;
    char temp[20];

    sem_init(&rope,0,1);                        //ensure mutual exclusion on rope ownership
    sem_init(&east_mutex,0,1);                  //east side on travel
    sem_init(&west_mutex,0,1);                  //west side on travel
    sem_init(&deadlock_protection,0,1);         //used to prevent deadlocks while using semaphores
    sem_init(&counter,0,1);                     //ensure only 3 baboons are allowed on the rope

for(int i=0; i<baboonCnt;i++){
  int r = rand()%50;
  if(r<25){
eastBab++;
  
temp[i]='L';
  }
else{
westBab++;
  
temp[i]='R';
}
  }
  printf("%d ", baboonCnt);
printf("%d  >>>  <<< %d\n",eastBab, westBab);
    
    if ( argc == 2 )                    
    {
        int id[baboonCnt];
        pthread_t eastern[baboonCnt],western[baboonCnt];
        int eastid[eastBab], westid[westBab], i=0;
    for(i;i<baboonCnt;++i)
      {
        if(temp[i]=='L')
          {
        eastid[eastcnt]=i;
        printf("%d >>> / " ,i); 
        pthread_create(&eastern[eastcnt],NULL, (void *) &east_side,(void *) &eastid[eastcnt] );
        ++eastcnt;
          }
        else if(temp[i]=='R')
          {
        westid[westcnt]=i;
        printf("%d <<< / ",i);
        pthread_create(&western[westcnt],NULL, (void *) &west_side,(void *) &westid[westcnt] );
        ++westcnt;
          }
      }
int k =0;
    for(k;k<westBab;++k)
    {
        pthread_join(western[k],NULL); 
    }
    k=0;
    for(k;k<eastBab;++k)
    {
        pthread_join(eastern[k],NULL); 
    }
        //destroy all semaphores
        sem_destroy (&rope); 
        sem_destroy (&east_mutex);
        sem_destroy (&west_mutex);
        sem_destroy (&deadlock_protection);
        sem_destroy (&counter);
        return 0;
    }
    else
    {
        
    }
}
//thread handling the east to west to travel
void* east_side(void*arg)            
{
    int baboon = *(int*)arg;
    int on_rope;
    sem_wait(&deadlock_protection);
    sem_wait(&east_mutex);
    east++;
    if (east == 1)
    {
        sem_wait(&rope);
        printf("%d: waiting\n", baboon);
    }
    sem_post(&east_mutex);
    sem_post(&deadlock_protection);
    sem_wait(&counter);
    sem_getvalue(&counter, &on_rope);
    eastcnt--;
    printf("%d [>> %d >>] %d \n",eastcnt, baboon, westcnt);
    sem_post(&counter);
    sem_wait(&east_mutex);
    east--;
    if (east == 0)
        sem_post(&rope);
    sem_post(&east_mutex);
}

//thread handling west to east travel
void* west_side(void*arg)    
{
    int baboon = *(int*)arg;
    int on_rope;
    sem_wait(&deadlock_protection);
    sem_wait(&west_mutex);
    west++;
    if (west == 1)
    {
        sem_wait(&rope);
        printf("%d: waiting\n", baboon);
    }
    sem_post(&west_mutex);
    sem_post(&deadlock_protection);
    sem_wait(&counter);
    sem_getvalue(&counter, &on_rope);
    westcnt--;
    printf("%d [<< %d <<] %d \n",eastcnt, baboon, westcnt);
    sem_post(&counter);
    sem_wait(&west_mutex);
    west--;
    if (west == 0)
        sem_post(&rope);
    sem_post(&west_mutex);
}