#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <sys/resource.h>

using namespace std;


sem_t* semaphore_I;
sem_t* semaphore_O;
void* boardered_memory;


struct Targets{
   bool flag;
   int number;
};

void *ThreadFunction1(void *value1){
    Targets *val1 = (Targets*) value1;
    cout<<"Thread 1 work was started\n";
    while (val1->flag == true) {
        int priority = getpriority(PRIO_PROCESS, 0);
        if (priority == -1) {
        perror("Failed to get thread priority");
        } else {
        cout << "Thread priority: " << priority << endl;
        }
        memcpy(boardered_memory, &priority, 1);
        sem_post(semaphore_I);
        sem_wait(semaphore_O);
        sleep(1);
    }
    cout<<"Thread 1 work was finished\n";
    pthread_exit((void*) 3);
}



int main() {
    pthread_t id1;
    Targets val1;
    int *exitcode1;
    val1.flag = true;
    val1.number = 1;


    int shm_memory = shm_open("/shared_memory", O_CREAT | O_RDWR, 0664);
    if (shm_memory == -1){
       perror("shm_open");
       return 1;
    }

    if(ftruncate(shm_memory, 1024) == -1){
       perror("ftruncate ");
       return 1;
    }

    boardered_memory = mmap(0, 1024, PROT_WRITE, MAP_SHARED, shm_memory, 0);
    if (boardered_memory == MAP_FAILED){
       perror("mmap ");
       return 2;
    }

    semaphore_I = sem_open("/semaphore_I", O_CREAT, 0664, 1);
    if (semaphore_I == SEM_FAILED){
       perror("sem_open");
       return 3;
    }

    semaphore_O = sem_open("/semaphore_O", O_CREAT, 0664, 1);
    if (semaphore_I == SEM_FAILED){
       perror("sem_open");
       return 4;
    }

    //thread creation/illumination

    if (pthread_create(&id1, NULL, ThreadFunction1, &val1)){
        perror("pthread_create ");
        return 1;
    }
    getchar();
    val1.flag = false;

    if (pthread_join(id1, (void**)&exitcode1)){
        perror("pthread_join ");
    }
    else{
    cout<<"Exitcode 1: "<< exitcode1 <<endl;
    }

    // illumination

    if (sem_close(semaphore_I) == -1){
       perror("sem_close ");
       return 3;
    }
    if (sem_unlink("/semaphore_I") == -1){
       perror("sem_unlink ");
       return 4;
    }
    if (sem_close(semaphore_O) == -1){
       perror("sem_close ");
       return 5;
    }
    if (sem_unlink("/semaphore_O") == -1){
       perror("sem_ unlink ");
       return 6;
    }

    if (munmap(boardered_memory, 1024) == -1){
       perror("munmap ");
       return 7;
    }
    if (shm_unlink("/shared_memory") == -1){
       perror("shm_unlink ");
       return 8;
    }
    return 0;
}

