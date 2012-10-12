#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>

#define SIZE 16

int main (int argc, const char* argv[]) 
{ 
   int status; 
   long int i, loop, temp, *shmPtr; 
   int shmId, semId; 
   pid_t pid;
   struct sembuf wait1,signal;

   wait1.sem_num = 0;
   wait1.sem_op = -1;
   wait1.sem_flg = SEM_UNDO;

   signal.sem_num = 0;
   signal.sem_op = 1;
   signal.sem_flg = SEM_UNDO;

   loop = atoi(argv[1]);

   if ((shmId = shmget (IPC_PRIVATE, SIZE, IPC_CREAT|S_IRUSR|S_IWUSR)) < 0) {
      perror ("i can't get no..\n"); 
      exit (1); 
   } 
   if ((shmPtr = shmat (shmId, 0, 0)) == (void*) -1) { 
      perror ("can't attach\n"); 
      exit (1); 
   }
   shmPtr[0] = 0; 
   shmPtr[1] = 1;
   semId = semget(IPC_PRIVATE, 1, 00600);
   semctl (semId, 0, SETVAL, 1);
   if (!(pid = fork())) {
      semop(semId,&wait,1); 
      for (i=0; i<loop; i++) { 
               // swap the contents of shmPtr[0] and shmPtr[1]
		temp = shmPtr[0];
		shmPtr[0] = shmPtr[1];
		shmPtr[1] = temp; 
      } 
      if (shmdt (shmPtr) < 0) { 
         perror ("just can't let go\n"); 
         exit (1); 
      } 
      semctl (semId, 0, IPC_RMID);
      exit(0); 
   } else { 
      semop(semId,&wait,1);
      for (i=0; i<loop; i++) { 
               // swap the contents of shmPtr[1] and shmPtr[0] 
		temp = shmPtr[0];
                shmPtr[0] = shmPtr[1];
                shmPtr[1] = temp;
      }
      semctl (semId, 0, IPC_RMID);
   }

   wait (&status); 

   printf ("values: %li\t%li\n", shmPtr[0], shmPtr[1]);

   if (shmdt (shmPtr) < 0) { 
      perror ("just can't let go\n"); 
      exit (1); 
   } 
   if (shmctl (shmId, IPC_RMID, 0) < 0) { 
      perror ("can't deallocate\n"); 
      exit(1); 
   }

   return 1; 
}
