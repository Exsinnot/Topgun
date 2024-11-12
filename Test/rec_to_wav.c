#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <unistd.h>
#include <curl/curl.h>
#define PCM_DEVICE "default"
#define SAMPLE_RATE 44100
#define CHANNELS 1
#define BITS_PER_SAMPLE 16
#define BUFFER_SIZE 128
#define RECORD_SECONDS 4
#define BUFFER_SECONDS 1

int total_buffers;
int save_frames;
int total_frames;
pthread_mutex_t buffer_mutex;
short (*Buffer_Data)[BUFFER_SIZE];
int indexBuffer = 0;

typedef struct WAVHeader {
    char riff[4];
    unsigned int chunkSize;
    char wave[4];
    char fmt[4];
    unsigned int subchunk1Size;
    unsigned short audioFormat;
    unsigned short numChannels;
    unsigned int sampleRate;
    unsigned int byteRate;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
    char data[4];
    unsigned int dataSize;
} WAVHeader;

void write_wav_header(FILE *file, int sampleRate, int channels, int bitsPerSample, int dataSize) {
    WAVHeader header;
    strncpy(header.riff, "RIFF", 4);
    header.chunkSize = 36 + dataSize;
    strncpy(header.wave, "WAVE", 4);
    strncpy(header.fmt, "fmt ", 4);
    header.subchunk1Size = 16;
    header.audioFormat = 1;
    header.numChannels = channels;
    header.sampleRate = sampleRate;
    header.byteRate = sampleRate * channels * bitsPerSample / 8;
    header.blockAlign = channels * bitsPerSample / 8;
    header.bitsPerSample = bitsPerSample;
    strncpy(header.data, "data", 4);
    header.dataSize = dataSize;

    fwrite(&header, sizeof(WAVHeader), 1, file);
}

static size_t write_callback(void *ptr, size_t size, size_t nmemb, char *data) {
    size_t total_size = size * nmemb;
    strcat(data, ptr);
    return total_size;
}

int Send_API(){
    CURL *curl;
    CURLcode res;

    const char *file1_path = "output.wav";
    const char *file2_path = "output.wav";
    
    time_t timestamp = time(NULL);
    printf("Unix timestamp: %ld\n", timestamp);
    char timestamp_str[20];
    sprintf(timestamp_str, "%ld", timestamp);

    FILE *file1 = fopen(file1_path, "rb");
    FILE *file2 = fopen(file2_path, "rb");
    if (!file1 || !file2) {
        fprintf(stderr, "Could not open files\n");
        return 1;
    }

    curl_mime *mime;
    curl_mimepart *part;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        mime = curl_mime_init(curl);

        part = curl_mime_addpart(mime);
        curl_mime_filedata(part, file1_path);
        curl_mime_type(part, "audio/wav");
        curl_mime_name(part, "file1");

        part = curl_mime_addpart(mime);
        curl_mime_filedata(part, file2_path);
        curl_mime_type(part, "audio/wav");
        curl_mime_name(part, "file2");

        part = curl_mime_addpart(mime);
        curl_mime_data(part, timestamp_str, strlen(timestamp_str)); 
        curl_mime_name(part, "timestamp");

        curl_easy_setopt(curl, CURLOPT_URL, "http://210.246.215.31:1000/api/v1/upload");
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        char response_data[1000] = {0};
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_data);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("Response: %s\n", response_data);
        }
        curl_mime_free(mime);
        curl_easy_cleanup(curl);
    }

    fclose(file1);
    fclose(file2);
    curl_global_cleanup();

}

