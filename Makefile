all: zmienne mutex

zmienne:
	gcc -pthread -Wall -o zmienne zmienne.c

mutex:
	gcc -pthread -Wall -o mutex mutex.c
