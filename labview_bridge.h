#pragma once

#include <stdbool.h>

// Простая обертка для отправки данных пиков в LabVIEW через TCP
// Предполагается, что на стороне LabVIEW запущен TCP-сервер, принимающий строки
// формата: "P,<amplitude>,<bin>\n" или "N,<amplitude>,<bin>\n"

#ifdef __cplusplus
extern "C" {
#endif

bool lvb_init(const char* host, unsigned short port);
void lvb_close(void);

// is_positive: true для положительного пика, false для отрицательного
void lvb_send_peak(bool is_positive, float amplitude, int bin_index);

#ifdef __cplusplus
}
#endif