void *GetData_AND_Buffer(void *arg) {
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    unsigned int sample_rate = SAMPLE_RATE;
    int pcm, dir;
    short buf[BUFFER_SIZE];

    pthread_mutex_lock(&buffer_mutex);

    pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (pcm < 0) {
        fprintf(stderr, "ERROR: Cannot open PCM device %s: %s\n", PCM_DEVICE, snd_strerror(pcm));
        pthread_mutex_unlock(&buffer_mutex);
        return NULL;
    }

    snd_pcm_hw_params_malloc(&params);
    snd_pcm_hw_params_any(pcm_handle, params);
    snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(pcm_handle, params, CHANNELS);
    snd_pcm_hw_params_set_rate_near(pcm_handle, params, &sample_rate, &dir);

    pcm = snd_pcm_hw_params(pcm_handle, params);
    if (pcm < 0) {
        fprintf(stderr, "ERROR: Cannot set hardware parameters: %s\n", snd_strerror(pcm));
        snd_pcm_hw_params_free(params);
        snd_pcm_close(pcm_handle);
        pthread_mutex_unlock(&buffer_mutex);
        return NULL;
    }

    total_frames = SAMPLE_RATE * (RECORD_SECONDS + BUFFER_SECONDS);
    save_frames = SAMPLE_RATE * RECORD_SECONDS / BUFFER_SIZE;
    total_buffers = total_frames / BUFFER_SIZE;

    Buffer_Data = malloc(total_buffers * sizeof(short[BUFFER_SIZE]));
    if (!Buffer_Data) {
        fprintf(stderr, "ERROR: Cannot allocate memory for buffer data\n");
        snd_pcm_hw_params_free(params);
        snd_pcm_close(pcm_handle);
        pthread_mutex_unlock(&buffer_mutex);
        return NULL;
    }

    pthread_mutex_unlock(&buffer_mutex);

    while(1) {
        pcm = snd_pcm_readi(pcm_handle, buf, BUFFER_SIZE);
        if (pcm == -EPIPE) {
            fprintf(stderr, "XRUN.\n");
            snd_pcm_prepare(pcm_handle);
        } else if (pcm < 0) {
            fprintf(stderr, "ERROR: Can't read from PCM device. %s\n", snd_strerror(pcm));
        } else if (pcm != BUFFER_SIZE) {
            fprintf(stderr, "Short read, read %d frames\n", pcm);
        }

        pthread_mutex_lock(&buffer_mutex);
        memcpy(Buffer_Data[indexBuffer], buf, BUFFER_SIZE * sizeof(short));
        indexBuffer = (indexBuffer + 1) % total_buffers;
        pthread_mutex_unlock(&buffer_mutex);
    }

    snd_pcm_hw_params_free(params);
    snd_pcm_close(pcm_handle);
    return NULL;
}

void *Send_Data(void *arg) {
    usleep(5000000);

    while (1) {
        FILE *file = fopen("output.wav", "wb"); 
        if (!file) {
            fprintf(stderr, "ERROR: Cannot open output.wav for writing\n");
            return NULL;
        }

        int data_size = save_frames * BUFFER_SIZE * sizeof(short);
        write_wav_header(file, SAMPLE_RATE, CHANNELS, BITS_PER_SAMPLE, data_size);
        usleep(100000);

        pthread_mutex_lock(&buffer_mutex);
        for (int i = indexBuffer, c = 0; c < save_frames; i++, c++) {
            if (i >= total_buffers) {
                i = 0;
            }
            fwrite(Buffer_Data[i], sizeof(short), BUFFER_SIZE, file);
        }
        pthread_mutex_unlock(&buffer_mutex);

        fseek(file, 4, SEEK_SET);
        unsigned int file_size = 36 + data_size;
        fwrite(&file_size, sizeof(unsigned int), 1, file);
        fseek(file, 40, SEEK_SET);
        fwrite(&data_size, sizeof(unsigned int), 1, file);
        fclose(file);
        printf("save\n");
        Send_API();
    }
    

    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    pthread_mutex_init(&buffer_mutex, NULL);

    if (pthread_create(&thread1, NULL, GetData_AND_Buffer, NULL) != 0) {
        fprintf(stderr, "Error creating thread1\n");
        return 1;
    }

    if (pthread_create(&thread2, NULL, Send_Data, NULL) != 0) {
        fprintf(stderr, "Error creating thread2\n");
        pthread_join(thread1, NULL);
        return 1;
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&buffer_mutex);
    free(Buffer_Data);


    return 0;
}
