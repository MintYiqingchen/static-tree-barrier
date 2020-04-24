#include "mybarrier.hpp"
#include <pthread.h>
#include <unistd.h>

constexpr int THREAD_NUM = 5;
int array[THREAD_NUM];

struct RunParams {
    int thread_id;
    StaticTreeBarrier* barrier;
};

void* runnable(void* args) {
    RunParams* param = (RunParams*)args;
    param->barrier->await(param->thread_id);
    printf("Hello, I'm thread %d\n", param->thread_id);
}

int main(int argc, char **argv) {
    StaticTreeBarrier* barrier = new StaticTreeBarrier(THREAD_NUM, 2);
    RunParams params[THREAD_NUM];

//    thrd_t threads[THREAD_NUM];
    pthread_t threads[THREAD_NUM];
    for(int i = 1; i < THREAD_NUM; ++ i){
        params[i].thread_id = i;
        params[i].barrier = barrier;
        pthread_create(&threads[i], nullptr, runnable, (void*)&params[i]);
//        thrd_create(&threads[i], runnable, (void*)&params[i]);
    }
    sleep(10);
    barrier->await(0);
    pthread_exit(0);
    return 0;
}
