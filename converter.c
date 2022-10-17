#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"

#define s21_FLOAT_MAX (79228162514264337593543950336.0)
#define s21_FLOAT_MIN (1e-28)
#define s21_INT_MAX (2147483647)

typedef union {
    float value;
    unsigned int n;
} s21_float;

int s21_from_int_to_decimal(int src, s21_decimal *dst) {
    if (dst == NULL) return s21_error;
    s21_decimal_init(dst);

    unsigned int neg = s21_uint_get_bit(src, s21_INT_SIZE - 1UL);
    if (neg) {
        src = ~(src) + 1;
    }

    dst->bits[0] = src;
    s21_decimal_set_sign(dst, neg);

    return s21_success;
}

void s21_set_value_type(float src, s21_decimal *dst) {
    if (isnan(src)) {
        dst->value_type = s21_NAN;
    } else if (isinf(src) || fabs(src) >= s21_FLOAT_MAX) {
        if (src > 0) {
            dst->value_type = s21_INFINITY;
        } else {
            dst->value_type = s21_NEGATIVE_INFINITY;
        }
    }
}

int s21_from_calc_precision(char *s, unsigned int count) {
    unsigned int is_start = 0;
    unsigned int prec = 0;
    unsigned int i = count + 1;
    while (isdigit(s[i]) && !is_start) {
        if (s[i] != '0')
            is_start = 1;
        else
            i++;
    }
    if (is_start) {
        while (isdigit(s[i + prec]) && prec < s21_FLOAT_PRECISION) {
            prec++;
        }
        prec += i - 1UL - count;
    }
    return prec;
}

void s21_from_float_to_str(float src, char *s, unsigned int length) {
    unsigned int count = 0;
    snprintf(s, length, "%.0f", src);
    while (isdigit(s[count])) {
        count++;
    }
    snprintf(s, length, "%#.*f", (int)(s21_SCALE_MAX_VALUE + 1UL) - count, src);
    for (unsigned int i = 0; i < 2; i++) {
        snprintf(s, length, "%.*f", s21_from_calc_precision(s, count), src);
    }
}

unsigned int s21_float_str_get_scale(char *s) {
    unsigned int scale = 0;
    for (unsigned int i = 0; i < strlen(s); i++) {
        if (s[i] == '.') {
            scale = strlen(s) - 1 - i;
        }
    }
    return scale;
}

void s21_float_str_trim(char *s) {
    unsigned int i = 0;
    while (isdigit(s[i])) {
        i++;
    }
    if (s[i] == '.') {
        i = strlen(s) - 1;
        while (i > 0 && s[i] == '0') {
            i--;
        }
        s[s[i] == '.' ? i : i + 1] = '\0';
    }
}

void s21_from_str_to_decimal(char *s, s21_decimal *dst) {
    s21_decimal digit;
    for (unsigned int i = 0; i < strlen(s); i++) {
        if (isdigit(s[i])) {
            s21_from_int_to_decimal(s[i] - '0', &digit);

            *dst = s21_bitwise_mul(*dst, s21_decimal_get_const(S21_TEN));
            *dst = s21_bitwise_add(*dst, digit);
        }
    }
}

int s21_from_float_to_decimal(float src, s21_decimal *dst) {
    if (dst == NULL) return s21_error;
    s21_status status = s21_error;

    s21_decimal_init(dst);
    s21_set_value_type(src, dst);
    if (dst->value_type == s21_NORMAL_VALUE && (fabs(src) > s21_FLOAT_MIN || fabs(src) == 0.)) {
        status = s21_success;

        char s[64] = {0};
        s21_from_float_to_str(fabs(src), s, sizeof(s));
        s21_float_str_trim(s);
        s21_from_str_to_decimal(s, dst);

        s21_float f = {.value = src};
        s21_decimal_set_sign(dst, s21_uint_get_bit(f.n, (s21_INT_SIZE - 1UL)));
        s21_decimal_set_scale(dst, s21_float_str_get_scale(s));
    }

    return status;
}

void s21_strrev(char *str) {
    int i = 0;
    int j = strlen(str) - 1;
    for (; i < j; i++, j--) {
        char t = str[i];
        str[i] = str[j];
        str[j] = t;
    }
}

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
    if (dst == NULL) return s21_error;
    s21_status status = s21_error;
    if (src.value_type == s21_NORMAL_VALUE) {
        src = s21_truncate(src);
        if (src.bits[HIGH] == 0 && src.bits[MID] == 0 &&
            ((s21_decimal_is_positive(src) && src.bits[LOW] <= s21_INT_MAX) ||
             (s21_decimal_is_negative(src) && src.bits[LOW] <= (s21_INT_MAX + 1UL)))) {
            status = s21_success;

            *dst = src.bits[LOW];
            if (s21_decimal_is_negative(src)) {
                *dst = ~src.bits[LOW] + 1;
            }
        }
    }
    return status;
}

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
    if (dst == NULL) return s21_error;
    if (src.value_type == s21_INFINITY || src.value_type == s21_NEGATIVE_INFINITY) {
        *dst = src.value_type == s21_INFINITY ? INFINITY : -INFINITY;
    } else if (src.value_type == s21_NAN) {
        *dst = NAN;
    } else {
        unsigned int scale = s21_decimal_get_scale(src);
        unsigned int is_negative = s21_decimal_is_negative(src);

        s21_decimal_set_scale(&src, 0);
        s21_decimal_set_sign(&src, s21_SIGN_POS_VALUE);

        char s[64];
        unsigned int i = 0, dot_ind = scale;
        do {
            s21_decimal digit;
            src = s21_int_div(src, s21_decimal_get_const(S21_TEN), &digit);

            s[i++] = digit.bits[LOW] + '0';
            if (i == dot_ind) s[i++] = '.';

            if (scale) scale--;
        } while (!s21_decimal_is_zero(src) || scale);
        if (s[i - 1] == '.') s[i++] = '0';
        if (is_negative) s[i++] = '-';
        s[i] = '\0';
        s21_strrev(s);

        *dst = strtof(s, NULL);
    }
    return s21_true;
}
