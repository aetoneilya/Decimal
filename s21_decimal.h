#ifndef SRC_S21_DECIMAL_H_
#define SRC_S21_DECIMAL_H_

typedef enum {
    s21_NORMAL_VALUE = 0,
    s21_INFINITY = 1,
    s21_NEGATIVE_INFINITY = 2,
    s21_NAN = 3,
    s21_COMPLEMENT = 4,
} value_type_t;

typedef struct {
    unsigned int bits[4];
    value_type_t value_type;
} s21_decimal;

typedef struct {
    unsigned int bits[6];
    value_type_t value_type;
} s21_super_decimal;

s21_decimal s21_add(s21_decimal left, s21_decimal right);
s21_decimal s21_sub(s21_decimal left, s21_decimal right);
s21_decimal s21_mul(s21_decimal left, s21_decimal right);
s21_decimal s21_div(s21_decimal left, s21_decimal right);
s21_decimal s21_mod(s21_decimal left, s21_decimal right);

int s21_is_less(s21_decimal left, s21_decimal right);
int s21_is_less_or_equal(s21_decimal left, s21_decimal right);
int s21_is_greater(s21_decimal left, s21_decimal right);
int s21_is_greater_or_equal(s21_decimal left, s21_decimal right);
int s21_is_equal(s21_decimal left, s21_decimal right);
int s21_is_not_equal(s21_decimal left, s21_decimal right);

int s21_from_int_to_decimal(int src, s21_decimal *dst);
int s21_from_float_to_decimal(float src, s21_decimal *dst);
int s21_from_decimal_to_int(s21_decimal src, int *dst);
int s21_from_decimal_to_float(s21_decimal src, float *dst);

s21_decimal s21_floor(s21_decimal decimal);
s21_decimal s21_round(s21_decimal decimal);
s21_decimal s21_truncate(s21_decimal decimal);
s21_decimal s21_negate(s21_decimal decimal);

#endif  // SRC_S21_DECIMAL_H_
