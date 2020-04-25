#include "mybarrier.hpp"
#include <unistd.h>
#ifdef WITH_CDS
#include <threads.h>
#else
#include <pthread.h>
#endif

constexpr int THREAD_NUM = 3;
constexpr int RADIX = 2;
int array[THREAD_NUM]{0};

struct RunParams {
    int thread_id;
    int target;
    StaticTreeBarrier* barrier;
};

#ifdef WITH_CDS
void runnable(void* args) {
    RunParams* param = (RunParams*)args;
    array[param->thread_id] = 1;
    param->barrier->await(param->thread_id);
}
#else
void* runnable(void* args) {
    RunParams* param = (RunParams*)args;
    param->barrier->await(param->thread_id);
    if(array[param->thread_id] != param->target)
        printf("barrier failure detected in runnable\n");
}
#endif
#ifdef WITH_CDS
int user_main(int argc, char **argv) {
    StaticTreeBarrier *barrier = new StaticTreeBarrier(THREAD_NUM, RADIX);
    RunParams params[THREAD_NUM];

    thrd_t threads[THREAD_NUM];
    for (int i = 1; i < THREAD_NUM; ++i) {
        params[i].thread_id = i;
        params[i].barrier = barrier;
        thrd_create(&threads[i], runnable, (void *) &params[i]);
    }
    barrier->await(0);
    int sum = 0;
    for(int i = 0; i < THREAD_NUM; ++ i)
        sum += array[i];
    MODEL_ASSERT(sum == THREAD_NUM-1);
    return 0;
}
#else
int main(int argc, char **argv) {
    StaticTreeBarrier* barrier = new StaticTreeBarrier(THREAD_NUM, 2);
    RunParams params[THREAD_NUM];
    pthread_t threads[THREAD_NUM];
    for(int j = 0; j < 10000; ++ j) {
        int target = 1;
        for (int i = 1; i < THREAD_NUM; ++i) {
            params[i].thread_id = i;
            params[i].barrier = barrier;
            params[i].target = target;
            pthread_create(&threads[i], nullptr, runnable, (void *) &params[i]);
            array[i] = target;
        }
        barrier->await(0);
        target = 1-target;
    }
    pthread_exit(0);
}
#endif
