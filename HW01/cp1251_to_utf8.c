#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

/* Таблица соответствий CP1251 (0x80..0xFF) -> Unicode code points */
static const uint16_t cp1251_to_unicode[128] = {
    0x0402,0x0403,0x201A,0x0453,0x201E,0x2026,0x2020,0x2021,
    0x20AC,0x2030,0x0409,0x2039,0x040A,0x040C,0x040B,0x040F,
    0x0452,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
    0x0098,0x2122,0x0459,0x203A,0x045A,0x045C,0x045B,0x045F,
    0x00A0,0x040E,0x045E,0x0408,0x00A4,0x0490,0x00A6,0x00A7,
    0x0401,0x00A9,0x0404,0x00AB,0x00AC,0x00AD,0x00AE,0x0407,
    0x00B0,0x00B1,0x0406,0x0456,0x0491,0x00B5,0x00B6,0x00B7,
    0x0451,0x2116,0x0454,0x00BB,0x0458,0x0405,0x0455,0x0457,
    0x0410,0x0411,0x0412,0x0413,0x0414,0x0415,0x0416,
    0x0417,0x0418,0x0419,0x041A,0x041B,0x041C,0x041D,0x041E,
    0x041F,0x0420,0x0421,0x0422,0x0423,0x0424,0x0425,0x0426,
    0x0427,0x0428,0x0429,0x042A,0x042B,0x042C,0x042D,0x042E,
    0x042F,0x0430,0x0431,0x0432,0x0433,0x0434,0x0435,0x0436,
    0x0437,0x0438,0x0439,0x043A,0x043B,0x043C,0x043D,0x043E,
    0x043F,0x0440,0x0441,0x0442,0x0443,0x0444,0x0445,0x0446,
    0x0447,0x0448,0x0449,0x044A,0x044B,0x044C,0x044D,0x044E,
    0x044F
};

/*
 * cp1251_to_utf8
 *
 * Вход:
 *   in      - указатель на буфер с входными байтами в CP1251
 *   in_len  - длина входного буфера
 * Выход:
 *   out     - адрес указателя, в который будет записан malloc'енный UTF-8 буфер
 *   out_len - указатель на размер полученного буфера
 *
 * Возвращает:
 *   0  - успех (caller должен free(*out))
 *  -1  - ошибка (errno установлен)
 *
 * Замечания:
 *  - Функция выделяет (in_len * 3) + 1 байт (хватает для всех CP1251 кодпоинтов: максимум 3 байта UTF-8).
 *  - На ошибке выделения возвращает -1 и устанавливает errno = ENOMEM.
 */
int cp1251_to_utf8(const unsigned char *in, size_t in_len, unsigned char **out, size_t *out_len) {
    if (!in || !out || !out_len) {
        errno = EINVAL;
        return -1;
    }

    /* максимум 3 байта UTF-8 на входный байт (CP1251 кодпоинты <= U+0491), +1 для NUL если надо */
    size_t max_out = in_len * 3 + 1;
    unsigned char *buf = malloc(max_out);
    if (!buf) {
        errno = ENOMEM;
        return -1;
    }

    size_t w = 0;
    for (size_t i = 0; i < in_len; ++i) {
        unsigned char b = in[i];
        if (b < 0x80) {
            /* ASCII: один байт */
            buf[w++] = b;
        } else {
            uint32_t cp = cp1251_to_unicode[b - 0x80];
            /* Кодовые точки CP1251 всегда <= 0x0491, так что нужны только 2..3 байта */
            if (cp <= 0x7F) {
                buf[w++] = (unsigned char)cp;
            } else if (cp <= 0x7FF) {
                buf[w++] = (unsigned char)(0xC0 | ((cp >> 6) & 0x1F));
                buf[w++] = (unsigned char)(0x80 | (cp & 0x3F));
            } else if (cp <= 0xFFFF) {
                /* Проверим суррогатный диапазон на случай некорректных таблиц */
                if (cp >= 0xD800 && cp <= 0xDFFF) {
                    /* Нежелательный одиночный суррогат — заменим на U+FFFD */
                    cp = 0xFFFD;
                }
                buf[w++] = (unsigned char)(0xE0 | ((cp >> 12) & 0x0F));
                buf[w++] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
                buf[w++] = (unsigned char)(0x80 | (cp & 0x3F));
            } else {
                /* Для полноты: если cp > 0xFFFF (не в CP1251), используем U+FFFD */
                cp = 0xFFFD;
                buf[w++] = (unsigned char)(0xE0 | ((cp >> 12) & 0x0F));
                buf[w++] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
                buf[w++] = (unsigned char)(0x80 | (cp & 0x3F));
            }
        }
    }

    /* Опционально можно NUL-терминировать (удобно для строк) */
    buf[w] = '\0';

    /* По желанию можно перештатировать буфер на w+1 байт, но не обязательно */
    unsigned char *shrunk = (unsigned char *)realloc(buf, w + 1);
    if (shrunk) buf = shrunk; /* если realloc не удался, старый buf всё ещё корректен */

    *out = buf;
    *out_len = w;
    return 0;
}


int main(int argc, char **argv) {
    const char *infile = (argc > 1) ? argv[1] : NULL;
    FILE *f = infile ? fopen(infile, "rb") : stdin;
    if (!f) {
        perror("fopen");
        return 2;
    }

    if (fseek(f, 0, SEEK_END) != 0) { perror("fseek"); return 3; }
    long sz = ftell(f);
    if (sz < 0) { perror("ftell"); return 4; }
    rewind(f);

    unsigned char *inbuf = malloc((size_t)sz);
    if (!inbuf) { perror("malloc"); return 5; }
    if (fread(inbuf, 1, (size_t)sz, f) != (size_t)sz) { perror("fread"); return 6; }
    if (f != stdin) fclose(f);

    unsigned char *outbuf = NULL;
    size_t out_len = 0;
    if (cp1251_to_utf8(inbuf, (size_t)sz, &outbuf, &out_len) != 0) {
        perror("cp1251_to_utf8");
        free(inbuf);
        return 7;
    }

    /* записать в stdout */
    if (fwrite(outbuf, 1, out_len, stdout) != out_len) {
        perror("fwrite");
    }

    free(inbuf);
    free(outbuf);
    return 0;
}
