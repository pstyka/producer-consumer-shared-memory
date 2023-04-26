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
char znak;


void utworzPamiecDzielona();
void dolaczPamiecDzielona();
void odlaczPamiecDzielona();
static void semaforUpValue(int semaforNumber);
static void semaforLowValue(int semaforNumber);
static void resetValue(int semaforNumber);
static void utworzSemafor();
static void usunSemafor();


int main()
{	int i=0;
  char z;
	FILE * file;
  if (!(file = fopen("wy","w")))
  {
    fprintf(stderr, "Nie moge otworzyć pliku!!!!!!!!");
    exit(EXIT_FAILURE);
  }
  utworzSemafor();
  utworzPamiecDzielona();
  dolaczPamiecDzielona();


  while(1)
	{
    semaforLowValue(0);
    if(*adresPamieci!=EOF)
    {
      printf("Konsument skonsumowal: %c\n",*adresPamieci);
      fprintf(file,"%c",*adresPamieci);
      semaforUpValue(1);
    }
    else 
      break;
    
	}
  semaforUpValue(1);
  odlaczPamiecDzielona();
  printf("Koniec pobierania wartosci\n");
  exit(0);
}
//             tworzenie semafora
static void utworzSemafor()
{
  klucz = ftok(".", 'k');
	semafor = semget(997,2,IPC_CREAT|0606);                   //semget(klucz,liczba_semaforow,flaga i prawa dostepu); zwraca identyfikator semafora lub -1
	if(semafor == -1) 
  {
	  perror("semget error");
	  exit(EXIT_FAILURE);
	}
} // usun semafor
static void usunSemafor()
{
	int set;
  set=semctl(semafor,0,IPC_RMID);
	if (semafor == -1)
  {
		printf("semctl error\n");
	  exit(EXIT_FAILURE);
	}
  else 
  {
    printf("Pomyslnie usunieto semafor: %d\n",semafor);
  }
}

//            tworzenie dostepu do pamieci dzielonej
void utworzPamiecDzielona()
{
  pamiec=shmget(997,0, 0606 | IPC_CREAT);                     //shmget(klucz,rozmiar_pamieci,flaga); // zwraca identyfikator pamieci dzielonej lub -1
  if (pamiec==-1) 
    {
      printf("Problemy z utworzeniem pamieci dzielonej lub producent nie zostal wlaczony");
      exit(EXIT_FAILURE);
    }
}
//            dolaczenie (zyskanie dostepu) pamieci dzielonej
void dolaczPamiecDzielona()
{
    adresPamieci = shmat(pamiec, 0, 0);           // shmat(id_segmentu_pamieci)
    if (*adresPamieci == -1)
    {
      if(*adresPamieci == EOF)
      {
        printf("Plik jest pusty!!!!\n");
      }
      else
      {
        printf("shmat error\n");
        exit(EXIT_FAILURE);
      }
      
    }
}
void odlaczPamiecDzielona()
  {
    odlaczenie2=shmdt(adresPamieci);
    if (odlaczenie2==-1)
      {
        printf("Problemy z odlaczeniem pamieci dzielonej.\n");
        exit(EXIT_FAILURE);
      }
    else printf("Pamiec dzielona zostala odlaczona u konsumenta.\n");
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
