#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>

#define MAX_BINS 16

// Пороговые значения (по умолчанию)
float threshold_start = 1.6f;
float threshold_end = 1.5f;

int pulse_count = 0;
uint16_t bins[MAX_BINS] = { 0 };
bool inPulse = false;
float peak = 0.0f;

int NUM_BINS = 16;   // количество диапазонов (по умолчанию 16)
int channel_index = 3; // по умолчанию CH3

// Сердце программы. Функция для переноса на микрокотроллер
void processSample(float sample) {
    if (!inPulse) {
        // Начало нового импульса
        if (sample > threshold_start) {
            inPulse = true;
            peak = sample;
        }
    }
    else {
        // Продолжаем накапливать пик
        if (sample > peak) {
            peak = sample;
        }
        // Конец импульса
        if (sample < threshold_end) {
            inPulse = false;
            pulse_count++;
            // Классификация по амплитуде
            float amplitude = peak - threshold_start;
            // Большинство микроконтроллеров(например, STM32, AVR, ESP32) 
            // имеют опорное напряжение 3.3 В — это максимум, который АЦП может отобразить.
            int bin = (int)(amplitude / ((3.3f - threshold_start) / NUM_BINS));
            if (bin < 0) bin = 0;
            if (bin >= NUM_BINS) bin = NUM_BINS - 1;
            bins[bin]++;
        }
    }
}

int main(void) {
    SetConsoleOutputCP(CP_UTF8);
    char filename[32];
    int choice;

    float user_threshold_start = 0.0f, user_threshold_end = 0.0f;
    printf("Введите порог начала импульса (по умолчанию 1.6): ");
    scanf("%f", &user_threshold_start);
    if (user_threshold_start > 0.0f) threshold_start = user_threshold_start;

    printf("Введите порог конца импульса (по умолчанию 1.5): ");
    scanf("%f", &user_threshold_end);
    if (user_threshold_end > 0.0f) threshold_end = user_threshold_end;

    printf("Введите количество диапазонов (1-16): ");
    scanf("%d", &NUM_BINS);
    if (NUM_BINS < 1) NUM_BINS = 1;
    if (NUM_BINS > 16) NUM_BINS = 16;

    printf("Выберите файл:\n");
    printf("1 - lobz0.csv\n");
    printf("2 - lobz1.csv\n");
    printf("3 - lobz2.csv\n");
    printf("Ваш выбор: ");
    scanf("%d", &choice);

    switch (choice) {
    case 1: strcpy(filename, "lobz0.csv"); break;
    case 2: strcpy(filename, "lobz1.csv"); break;
    case 3: strcpy(filename, "lobz2.csv"); break;
    default: strcpy(filename, "lobz0.csv"); break;
    }

    printf("Выберите канал:\n");
    printf("1 - CH1(V)\n");
    printf("2 - CH2(V)\n");
    printf("3 - CH3(V)\n");
    printf("4 - CH4(V)\n");
    printf("5 - MATH1(V)\n");
    printf("Ваш выбор: ");
    scanf("%d", &channel_index);
    if (channel_index < 1 || channel_index > 5) channel_index = 3;

    FILE* f = fopen(filename, "r");
    if (!f) {
        printf("Ошибка: не удалось открыть %s\n", filename);
        return 1;
    }

    char line[256];
    fgets(line, sizeof(line), f);

    while (fgets(line, sizeof(line), f)) {
        float time, ch1, ch2, ch3, ch4, math1;
        if (sscanf(line, "%f,%f,%f,%f,%f,%f",
            &time, &ch1, &ch2, &ch3, &ch4, &math1) == 6) {
            float sample = 0.0f;
            switch (channel_index) {
            case 1: sample = ch1; break;
            case 2: sample = ch2; break;
            case 3: sample = ch3; break;
            case 4: sample = ch4; break;
            case 5: sample = math1; break;
            }
            processSample(sample);
        }
    }
    fclose(f);

    printf("\nФайл: %s\n", filename);
    printf("Канал: CH%d\n", channel_index);
    printf("Порог начала: %.2f В\n", threshold_start);
    printf("Порог конца: %.2f В\n", threshold_end);
    printf("Количество диапазонов: %d\n", NUM_BINS);
    printf("Общее число импульсов: %d\n", pulse_count);

    for (int i = 0; i < NUM_BINS; i++) {
        printf("Bin %2d: %u\n", i + 1, bins[i]);
    }
    printf("\nНажмите Enter для выхода...");
    getchar();
    getchar();

    return 0;
}
