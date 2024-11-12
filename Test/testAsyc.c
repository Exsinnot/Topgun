#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
long long timestamp;
int delay(int number_of_seconds){
    int milli_seconds =  number_of_seconds*1000000;

    clock_t start_time = clock();

    while (clock() < start_time + milli_seconds);
}

long long current_timestamp_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    long long ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

    return ms;
}

void *function1(void *arg) {
    for (int i = 0; i < 5; i++) {

        printf("Function 1: %d\n", i);
        printf("Function 1: %ld\n", (long)timestamp);
        delay(2);
    }
    return NULL;
}

void *function2(void *arg) {
    for(;;){
        timestamp = current_timestamp_ms();
        delay(1);
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    if (pthread_create(&thread1, NULL, function1, NULL) != 0) {
        fprintf(stderr, "Error creating thread1\n");
        return 1;
    }

    // สร้าง thread ที่สอง
    if (pthread_create(&thread2, NULL, function2, NULL) != 0) {
        fprintf(stderr, "Error creating thread2\n");
        return 1;
    }

    // รอให้ thread ทั้งสองทำงานเสร็จ
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Both threads completed.\n");
    return 0;
}
