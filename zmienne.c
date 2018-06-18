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
#include <string.h>
#include <signal.h>
#include <errno.h>

volatile sig_atomic_t exitflag = 0;
void sig_handler(int signo)
{
  if (signo == SIGINT)
    exitflag=1;
}

typedef struct KOLEJKA Kolejka;
struct KOLEJKA{
	int* numer;     //Numer naszego samochodu w kolejce
	Kolejka* next;
        int* stan;
};     

//Utworzenie kolejki
Kolejka * start = NULL;

//Zliczenie zawartości listy
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

//usunięcie pierwszego elementu z listy
void FirstOut(){
  Kolejka *p;
  p = start;   
  if(p!= NULL) // jeśli lista nie jest pusta
  {
    start = p->next; // nowy początek
    free (p);    // usuń element z pamięci
  }
}

//Wypisanie zawartości kolejki
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

//Wstawianie nowego elementu na koniec listy
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

//kasowanie listy
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
pthread_mutex_t most, mutexkolejki,mutexkolejkiOut;     //Deklaracja mutexów
pthread_cond_t zmienna = PTHREAD_COND_INITIALIZER;      //Deklaracja oraz inicjacja zmiennej warunkowej
int nn; //Licznik wątków / samochodów
int ACount = 0, BCount = 0, MACount = 0, MBCount = 0;   //Liczniki kolejek i miast
int *temp;
int opterr = 0; //Kontrola błędów w argumentach
int debug = 0;  //Flaga argumentu -debug.

void *Crossing(void *arg)
{
        while(exitflag==0){
                //Zablokowanie mostu by bezpiecznie sprawdzić kolejke i ewentualny przejazd.
                pthread_mutex_lock(&most);
                while((int)arg!=start->numer)                   //Gdy Numer samochodu nie jest pierwszy w kolejce.
                        pthread_cond_wait(&zmienna,&most);      //Uśpienie i odblokowanie mostu by inne samochody mogły z niego skorzystać.
                
                if(temp[(long)arg] == 0){
                    ACount--;
                    printf("A - %d | %d [>> %d >>] %d | %d - B",MACount, ACount, (long)arg , BCount, MBCount);
                    if(debug==1) Wypisz();
                    temp[(long)arg]=1;
                }
                else{
                    BCount--;
                    printf("A - %d | %d [<< %d <<] %d | %d - B",MACount, ACount, (long)arg , BCount, MBCount);
                    if(debug==1) Wypisz();
                    temp[(long)arg]=0;  
                }
                if(temp[(int)arg]==0)
                    MACount++;
                else
                    MBCount++;

                if(debug==0) printf("\n");
                
                pthread_mutex_unlock(&most);
                
                //Zablokowanie kolejki aby można było bezpiecznie usunąć samochód.
                pthread_mutex_lock(&mutexkolejkiOut); 
                FirstOut();     //Usuniecie pierwszego samochodu z kolejki
                pthread_cond_broadcast(&zmienna);       //Powiadomienie wątków że stan kolejki się zmienił.
                int r = rand()%10000+1;
                pthread_mutex_unlock(&mutexkolejkiOut); //Odblokowanie kolejki
                
                usleep(r);      //Symulowanie jazdy samochodu po mieście.


                //Zablokowanie kolejki aby można było bezpiecznie dodać samochód.
                pthread_mutex_lock(&mutexkolejki);
                FirstIn((int)arg);      //Dodanie do kolejki ID obecnego wątku.
                if(temp[(int)arg]==0){
                    ACount++;
                    MACount--;
                }
                else{
                    BCount++;
                    MBCount--;
                }
                pthread_cond_broadcast(&zmienna);       //Wysłanie sygnału wątkom.
                pthread_mutex_unlock(&mutexkolejki);    //Odblokowanie  kolejki.
            

        }
        pthread_exit((void*) 0);
}

int main (int argc, char *argv[])
{
        signal(SIGINT, sig_handler); 
        void* status;
        char *eptr;
        //Inicializacje potrzebnych mutexów.
        pthread_mutex_init(&most, NULL);
        pthread_mutex_init(&mutexkolejki, NULL);
        pthread_mutex_init(&mutexkolejkiOut, NULL);
        srand(time(NULL)); 

        //Testowanie poprawności wprowadzonych argumentów
        switch(argc){
            case 2: 
                nn = strtol(argv[1], &eptr, 10); 
                if (nn==0)
                opterr = 1;
            break;
 
            case 3: 
                nn = strtol(argv[1], &eptr, 10); 
                if (nn==0)
                opterr = 1; 
                if(strcmp(argv[2],"-debug")==0)
                        debug=1;;
            break;
            default:
            opterr =1;
                printf("Blad przy wpisywaniu argumentow\n;"); 
                
            break;
        }
        //Sprawdzenie czy poprawnie wprowadzono argumenty.
        if(opterr==0){
            temp = (int*) malloc(nn*sizeof(int));
            myThread = (pthread_t*) malloc(nn*sizeof(pthread_t));
            //Losowanie kierunku przejazdu dla samochodu w kolejce.
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
            //Tworzenie wątków które bedą przekraczać most.
        for(long i=0; i < nn; i++)
            pthread_create(&myThread[i], NULL, Crossing, (void *)i);

        //Oczekiwanie na zakończenie wszystkich wątków.
        for(long i = 0; i<nn; i++)
                pthread_join(myThread[i], &status);

        //Kasowanie List.
        freeList(&start);
        free(myThread);
        free(temp);
        }
        else printf("Blad przy wpisywaniu liczby\n");
        pthread_mutex_destroy(&mutexkolejkiOut);
        pthread_mutex_destroy(&mutexkolejki);
        pthread_mutex_destroy(&most); 
        pthread_exit(NULL);
}