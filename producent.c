#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

int pamiec;
int semafor;
int klucz;
int odlaczenie1, odlaczenie2;
char *adresPamieci;
char znak, zn;


void utworzPamiecDzielona();
void dolaczPamiecDzielona();
void odlaczPamiecDzielona();
static void semaforUpValue(int semaforNumber);
static void semaforLowValue(int semaforNumber);
static void resetValue(int semaforNumber);
static void utworzSemafor();
static void utworzSemafor();
static void ustawSemafor();
static void usunSemafor();



int main()
{	int i=0;
  char z;
	FILE * file;
  if (!(file = fopen("we","r")))
  {
    fprintf(stderr, "Nie moge otworzyć pliku!!!!!!!!");
    exit(EXIT_FAILURE);
  }
  utworzSemafor();
  ustawSemafor();
  utworzPamiecDzielona();
  dolaczPamiecDzielona();

  semaforUpValue(1);
 
  do
	{  
    semaforLowValue(1);
    zn=fgetc(file); // producent wylacz p
    *adresPamieci=zn;
    printf("Producent sprzedał: %c\n",zn);
    semaforUpValue(0); // KONSUMENT 0
	}
  while(zn != EOF);
  semaforLowValue(1);

  odlaczPamiecDzielona();
  usunSemafor();
  fclose(file);
  exit(0);
}

//            tworzenie dostepu do pamieci dzielonej
void utworzPamiecDzielona()
{
  pamiec=shmget(997,1, 0606 | IPC_CREAT);
  if (pamiec==-1) 
    {
      printf("Problemy z utworzeniem pamieci dzielonej");
      exit(EXIT_FAILURE);
    }
}
//            dolaczenie (zyskanie dostepu) pamieci dzielonej
void dolaczPamiecDzielona()
{
    adresPamieci = shmat(pamiec, 0, 0);
    if (*adresPamieci == -1)
    {
      printf("shmat error\n");
      exit(EXIT_FAILURE);
    }
}
void odlaczPamiecDzielona()
  {
    odlaczenie1=shmctl(pamiec,IPC_RMID,0); //usuniecie segmentu pamieci
    odlaczenie2=shmdt(adresPamieci);// odlaczenie 
    if (odlaczenie1==-1 || odlaczenie2==-1)
    {
      printf("Problemy z odlaczeniem pamieci dzielonej.\n");
      exit(EXIT_FAILURE);
    }
    else printf("Pamiec dzielona zostala odlaczona u producenta.\n");
  }
//                  tworzenie semafora
static void utworzSemafor()
{
  klucz = ftok(".", 'k');
	semafor = semget(997,2,IPC_CREAT|0606);
	if(semafor == -1) 
  {
	  perror("semget error");
	  exit(EXIT_FAILURE);
	}
}
static void usunSemafor()
{
	int set;
  set=semctl(semafor,0,IPC_RMID);            //semctl(id_semafora, numer semafora na zbiorze, kod_polecenia, parametry polecenia); zwraca 0 lub wartosc zadana przez kod_polecenia
	if (semafor == -1)
  {
		printf("semctl error\n");
	  exit(EXIT_FAILURE);
	}
  else 
  {
    printf("Pomyslnie usunieto semafory: %d\n",semafor);
  }
}
// ustaw wartosc na semaforze
static void ustawSemafor(void)
  {
    int ustaw_sem;
    int i;
    for(i=0;i<2;i++)
    {
        ustaw_sem=semctl(semafor,i,SETVAL,0);
      if (ustaw_sem==-1)
        {
          printf("Nie mozna ustawic semafora.\n");
          exit(EXIT_FAILURE);
        }
    }
    
  }

//     podniesienie wartosci semafora
static void semaforUpValue(int semaforNumber)
{
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num=semaforNumber;
    bufor_sem.sem_op=1;
    bufor_sem.sem_flg=0;
    zmien_sem=semop(semafor,&bufor_sem,1);
    if (zmien_sem==-1) 
    {
      if(errno == ERANGE) 
        {
          resetValue(semaforNumber);
        }
      else
      {
        printf("Nie moglem otworzyc semafora.\n");
        exit(EXIT_FAILURE);
      }
    }
}
static void resetValue(int semaforNumber)
{
  int value;
  value = semctl(semafor,semaforNumber,SETVAL,1);
  if(value == -1)
  {
    printf("reset error\n");
	  exit(EXIT_FAILURE);
  }
}
//        obnizenie wartosci semafora
static void semaforLowValue(int semaforNumber)
{
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num=semaforNumber;
    bufor_sem.sem_op=-1;
    bufor_sem.sem_flg=0;
    zmien_sem=semop(semafor,&bufor_sem,1);
    if (zmien_sem==-1) 
    {
      if(errno == ERANGE)
      {
        resetValue(semaforNumber);
      }
      else if(errno == EINTR)
      {
        semaforLowValue(semaforNumber);
      }
  }
}
