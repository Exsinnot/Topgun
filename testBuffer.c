#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>  

// ตัวแปรที่ใช้ในการเก็บข้อมูลและการควบคุม
int OverBuffer_seconds = 1;
int duration_seconds = 5;
int sample_rate = 44100;
int *Buffer_Data;
int indexBuffer = 0;
int maxBuffer;
int maxOverBuffer;

pthread_mutex_t buffer_mutex;

void *GetData_AND_Buffer(void *arg) {
    while (1) {        
        pthread_mutex_lock(&buffer_mutex);
        Buffer_Data[indexBuffer] = rand();  // เติมข้อมูลสุ่มลงใน buffer
        // printf("index %d = %d \n",indexBuffer,Buffer_Data[indexBuffer]);
        indexBuffer++;
        if (indexBuffer >= maxOverBuffer) {
            indexBuffer = 0;
        }
        printf("Test\n %d",indexBuffer);
        pthread_mutex_unlock(&buffer_mutex);  
        usleep(100);  // ลดการใช้งาน CPU โดยการหน่วงเวลาเล็กน้อย
    }
}

// ฟังก์ชันส่งข้อมูลจาก buffer
void *Send_Data(void *arg) {
    while (1) {
        pthread_mutex_lock(&buffer_mutex);  // ป้องกันการเข้าถึงที่ขัดแย้งกัน
        printf("Buffer[0]: %d\n", Buffer_Data[0]);  // พิมพ์ข้อมูลจากตำแหน่งแรกของ buffer
        usleep(1000000);  

        pthread_mutex_unlock(&buffer_mutex);  
        usleep(1000000);  
    }
}

int main() {
    maxOverBuffer = sample_rate * (OverBuffer_seconds + duration_seconds);
    maxBuffer = sample_rate * duration_seconds;

    // จัดสรรหน่วยความจำสำหรับ Buffer_Data
    Buffer_Data = (int *)malloc(maxOverBuffer * sizeof(int)); 
    if (Buffer_Data == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;  // ตรวจสอบว่า malloc สำเร็จหรือไม่
    }
    
    printf("Memory allocated for Buffer_Data\n");

    pthread_t thread1, thread2;

    // สร้าง mutex
    pthread_mutex_init(&buffer_mutex, NULL);

    // สร้างเธรดสำหรับการบันทึกข้อมูล
    if (pthread_create(&thread1, NULL, GetData_AND_Buffer, NULL) != 0) {
        fprintf(stderr, "Error creating thread1\n");
        return 1;
    }

    // สร้างเธรดสำหรับการส่งข้อมูล
    if (pthread_create(&thread2, NULL, Send_Data, NULL) != 0) {
        fprintf(stderr, "Error creating thread2\n");
        return 1;
    }

    printf("Threads created successfully.\n");

    // รอให้เธรดทั้งสองทำงานเสร็จ
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Both threads completed.\n");

    // ทำลาย mutex และคืนหน่วยความจำ
    pthread_mutex_destroy(&buffer_mutex);
    free(Buffer_Data);

    return 0;
}
