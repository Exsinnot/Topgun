#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

// WAV header structure
#pragma pack(push, 1)
typedef struct {
    char RIFF[4];               // "RIFF" in ASCII
    uint32_t ChunkSize;         // Size of the rest of the chunk (after RIFF)
    char WAVE[4];               // "WAVE" in ASCII
    char fmt[4];                // "fmt " in ASCII
    uint32_t Subchunk1Size;     // Size of the format chunk (16 for PCM)
    uint16_t AudioFormat;       // Audio format (1 = PCM)
    uint16_t NumChannels;       // Number of channels (1 = mono, 2 = stereo)
    uint32_t SampleRate;        // Sample rate (e.g., 44100)
    uint32_t ByteRate;          // SampleRate * NumChannels * BitsPerSample / 8
    uint16_t BlockAlign;        // NumChannels * BitsPerSample / 8
    uint16_t BitsPerSample;     // Bits per sample (e.g., 16 for 16-bit)
    char data[4];               // "data" in ASCII
    uint32_t DataSize;          // Size of the audio data
} WAVHeader;
#pragma pack(pop)

// Function to write a WAV file with random audio data
void generate_random_noise_wav(const char *filename, int duration_seconds, int sample_rate) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Failed to open file for writing.\n");
        return;
    }

    // Set up the WAV header
    WAVHeader header = {0};
    header.RIFF[0] = 'R';
    header.RIFF[1] = 'I';
    header.RIFF[2] = 'F';
    header.RIFF[3] = 'F';
    header.WAVE[0] = 'W';
    header.WAVE[1] = 'A';
    header.WAVE[2] = 'V';
    header.WAVE[3] = 'E';
    header.fmt[0] = 'f';
    header.fmt[1] = 'm';
    header.fmt[2] = 't';
    header.fmt[3] = ' ';
    header.Subchunk1Size = 16; // PCM format
    header.AudioFormat = 1;    // PCM format
    header.NumChannels = 1;    // Mono audio
    header.SampleRate = sample_rate;
    header.ByteRate = sample_rate * header.NumChannels * 2; // 16-bit samples
    header.BlockAlign = header.NumChannels * 2;  // 16-bit samples
    header.BitsPerSample = 16;  // 16-bit samples
    header.data[0] = 'd';
    header.data[1] = 'a';
    header.data[2] = 't';
    header.data[3] = 'a';

    // Calculate the size of the audio data (duration * sample_rate * num_channels * sample_size)
    int num_samples = duration_seconds * sample_rate;
    header.DataSize = num_samples * 2; // 2 bytes per sample (16-bit)

    // Update the ChunkSize field
    header.ChunkSize = 36 + header.DataSize;

    // Write the WAV header to the file
    fwrite(&header, sizeof(WAVHeader), 1, file);

    // Generate and write random samples
    srand(time(NULL));  // Seed the random number generator
    for (int i = 0; i < num_samples; i++) {
        int16_t sample = (rand() % 65536) - 32768; // Random 16-bit value (-32768 to 32767)
        fwrite(&sample, sizeof(int16_t), 1, file);
    }

    // Close the file
    fclose(file);
    printf("WAV file '%s' generated with random noise.\n", filename);
}

int main() {
    const char *filename = "random_noise.wav";
    int duration_seconds = 5;  // 5 seconds of random noise
    int sample_rate = 44100;   // 44.1 kHz sample rate

    generate_random_noise_wav(filename, duration_seconds, sample_rate);
    
    return 0;
}
