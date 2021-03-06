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
#include <semaphore.h>
#include <sched.h>

volatile sig_atomic_t exitflag = 0;
void sig_handler(int signo)
{
  if (signo == SIGINT)
    exitflag=1;
}


pthread_t *myThread;
pthread_mutex_t mutex;
sem_t smost,skolejkiIn;      //Deklaracja oraz inicjacja zmiennej warunkowej
int nn; //Licznik wątków / samochodów
int ACount = 0, BCount = 0, MACount = 0, MBCount = 0;   //Liczniki kolejek i miast
int *temp;  //Tymczasowa tablica przechowujaca polozenie samochodu.
int opterr = 0; //Kontrola błędów w argumentach
int debug = 0;  //Flaga argumentu -debug.

void *Crossing(void *arg)
{
        while(exitflag==0){//Zakonz watki sygnalem SIGINT
                //Zablokowanie mostu by bezpiecznie przejechac.
                sem_wait(&smost);
                if(temp[(long)arg] == 0){
                    ACount--;
                    printf("A - %d | %d [>> %d >>] %d | %d - B",MACount, ACount, (long)arg , BCount, MBCount);
                    temp[(long)arg]=1;  
                }
                else{
                    BCount--;
                    printf("A - %d | %d [<< %d <<] %d | %d - B",MACount, ACount, (long)arg , BCount, MBCount);
                    temp[(long)arg]=0;  
                }
                if(temp[(int)arg]==0)
                    MACount++;
                else
                    MBCount++;

                if(debug==0) printf("\n");
                sem_post(&smost);//Zwolnienie mostu.

                //Okres gdy samochod jest w miescie.
                //Samochod wyjezdza z miasta i staje w kolejce.
                sem_wait(&skolejkiIn);
                if(temp[(int)arg]==0){
                    ACount++;
                    MACount--;
                }
                else{
                    BCount++;
                    MBCount--;
                }
                sem_post(&skolejkiIn);
                //Zwolnienie semafora kolejki aby inne samochody mogly zmieniac liczniki
        }
        pthread_exit((void*) 0);
}

int main (int argc, char *argv[])
{
        signal(SIGINT, sig_handler); 
        void* status;
        char *eptr;
        int reterror;

        reterror = sem_init(&smost,0,1);
        reterror = sem_init(&skolejkiIn,0,1);   //Inicjalizacja semaforow

        if (reterror != 0){
            printf("Blad przy inicjacji mutexow/semaforow");
            return 1;
        }
        srand(time(NULL)); 

        //Testowanie poprawności wprowadzonych argumentów
        switch(argc){
            case 2: 
                nn = strtol(argv[1], &eptr, 10); 
                if (nn==0)
                opterr = 1;
            break;
 
            default:
            opterr =1;
                printf("Blad przy wpisywaniu argumentow\n;"); 
                
            break;
        }
        //Sprawdzenie czy poprawnie wprowadzono argumenty.
        if(opterr==0){
            //Lokowanie tymczasowej tablicy
            temp = (int*) malloc(nn*sizeof(int));
            //Tworzenie listy watkow.
            myThread = (pthread_t*) malloc(nn*sizeof(pthread_t));
            //Losowanie kierunku przejazdu dla samochodu w kolejce.
            for(int i=0; i<nn;i++){
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
        for(long i=0; i < nn; i++){
           reterror = pthread_create(&myThread[i], NULL, Crossing, (void *)i);
                if(reterror!=0)
                        printf("Nie mozna bylo utworzyc watku numer %d ", i);
        }
        //Oczekiwanie na zakończenie wszystkich wątków.
        for(long i = 0; i<nn; i++)
                pthread_join(myThread[i], &status);

        //Kasowanie List.
        free(myThread);
        free(temp);
        }
        else printf("Blad przy wpisywaniu liczby\n");
        //Czyszczenie semaforow.
        sem_destroy(&smost);
        sem_destroy(&skolejkiIn);
        pthread_exit(NULL);
}