#include "core.h"

void s21_uint_set_bit(unsigned int *n, unsigned int pos) {
    *n |= 1UL << pos;
}

void s21_uint_reset_bit(unsigned int *n, unsigned int pos) {
    *n &= ~(1UL << pos);
}

unsigned int s21_uint_get_bit(unsigned int n, unsigned int pos) {
    return (n >> pos) & 1UL;
}

void s21_decimal_init(s21_decimal *decimal) {
    for (unsigned int i = 0; i < s21_DEC_SIZE; i++) {
        decimal->bits[i] = 0UL;
    }
    decimal->value_type = s21_NORMAL_VALUE;
}

void s21_decimal_set_bit(s21_decimal *decimal, unsigned int pos, unsigned int val) {
    if (val) {
        s21_uint_set_bit(&decimal->bits[pos / s21_INT_SIZE], pos % s21_INT_SIZE);
    } else {
        s21_uint_reset_bit(&decimal->bits[pos / s21_INT_SIZE], pos % s21_INT_SIZE);
    }
}

void s21_super_decimal_set_bit(s21_super_decimal *decimal, unsigned int pos, unsigned int val) {
    if (val) {
        s21_uint_set_bit(&decimal->bits[pos / s21_INT_SIZE], pos % s21_INT_SIZE);
    } else {
        s21_uint_reset_bit(&decimal->bits[pos / s21_INT_SIZE], pos % s21_INT_SIZE);
    }
}

unsigned int s21_decimal_get_bit(s21_decimal decimal, unsigned int pos) {
    return s21_uint_get_bit(decimal.bits[pos / s21_INT_SIZE], pos % s21_INT_SIZE);
}

unsigned int s21_super_decimal_get_bit(s21_super_decimal decimal, unsigned int pos) {
    return s21_uint_get_bit(decimal.bits[pos / s21_INT_SIZE], pos % s21_INT_SIZE);
}

void s21_decimal_set_sign(s21_decimal *decimal, unsigned int neg) {
    s21_decimal_set_bit(decimal, s21_SIGN_LBORDER, neg);
}

unsigned int s21_decimal_get_sign(s21_decimal decimal) {
    return s21_decimal_get_bit(decimal, s21_SIGN_LBORDER);
}

void s21_decimal_set_scale(s21_decimal *decimal, unsigned int val) {
    for (unsigned int i = s21_SCALE_LBORDER; i <= s21_SCALE_RBORDER; i++) {
        s21_decimal_set_bit(decimal, i, val & (1UL << (i - s21_SCALE_LBORDER)));
    }
}

unsigned int s21_decimal_get_scale(s21_decimal decimal) {
    return (decimal.bits[SCALE] >> (s21_SCALE_LBORDER - (s21_NUM_RBORDER + 1UL))) & s21_SCALE_MASK;
}

unsigned int s21_decimal_is_positive(s21_decimal decimal) {
    return s21_decimal_get_sign(decimal) == s21_SIGN_POS_VALUE;
}

unsigned int s21_decimal_is_negative(s21_decimal decimal) {
    return !s21_decimal_is_positive(decimal);
}

unsigned int s21_decimal_is_zero(s21_decimal decimal) {
    return decimal.bits[LOW] == 0 && decimal.bits[MID] == 0 && decimal.bits[HIGH] == 0;
}

int s21_decimal_scale_up(s21_decimal *decimal) {
    unsigned int scale = s21_decimal_get_scale(*decimal);
    int sign = s21_decimal_get_sign(*decimal);
    if (scale >= s21_SCALE_MAX_VALUE || decimal->value_type != s21_NORMAL_VALUE) return s21_error;
    s21_decimal ten = s21_decimal_get_const(S21_TEN);
    *decimal = s21_bitwise_mul(*decimal, ten);
    s21_decimal_set_scale(decimal, scale + 1);
    s21_decimal_set_sign(decimal, sign);
    if (decimal->value_type == s21_INFINITY) return s21_error;
    return s21_success;
}

int s21_decimal_scale_down(s21_decimal *decimal) {
    unsigned int scale = s21_decimal_get_scale(*decimal);
    int sign = s21_decimal_get_sign(*decimal);
    if (decimal->value_type != s21_NORMAL_VALUE) return s21_error;
    s21_decimal reminder;
    s21_decimal ten = s21_decimal_get_const(S21_TEN);
    *decimal = s21_int_div(*decimal, ten, &reminder);
    s21_decimal_set_scale(decimal, scale - 1);
    s21_decimal_set_sign(decimal, sign);
    return s21_success;
}

