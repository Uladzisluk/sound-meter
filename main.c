#include <alsa/asoundlib.h>
#include <math.h>

#define PCM_DEVICE "default"

short buffer[8*1024];
int buffer_size = sizeof(buffer)>>1;
double k = 0.75255;

double calculateRMS(short* buffer) {
    double rms = 0.0;
    for (size_t i = 0; i < buffer_size; ++i) {
        rms += buffer[i] * buffer[i];
    }
    rms = sqrt(rms / buffer_size);
    return rms;
}

int main() {
    int rc;
    snd_pcm_t *handle;

    // Открытие устройства PCM
    rc = snd_pcm_open(&handle, PCM_DEVICE, SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr, "Не удается открыть устройство PCM: %s\n", snd_strerror(rc));
        return 1;
    }

    rc = snd_pcm_set_params(handle,
                             SND_PCM_FORMAT_S16_LE,
                             SND_PCM_ACCESS_RW_INTERLEAVED,
                             1,
                             48000,
                             1,
                             500000);
    if (rc < 0) {
        fprintf(stderr, "Не удается применить параметры PCM: %s\n", snd_strerror(rc));
        return 1;
    }

    // Чтение данных с микрофона и измерение громкости
    while (1) {
        rc = snd_pcm_readi(handle, buffer, buffer_size);
        if (rc == -EPIPE) {
            fprintf(stderr, "Произошла ошибка переполнения буфера\n");
            snd_pcm_prepare(handle);
        } else if (rc < 0) {
            fprintf(stderr, "Не удается прочитать данные из устройства PCM: %s\n", snd_strerror(rc));
        } else if (rc > 0) {
            // Вычисление громкости (просто пример, не является точным измерением)
            double rms = calculateRMS(buffer);
            double db = 20 * log10(rms * k);

            printf("Громкость: %f db\n", db);
        }
    }

    // Закрытие устройства PCM
    snd_pcm_close(handle);

    return 0;
}
