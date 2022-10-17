#ifndef SRC_CORE_H_
#define SRC_CORE_H_

#include "common.h"
#include "s21_decimal.h"

#define s21_INT_SIZE (32UL)
#define s21_DEC_SIZE (4UL)

#define s21_SUP_SCALE (6U)

typedef enum {
    s21_true,
    s21_false
} s21_bool;

typedef enum {
    s21_success,
    s21_error
} s21_status;

typedef enum {
    LOW,
    MID,
    HIGH,
    SCALE
} s21_segment;

#endif  // SRC_CORE_H_