s21_decimal s21_decimal_squeeze(s21_decimal decimal) {
    s21_decimal tmp;
    s21_decimal reminder;
    s21_decimal ten;
    s21_decimal zero;
    s21_decimal_init(&zero);
    s21_decimal_init(&ten);
    ten.bits[LOW] = 10;

    unsigned int sign = s21_decimal_get_sign(decimal);
    unsigned int scale = s21_decimal_get_scale(decimal);

    while (s21_decimal_get_scale(decimal) > 0) {
        tmp = s21_int_div(decimal, ten, &reminder);
        scale--;

        if (s21_bits_compare(&reminder, &zero) == 0) {
            decimal = tmp;
            s21_decimal_set_scale(&decimal, scale);
        } else {
            break;
        }
    }

    s21_decimal_set_sign(&decimal, sign);

    return decimal;
}

int s21_bit_shift_left(s21_decimal *decimal) {
    unsigned int last_low_bit = s21_decimal_get_bit(*decimal, 31);
    unsigned int last_mid_bit = s21_decimal_get_bit(*decimal, 63);
    unsigned int last_high_bit = s21_decimal_get_bit(*decimal, 95);
    decimal->bits[0] = decimal->bits[0] << 1;
    decimal->bits[1] = decimal->bits[1] << 1;
    decimal->bits[2] = decimal->bits[2] << 1;
    s21_decimal_set_bit(decimal, 32, last_low_bit);
    s21_decimal_set_bit(decimal, 64, last_mid_bit);
    if (last_high_bit == 1) return s21_error;
    return s21_success;
}

int s21_super_bit_shift_left(s21_super_decimal *decimal) {
    int move_bits[6];
    for (int i = 1; i < 6; i++) {
        move_bits[i] = s21_super_decimal_get_bit(*decimal, i * s21_INT_SIZE - 1);
    }

    decimal->bits[0] = decimal->bits[0] << 1;
    decimal->bits[1] = decimal->bits[1] << 1;
    decimal->bits[2] = decimal->bits[2] << 1;
    decimal->bits[3] = decimal->bits[3] << 1;
    decimal->bits[4] = decimal->bits[4] << 1;
    decimal->bits[5] = decimal->bits[5] << 1;

    for (int i = 1; i < 6; i++) {
        s21_super_decimal_set_bit(decimal, i * s21_INT_SIZE, move_bits[i]);
    }

    return s21_success;
}

void s21_decimal_equalize_scales(s21_decimal *left, s21_decimal *right, unsigned int round) {
    unsigned int lscale = s21_decimal_get_scale(*left);
    unsigned int rscale = s21_decimal_get_scale(*right);

    if (lscale != rscale) {
        s21_decimal *lessd = lscale < rscale ? left : right;
        unsigned int scale = lscale < rscale ? (rscale - lscale) : (lscale - rscale);

        s21_decimal next = *lessd;
        while (scale && s21_decimal_scale_up(&next) == s21_success && next.value_type == s21_NORMAL_VALUE) {
            *lessd = next;
            scale--;
        }

        s21_decimal *greaterd = lessd != left ? left : right;
        s21_decimal greater = *greaterd;
        s21_status status = s21_success;
        while (scale && status == s21_success) {
            if (scale == 1 && round) {
                unsigned int t = s21_decimal_get_scale(*greaterd);
                s21_decimal d = *greaterd, digit;
                d = s21_int_div(d, s21_decimal_get_const(S21_TEN), &digit);
                while (s21_decimal_get_scale(greater) != t && digit.bits[LOW] == 0) {
                    d = s21_int_div(d, s21_decimal_get_const(S21_TEN), &digit);
                    t--;
                }

                s21_decimal five = s21_decimal_construct(0, 0, 5, 0, 0, s21_NORMAL_VALUE);
                if (s21_decimal_get_scale(greater) == t && digit.bits[LOW] == 5) {
                    s21_decimal d1, d2;
                    s21_int_div(*lessd, s21_decimal_get_const(S21_TEN), &d1);
                    s21_int_div(d, s21_decimal_get_const(S21_TEN), &d2);
                    if ((d1.bits[LOW] % 2 != 0) ^ (d2.bits[LOW] % 2 != 0))
                        greater = s21_bitwise_add(greater, five);
                } else {
                    greater = s21_bitwise_add(greater, five);
                }
            }

            status = s21_decimal_scale_down(&greater);
            scale--;
        }
        *greaterd = greater;
    }
}

