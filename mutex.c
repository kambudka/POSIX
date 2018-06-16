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
#include <string.h>

#define N 10
#define L 20

typedef struct KOLEJKA Kolejka;
struct KOLEJKA{
	int* numer;
	Kolejka* next;
};     

Kolejka * start = NULL;

unsigned Ile(){ 
  Kolejka * p;
  unsigned c = 0; // zerujemy licznik
  p = start;
  while(p)   // dopóki lista nie jest pusta
  { 
    c++;     // zwiększamy licznik o 1
    p = p->next; // p staje się kolejnym z listy 
  }
  return c;
}

void FirstOut(){
  Kolejka *p;
  p = start;   
  if(p!= NULL) // jeśli lista nie jest pusta
  {
    start = p->next; // nowy początek
    free (p);    // usuń element z pamięci
  }
}

void Wypisz(){
    Kolejka * p;
    p =start;
    if(p!=NULL){
        printf("//");
          while(p)   // dopóki lista nie jest pusta
        { 
            printf("%d ",p->numer);
            p = p->next; // p staje się kolejnym z listy 
        }
        printf("\n");    
    }
    else printf("\n")
;}


void FirstIn(int v){
  Kolejka * p, * n;
  n = malloc(sizeof(*p));  // tworzymy nowy element
  n->next = NULL;   // będzie wyznaczał koniec listy
  n->numer = v;       //I przechowywał podaną wartość
  p = start;
  if(p!=NULL) //jeśli lista nie jest pusta
  {
     while(p->next) p = p->next; //szukamy końca listy
     p->next = n; //wstawiamy na końcowy zamiast NULL
  }
  else start = n; //lista była pusta
}

pthread_t myThread[N];
pthread_mutex_t mutex, mAC, mBC, most, mutkolejki;

int nn;
int ACount = 0, BCount = 0, MACount = 0, MBCount = 0;
int temp[N];
int opterr;
int debug = 0;
//Lista/kolejka samochodów.
//Mutex kolejki

void *Crossing(void *arg)
{
        while(1){
            
            if((int)arg==start->numer){
                pthread_mutex_lock(&mutex);
                if(temp[(long)arg] == 0){
                    ACount--;
                    printf("%d | %d [>> %d >>] %d | %d ",MACount, ACount, (long)arg , BCount, MBCount);
                    if(debug==1) Wypisz();
                    //BCount++;
                    temp[(long)arg]=1;
                }
                else{
                    BCount--;
                    printf("%d | %d [<< %d <<] %d | %d ",MACount, ACount, (long)arg , BCount, MBCount);
                    //ACount++; 
                    if(debug==1) Wypisz();
                    temp[(long)arg]=0;  
                }
                FirstOut();
                if(temp[(int)arg]==0)
                    MACount++;
                else
                    MBCount++;
                if(debug==0) printf("\n");
                pthread_mutex_unlock(&mutex);
                    int r = rand()%100000+1;
                    usleep(r);


                pthread_mutex_lock(&mutkolejki);
                FirstIn((int)arg);
                if(temp[(int)arg]==0){
                    ACount++;
                    MACount--;
                }
                else{
                    BCount++;
                    MBCount--;
                }
                pthread_mutex_unlock(&mutkolejki);

            }

        }

        pthread_exit((void*) 0);
}

int main (int argc, char *argv[])
{
        void *status;
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_init(&mutkolejki, NULL);
        srand(time(NULL)); 
        for(int i=0; i<N;i++){
            FirstIn(i);
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
        printf(" %d \n", argc);
        switch(argc){
            case 2: 
            nn = atoi(argv[1]); 
            break;
 
            case 3: 
            nn = atoi(argv[1]); 
            if(strcmp(argv[2],"-debug")==0)
                debug=1;;
            break;
            default:
                printf("Blad przy wpisywaniu argumentow\n;"); 
        }

        for(long i=0; i < N; i++)
                pthread_create(&myThread[i], NULL, Crossing, (void *)i);

        while(1);

        /* Results and cleanup */
        pthread_mutex_destroy(&mutkolejki);
        pthread_mutex_destroy(&mutex); 
        pthread_exit(NULL);
}