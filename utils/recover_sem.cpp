#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>

#define MANGO_SEMAPHORE       "mango_sem"

int main(int argc, char const *argv[])
{
    sem_t * sem_id = sem_open(MANGO_SEMAPHORE, O_CREAT, 0666, 1);

    if (sem_id == SEM_FAILED) {
        printf("Sem opening failed!\n");
        return -1;
    }

    sem_post(sem_id);

    sem_close(sem_id);

    return 0;
}
