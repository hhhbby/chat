#include <semaphore.h>

class Sem{
public:
    Sem(unsigned int value)
    {
        sem_init(&sem, 0, value);
    }
    ~Sem()
    {
        sem_destroy(&sem);
    }
    int wait()
    {
        return sem_wait(&sem);
    }
    void post()
    {
        sem_post(&sem);
    }
private:
    sem_t sem;
};