s21_decimal s21_decimal_construct(unsigned int high_bit, unsigned int mid_bit, unsigned int low_bit,
                                  unsigned int sign, unsigned int scale, value_type_t type) {
    s21_decimal result;

    result.bits[LOW] = low_bit;
    result.bits[MID] = mid_bit;
    result.bits[HIGH] = high_bit;
    result.bits[SCALE] = 0;

    s21_decimal_set_scale(&result, scale);
    s21_decimal_set_sign(&result, sign);
    result.value_type = type;

    return result;
}

s21_decimal s21_decimal_get_const(s21_decimal_const constant) {
    const s21_decimal constants[7] = {
        s21_decimal_construct(0, 0, 0, 0, 0, s21_NORMAL_VALUE),
        s21_decimal_construct(0, 0, 10, 0, 0, s21_NORMAL_VALUE),
        s21_decimal_construct(0, 0, 1, 0, 0, s21_NORMAL_VALUE),
        s21_decimal_construct(0, 0, 5, 0, 0, s21_NORMAL_VALUE),
        s21_decimal_construct(0, 0, 0, 0, 0, s21_INFINITY),
        s21_decimal_construct(0, 0, 0, 0, 0, s21_NEGATIVE_INFINITY),
        s21_decimal_construct(0, 0, 0, 0, 0, s21_NAN)};

    return constants[constant];
}

s21_super_decimal dec_to_supdec(s21_decimal decimal) {
    s21_super_decimal sup_dec;
    sup_dec.bits[0] = decimal.bits[0];
    sup_dec.bits[1] = decimal.bits[1];
    sup_dec.bits[2] = decimal.bits[2];
    sup_dec.bits[3] = 0;
    sup_dec.bits[4] = 0;
    sup_dec.bits[5] = 0;
    sup_dec.value_type = s21_NORMAL_VALUE;
    return sup_dec;
}

s21_decimal supdec_to_dec(s21_super_decimal sup_decimal, unsigned int scale) {
    s21_super_decimal rem = dec_to_supdec(s21_decimal_get_const(S21_ZERO));
    s21_super_decimal zero = dec_to_supdec(s21_decimal_get_const(S21_ZERO));
    s21_super_decimal ten = dec_to_supdec(s21_decimal_get_const(S21_TEN));

    int round_count = 0;
    while (scale > s21_SCALE_MAX_VALUE) {
        sup_decimal = s21_super_int_div(sup_decimal, ten, &rem);
        scale--;
        if (s21_super_bits_compare(&rem, &zero) > 0)
            round_count++;
    }
    while ((sup_decimal.bits[3] != 0 || sup_decimal.bits[4] != 0 || sup_decimal.bits[5] != 0) &&
           (scale > 0)) {
        sup_decimal = s21_super_int_div(sup_decimal, ten, &rem);
        scale--;
        if (s21_super_bits_compare(&rem, &zero) > 0)
            round_count++;
    }

    s21_decimal result = s21_decimal_get_const(S21_ZERO);
    s21_super_decimal round = dec_to_supdec(s21_decimal_get_const(S21_FIVE));
    if (sup_decimal.bits[3] == 0 && sup_decimal.bits[4] == 0 && sup_decimal.bits[5] == 0) {
        result.bits[0] = sup_decimal.bits[0];
        result.bits[1] = sup_decimal.bits[1];
        result.bits[2] = sup_decimal.bits[2];
        s21_decimal_set_scale(&result, scale);
        if (round_count <= 1 && s21_super_bits_compare(&rem, &round) == 0
            && s21_decimal_get_bit(result, 0) == 1)
            result = s21_bitwise_add(result, s21_decimal_get_const(S21_ONE));
        else if (round_count > 1 && s21_super_bits_compare(&rem, &round) == 0)
            result = s21_bitwise_add(result, s21_decimal_get_const(S21_ONE));
        else if (s21_super_bits_compare(&rem, &round) > 0)
            result = s21_bitwise_add(result, s21_decimal_get_const(S21_ONE));
    } else {
        result = s21_decimal_get_const(S21_INFINITY_TYPE);
    }

    return result;
}
