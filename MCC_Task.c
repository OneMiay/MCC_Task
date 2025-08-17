#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>

#define MAX_BINS 16

// Пороговые значения (по умолчанию)
float threshold_start = 1.6f;   // старт положительного импульса
float threshold_end = 1.5f;   // окончание положительного импульса

// Для отрицательных импульсов возьмем те же величины, но с минусом
// Например, -1.6 В и -1.5 В
// Это можно доработать — ввести отдельные переменные, если нужно раздельно настраивать

// Статистика
int pulse_count_pos = 0;
int pulse_count_neg = 0;
uint16_t bins_pos[MAX_BINS] = { 0 };
uint16_t bins_neg[MAX_BINS] = { 0 };

// Состояния
bool inPulsePos = false;
bool inPulseNeg = false;

float peakPos = 0.0f;
float peakNeg = 0.0f;

int NUM_BINS = 16;    // количество диапазонов (по умолчанию 16)
int channel_index = 3; // по умолчанию CH3

// Универсальная функция вычисления бина по амплитуде
int getBinIndex(float amplitude) {
    // Большинство микроконтроллеров(например, STM32, AVR, ESP32) 
    // // имеют опорное напряжение 3.3 В — это максимум, который АЦП может отобразить.
    int bin = (int)(amplitude / ((3.3f - threshold_start) / NUM_BINS));
    if (bin < 0) bin = 0;
    if (bin >= NUM_BINS) bin = NUM_BINS - 1;
    return bin;
}

// Сердце программы — обрабатывает и положительные, и отрицательные импульсы
void processSample(float sample) {
    // --- Положительный импульс ---
    if (!inPulsePos) {
        if (sample > threshold_start) {
            inPulsePos = true;
            peakPos = sample;
        }
    }
    else {
        if (sample > peakPos) peakPos = sample;
        if (sample < threshold_end) {
            inPulsePos = false;
            pulse_count_pos++;
            float amplitude = peakPos - threshold_start;
            int bin = getBinIndex(amplitude);
            bins_pos[bin]++;
        }
    }

    // --- Отрицательный импульс (инверсный) ---
    if (!inPulseNeg) {
        if (sample < -threshold_start) {
            inPulseNeg = true;
            peakNeg = sample; // для отрицательного это будет минимум
        }
    }
    else {
        if (sample < peakNeg) peakNeg = sample;
        if (sample > -threshold_end) {
            inPulseNeg = false;
            pulse_count_neg++;
            float amplitude = (-threshold_start) - peakNeg; // разница в отрицательной области
            int bin = getBinIndex(amplitude);
            bins_neg[bin]++;
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
    fgets(line, sizeof(line), f); // пропускаем заголовок

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
    printf("Порог начала (положит.): %.2f В\n", threshold_start);
    printf("Порог конца   (положит.): %.2f В\n", threshold_end);
    printf("Количество диапазонов: %d\n", NUM_BINS);

    printf("\n--- Положительные импульсы ---\n");
    printf("Общее число: %d\n", pulse_count_pos);
    for (int i = 0; i < NUM_BINS; i++) {
        printf("Bin %2d: %u\n", i + 1, bins_pos[i]);
    }

    printf("\n--- Отрицательные (инверсные) импульсы ---\n");
    printf("Общее число: %d\n", pulse_count_neg);
    for (int i = 0; i < NUM_BINS; i++) {
        printf("Bin %2d: %u\n", i + 1, bins_neg[i]);
    }

    printf("\nНажмите Enter для выхода...");
    getchar();
    getchar();

    return 0;
}
