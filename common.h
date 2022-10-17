#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include "s21_decimal.h"

#define s21_SUP_NUM_RBORDER (191UL)

#define s21_NUM_RBORDER (95UL)
#define s21_SCALE_LBORDER (112UL)
#define s21_SCALE_RBORDER (119UL)
#define s21_SCALE_MASK (0x1FUL)
#define s21_SCALE_MAX_VALUE (28UL)
#define s21_FLOAT_PRECISION (7UL)
#define s21_SIGN_LBORDER (127UL)
#define s21_SIGN_POS_VALUE (0UL)
#define s21_SIGN_NEG_VALUE (1UL)

typedef enum {
    S21_ZERO = 0,
    S21_TEN = 1,
    S21_ONE,
    S21_FIVE,
    S21_INFINITY_TYPE,
    S21_NEGATIVE_INFINITY_TYPE,
    S21_NAN_TYPE
} s21_decimal_const;

void s21_uint_set_bit(unsigned int *n, unsigned int pos);
void s21_uint_reset_bit(unsigned int *n, unsigned int pos);
unsigned int s21_uint_get_bit(unsigned int n, unsigned int pos);

void s21_decimal_init(s21_decimal *decimal);
void s21_decimal_set_bit(s21_decimal *decimal, unsigned int pos, unsigned int val);
unsigned int s21_decimal_get_bit(s21_decimal decimal, unsigned int pos);
void s21_decimal_set_sign(s21_decimal *decimal, unsigned int neg);
unsigned int s21_decimal_get_sign(s21_decimal decimal);
void s21_decimal_set_scale(s21_decimal *decimal, unsigned int val);
unsigned int s21_decimal_get_scale(s21_decimal decimal);
unsigned int s21_decimal_is_positive(s21_decimal decimal);
unsigned int s21_decimal_is_negative(s21_decimal decimal);
unsigned int s21_decimal_is_zero(s21_decimal decimal);

s21_decimal s21_decimal_construct(unsigned int high_bit, unsigned int mid_bit, unsigned int low_bit,
                                  unsigned int sign, unsigned int scale, value_type_t type);
s21_decimal s21_decimal_get_const(s21_decimal_const constant);
int s21_bits_compare(s21_decimal *first, s21_decimal *second);
int s21_bit_shift_left(s21_decimal *decimal);
s21_decimal s21_int_div(s21_decimal dividend, s21_decimal divisor, s21_decimal *remainder);
int s21_decimal_scale_up(s21_decimal *decimal);
int s21_decimal_scale_down(s21_decimal *decimal);
int s21_set_decimal_state(s21_decimal *decimal, int state);
void s21_decimal_equalize_scales(s21_decimal *left, s21_decimal *right, unsigned int round);
s21_decimal s21_decimal_squeeze(s21_decimal decimal);
s21_decimal s21_bitwise_mul(s21_decimal right, s21_decimal left);
s21_decimal s21_bitwise_add(s21_decimal left, s21_decimal right);

int s21_super_bit_shift_left(s21_super_decimal *decimal);
s21_super_decimal dec_to_supdec(s21_decimal decimal);
s21_decimal supdec_to_dec(s21_super_decimal sup_decimal, unsigned int scale);
void s21_super_decimal_set_bit(s21_super_decimal *decimal, unsigned int pos, unsigned int val);
unsigned int s21_super_decimal_get_bit(s21_super_decimal decimal, unsigned int pos);
int s21_super_bits_compare(s21_super_decimal *first, s21_super_decimal *second);
s21_super_decimal s21_super_int_div(s21_super_decimal dividend, s21_super_decimal divisor,
                                    s21_super_decimal *remainder);
s21_super_decimal s21_super_bitwise_add(s21_super_decimal left, s21_super_decimal right);

#endif  // SRC_COMMON_H_
