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

void freeList()
{
Kolejka *tmp, *node;
node = start;
   while (node != NULL)
    {
       tmp = node;
       node = node->next;
       free(tmp);
    }
    start = NULL;

}

pthread_t *myThread;
pthread_mutex_t mutex, mAC, mBC, most, mutkolejki;
pthread_cond_t zmienna = PTHREAD_COND_INITIALIZER;
pthread_cond_t zmiennakolejki = PTHREAD_COND_INITIALIZER;
int nn;
int ACount = 0, BCount = 0, MACount = 0, MBCount = 0;
int *temp;
int opterr = 0;
int debug = 0;
int zajeta = 0;

void *Crossing(void *arg)
{
        while(1){
            while((int)arg!=start->numer)
               pthread_cond_wait(&zmienna,&most);
                
                if(temp[(long)arg] == 0){
                    ACount--;
                    printf("%d | %d [>> %d >>] %d | %d ",MACount, ACount, (long)arg , BCount, MBCount);
                    if(debug==1) Wypisz();
                    
                    temp[(long)arg]=1;
                }
                else{
                    BCount--;
                    printf("%d | %d [<< %d <<] %d | %d ",MACount, ACount, (long)arg , BCount, MBCount);

                    if(debug==1) Wypisz();
                    temp[(long)arg]=0;  
                }
                FirstOut();

                if(temp[(int)arg]==0)
                    MACount++;
                else
                    MBCount++;

                if(debug==0) printf("\n");
                
                int r = rand()%100000+1;
                pthread_cond_signal(&zmienna);
                usleep(r);


                while(zajeta!=0)
                        pthread_cond_wait(&zmiennakolejki,&mutex);
                zajeta=1;
                FirstIn((int)arg);
                if(temp[(int)arg]==0){
                    ACount++;
                    MACount--;
                }
                else{
                    BCount++;
                    MBCount--;
                }
                zajeta=0;
                pthread_cond_signal(&zmiennakolejki);
                
            

        }

        pthread_exit((void*) 0);
}

int main (int argc, char *argv[])
{
        void *status;
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_init(&mutkolejki, NULL);
        srand(time(NULL)); 

        switch(argc){
            case 2: 
            if(atoi(argv[1])==0)
            nn = atoi(argv[1]); 
            break;
 
            case 3: 
            nn = atoi(argv[1]); 
            if(strcmp(argv[2],"-debug")==0)
                debug=1;;
            break;
            default:
            opterr =1;
                printf("Blad przy wpisywaniu argumentow\n;"); 
                
            break;
        }
        if(opterr==0){
            temp = (int*) malloc(nn*sizeof(int));
            myThread = (pthread_t*) malloc(nn*sizeof(pthread_t));
            for(int i=0; i<nn;i++){
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
            
            for(long i=0; i < nn; i++)
            pthread_create(&myThread[i], NULL, Crossing, (void *)i);

        while(1);

        freeList(&start);
        }
        pthread_mutex_destroy(&mutkolejki);
        pthread_mutex_destroy(&mutex); 
        pthread_exit(NULL);
}