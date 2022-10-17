#include "core.h"

s21_super_decimal s21_super_bitwise_add(s21_super_decimal left, s21_super_decimal right);
s21_super_decimal s21_super_bitwise_mul(s21_super_decimal right, s21_super_decimal left);
s21_super_decimal s21_super_to_complement(s21_super_decimal decimal);
s21_super_decimal s21_super_bitwise_sub(s21_super_decimal left, s21_super_decimal right);

s21_decimal s21_bitwise_add(s21_decimal left, s21_decimal right) {
    unsigned int carry = 0;
    for (unsigned int i = 0; i <= s21_NUM_RBORDER; i++) {
        unsigned int lbit = s21_decimal_get_bit(left, i);
        unsigned int rbit = s21_decimal_get_bit(right, i);
        if (lbit & rbit) {
            s21_decimal_set_bit(&left, i, carry);
            carry = 1;
        } else if (lbit ^ rbit) {
            s21_decimal_set_bit(&left, i, !carry);
        } else if (!lbit && !rbit && carry) {
            s21_decimal_set_bit(&left, i, carry);
            carry = 0;
        }
    }
    unsigned int is_complement = left.value_type == s21_COMPLEMENT || right.value_type == s21_COMPLEMENT;
    if (carry) {
        if (is_complement) {
            left.value_type = s21_NORMAL_VALUE;
            s21_decimal_set_sign(&left, s21_SIGN_POS_VALUE);
        } else {
            value_type_t val = s21_decimal_is_positive(left) ? s21_INFINITY : s21_NEGATIVE_INFINITY;
            s21_decimal_init(&left);
            left.value_type = val;
        }
    } else if (is_complement) {
        left.value_type = s21_COMPLEMENT;
        s21_decimal_set_sign(&left, s21_SIGN_NEG_VALUE);
    }
    return left;
}

s21_decimal s21_add(s21_decimal left, s21_decimal right) {
    s21_decimal result;
    if (left.value_type != s21_NORMAL_VALUE || right.value_type != s21_NORMAL_VALUE) {
        s21_decimal_init(&result);
        if (left.value_type == s21_NAN || right.value_type == s21_NAN ||
            (left.value_type == s21_NEGATIVE_INFINITY && right.value_type == s21_INFINITY) ||
            (left.value_type == s21_INFINITY && right.value_type == s21_NEGATIVE_INFINITY)) {
            result.value_type = s21_NAN;
        } else {
            result.value_type = left.value_type > right.value_type ? left.value_type : right.value_type;
        }
    } else {
        s21_decimal_equalize_scales(&left, &right, 1);
        unsigned int scale = s21_decimal_get_scale(left);
        do {
            if (s21_decimal_is_positive(left) && s21_decimal_is_positive(right)) {
                result = s21_bitwise_add(left, right);
            } else if (s21_decimal_is_negative(left) && s21_decimal_is_positive(right)) {
                s21_decimal_set_sign(&left, s21_SIGN_POS_VALUE);
                result = s21_sub(right, left);
            } else {
                s21_decimal_set_sign(&right, s21_SIGN_POS_VALUE);
                result = s21_sub(left, right);
            }
            if (scale) {
                s21_decimal_scale_down(&left);
                s21_decimal_scale_down(&right);
                scale--;
            }
        } while (result.value_type != s21_NORMAL_VALUE && scale);
    }
    return result;
}

s21_decimal s21_to_complement(s21_decimal decimal) {
    s21_decimal dec = decimal;
    for (unsigned int i = LOW; i < SCALE; i++) {
        dec.bits[i] = ~decimal.bits[i];
    }
    dec.value_type = decimal.value_type == s21_NORMAL_VALUE ? s21_COMPLEMENT : s21_NORMAL_VALUE;

    s21_decimal right;
    s21_from_int_to_decimal(1, &right);
    return s21_bitwise_add(dec, right);
}

s21_decimal s21_bitwise_sub(s21_decimal left, s21_decimal right) {
    s21_decimal result;
    if (s21_decimal_is_positive(left) && s21_decimal_is_positive(right)) {
        result = s21_bitwise_add(left, s21_to_complement(right));
    } else if (s21_decimal_is_negative(left) && s21_decimal_is_negative(right)) {
        result = s21_bitwise_add(s21_to_complement(left), right);
    } else {
        result = s21_bitwise_add(left, right);
        if (s21_decimal_is_negative(left) && result.value_type == s21_NORMAL_VALUE) {
            s21_decimal_set_sign(&result, s21_SIGN_NEG_VALUE);
        }
    }
    if (result.value_type == s21_COMPLEMENT) {
        result = s21_to_complement(result);
    }
    return result;
}

s21_decimal s21_sub(s21_decimal left, s21_decimal right) {
    s21_decimal result;
    if (left.value_type != s21_NORMAL_VALUE || right.value_type != s21_NORMAL_VALUE) {
        s21_decimal_init(&result);
        if (left.value_type == s21_NAN || right.value_type == s21_NAN ||
            (left.value_type == s21_INFINITY && right.value_type == s21_INFINITY) ||
            (left.value_type == s21_NEGATIVE_INFINITY && right.value_type == s21_NEGATIVE_INFINITY)) {
            result.value_type = s21_NAN;
        } else {
            result.value_type = left.value_type == s21_INFINITY ? s21_INFINITY : s21_NEGATIVE_INFINITY;
        }
    } else {
        s21_decimal_equalize_scales(&left, &right, 1);
        unsigned int scale = s21_decimal_get_scale(left);
        do {
            result = s21_bitwise_sub(left, right);
            if (scale) {
                s21_decimal_scale_down(&left);
                s21_decimal_scale_down(&right);
                scale--;
            }
        } while (result.value_type != s21_NORMAL_VALUE && scale);
    }
    return result;
}

s21_decimal s21_bitwise_mul(s21_decimal right, s21_decimal left) {
    s21_decimal result;
    s21_decimal_init(&result);
    s21_status bit_lost = s21_success;
    for (int i = 0; i < 96; i++) {
        int bit_left = s21_decimal_get_bit(left, i);
        if (bit_left == 1 && bit_lost == s21_error) {
            result.value_type = s21_INFINITY;
        }
        if (bit_left == 1) {
            result = s21_bitwise_add(result, right);
        }
        bit_lost = s21_bit_shift_left(&right);
    }
    if (result.value_type == s21_INFINITY) result = s21_decimal_get_const(S21_INFINITY_TYPE);
    return result;
}

s21_decimal s21_int_div(s21_decimal dividend, s21_decimal divisor, s21_decimal* remainder) {
    *remainder = s21_decimal_get_const(S21_ZERO);
    for (int N = 96; N > 0; N--) {
        s21_bit_shift_left(remainder);
        s21_decimal_set_bit(remainder, 0, s21_decimal_get_bit(dividend, s21_NUM_RBORDER));
        s21_bit_shift_left(&dividend);
        *remainder = s21_bitwise_sub(*remainder, divisor);
        if (s21_decimal_is_negative(*remainder)) *remainder = s21_to_complement(*remainder);
        if (s21_decimal_get_bit(*remainder, s21_NUM_RBORDER) == 1) {
            s21_decimal_set_bit(&dividend, 0, 0);
            *remainder = s21_bitwise_add(*remainder, divisor);
        } else {
            s21_decimal_set_bit(&dividend, 0, 1);
        }
    }
    return dividend;
}

s21_decimal s21_mod(s21_decimal left, s21_decimal right) {
    s21_decimal mod;
    if (left.value_type == s21_NORMAL_VALUE &&
        (right.value_type == s21_INFINITY || right.value_type == s21_NEGATIVE_INFINITY)) {
        mod = left;
    } else if (left.value_type == s21_NORMAL_VALUE && right.value_type == s21_NORMAL_VALUE) {
        s21_super_decimal super_left = dec_to_supdec(left);
        s21_super_decimal super_right = dec_to_supdec(right);
        unsigned int left_scale = s21_decimal_get_scale(left);
        unsigned int right_scale = s21_decimal_get_scale(right);

        s21_super_decimal ten = dec_to_supdec(s21_decimal_get_const(S21_TEN));

        while (right_scale < left_scale) {
            super_right = s21_super_bitwise_mul(super_right, ten);
            right_scale++;
        }

        while (right_scale > left_scale) {
            super_left = s21_super_bitwise_mul(super_left, ten);
            left_scale++;
        }

        s21_super_decimal super_mod;
        s21_super_int_div(super_left, super_right, &super_mod);

        mod = supdec_to_dec(super_mod, left_scale);
        unsigned int sign = s21_decimal_get_sign(left);
        s21_decimal_set_sign(&mod, sign);

    } else {
        mod = s21_decimal_get_const(S21_NAN_TYPE);
    }

    return mod;
}

s21_decimal s21_div(s21_decimal left, s21_decimal right) {
    s21_decimal zero = s21_decimal_get_const(S21_ZERO);
    unsigned int sign_sum = s21_decimal_get_sign(left) + s21_decimal_get_sign(right);

    s21_decimal res;
    if (right.value_type == s21_NORMAL_VALUE && s21_bits_compare(&right, &zero) == 0) {
        if (left.value_type == s21_NORMAL_VALUE && s21_bits_compare(&left, &zero) == 0)
            res = s21_decimal_get_const(S21_NAN_TYPE);
        else
            res = s21_decimal_get_sign(left) ? s21_decimal_get_const(S21_NEGATIVE_INFINITY_TYPE)
                                             : s21_decimal_get_const(S21_INFINITY_TYPE);
    } else if (left.value_type == s21_NORMAL_VALUE &&
               (right.value_type == s21_INFINITY || right.value_type == s21_NEGATIVE_INFINITY)) {
        res = s21_decimal_get_const(S21_ZERO);
    } else if ((left.value_type == s21_INFINITY || left.value_type == s21_NEGATIVE_INFINITY) &&
               right.value_type == s21_NORMAL_VALUE) {
        if (s21_decimal_get_sign(right))
            res = left.value_type == s21_INFINITY ? s21_decimal_get_const(S21_NEGATIVE_INFINITY_TYPE)
                                                  : s21_decimal_get_const(S21_INFINITY_TYPE);
        else
            res = left.value_type == s21_INFINITY ? s21_decimal_get_const(S21_INFINITY_TYPE)
                                                  : s21_decimal_get_const(S21_NEGATIVE_INFINITY_TYPE);
    } else if (right.value_type == s21_NORMAL_VALUE && left.value_type == s21_NORMAL_VALUE) {
        right = s21_decimal_squeeze(right);

        s21_super_decimal super_max_upscale;
        super_max_upscale.bits[0] = 0x99999999;
        super_max_upscale.bits[1] = 0x99999999;
        super_max_upscale.bits[2] = 0x99999999;
        super_max_upscale.bits[3] = 0x99999999;
        super_max_upscale.bits[4] = 0x99999999;
        super_max_upscale.bits[5] = 0x19999999;

        s21_super_decimal super_left = dec_to_supdec(left);
        s21_super_decimal super_right = dec_to_supdec(right);

        s21_super_decimal ten = dec_to_supdec(s21_decimal_get_const(S21_TEN));

        int new_scale = s21_decimal_get_scale(left) - s21_decimal_get_scale(right);
        while (s21_super_bits_compare(&super_left, &super_max_upscale) <= 0) {
            super_left = s21_super_bitwise_mul(super_left, ten);
            new_scale++;
        }

        s21_super_decimal mod;
        res = supdec_to_dec(s21_super_int_div(super_left, super_right, &mod), new_scale);
        res = s21_decimal_squeeze(res);
        s21_decimal_set_sign(&res, sign_sum % 2);
        if (res.value_type == s21_INFINITY && s21_decimal_get_sign(res) == 1)
            res = s21_decimal_get_const(S21_NEGATIVE_INFINITY_TYPE);
    } else {
        res = s21_decimal_get_const(S21_NAN_TYPE);
    }
    return res;
}

s21_decimal s21_mul(s21_decimal left, s21_decimal right) {
    s21_decimal result = s21_decimal_get_const(S21_ZERO);
    s21_decimal zero = s21_decimal_get_const(S21_ZERO);
    if (((right.value_type == s21_INFINITY || right.value_type == s21_NEGATIVE_INFINITY) &&
         s21_bits_compare(&left, &zero) == 0) ||
        ((left.value_type == s21_INFINITY || left.value_type == s21_NEGATIVE_INFINITY) &&
         s21_bits_compare(&right, &zero) == 0)) {
        result = s21_decimal_get_const(S21_NAN_TYPE);
    } else if ((right.value_type == s21_INFINITY || right.value_type == s21_NEGATIVE_INFINITY) &&
               left.value_type == s21_NORMAL_VALUE) {
        result.value_type = right.value_type == s21_INFINITY
                                ? s21_INFINITY + s21_decimal_get_sign(left)
                                : s21_NEGATIVE_INFINITY - s21_decimal_get_sign(left);
    } else if ((left.value_type == s21_INFINITY || left.value_type == s21_NEGATIVE_INFINITY) &&
               right.value_type == s21_NORMAL_VALUE) {
        result.value_type = left.value_type == s21_INFINITY
                                ? s21_INFINITY + s21_decimal_get_sign(right)
                                : s21_NEGATIVE_INFINITY - s21_decimal_get_sign(right);
    } else if ((left.value_type == s21_INFINITY || left.value_type == s21_NEGATIVE_INFINITY) &&
               (right.value_type == s21_INFINITY || right.value_type == s21_NEGATIVE_INFINITY)) {
        result.value_type = 1 + (left.value_type + right.value_type) % 2;
    } else if (right.value_type == s21_NORMAL_VALUE && left.value_type == s21_NORMAL_VALUE) {
        right = s21_decimal_squeeze(right);
        left = s21_decimal_squeeze(left);

        unsigned int sign_sum = s21_decimal_get_sign(left) + s21_decimal_get_sign(right);
        unsigned int sum_scale = s21_decimal_get_scale(left) + s21_decimal_get_scale(right);

        s21_super_decimal super_right = dec_to_supdec(right);
        s21_super_decimal super_left = dec_to_supdec(left);
        result = supdec_to_dec(s21_super_bitwise_mul(super_right, super_left), sum_scale);
        s21_decimal_set_sign(&result, sign_sum % 2);

        if (result.value_type == s21_INFINITY && s21_decimal_get_sign(result) == 1) {
            result = s21_decimal_get_const(S21_NEGATIVE_INFINITY_TYPE);
        }
    } else {
        result.value_type = s21_NAN;
    }
    return result;
}

s21_super_decimal s21_super_bitwise_mul(s21_super_decimal right, s21_super_decimal left) {
    s21_super_decimal result = dec_to_supdec(s21_decimal_get_const(S21_ZERO));
    for (unsigned int i = 0; i <= s21_NUM_RBORDER; i++) {
        int bit_left = s21_super_decimal_get_bit(left, i);
        if (bit_left == 1) {
            result = s21_super_bitwise_add(result, right);
        }
        s21_super_bit_shift_left(&right);
    }
    return result;
}

s21_super_decimal s21_super_bitwise_add(s21_super_decimal left, s21_super_decimal right) {
    unsigned int carry = 0;
    for (unsigned int i = 0; i <= s21_SUP_NUM_RBORDER; i++) {
        unsigned int lbit = s21_super_decimal_get_bit(left, i);
        unsigned int rbit = s21_super_decimal_get_bit(right, i);
        if (lbit & rbit) {
            s21_super_decimal_set_bit(&left, i, carry);
            carry = 1;
        } else if (lbit ^ rbit) {
            s21_super_decimal_set_bit(&left, i, !carry);
        } else if (!lbit && !rbit && carry) {
            s21_super_decimal_set_bit(&left, i, carry);
            carry = 0;
        }
    }
    return left;
}

s21_super_decimal s21_super_to_complement(s21_super_decimal decimal) {
    s21_super_decimal dec = decimal;
    for (unsigned int i = LOW; i < s21_SUP_SCALE; i++) {
        dec.bits[i] = ~decimal.bits[i];
    }
    dec.value_type = decimal.value_type == s21_NORMAL_VALUE ? s21_COMPLEMENT : s21_NORMAL_VALUE;

    s21_super_decimal right;
    right.bits[LOW] = 1;
    for (unsigned int i = MID; i < s21_SUP_SCALE; i++) {
        right.bits[i] = 0;
    }
    return s21_super_bitwise_add(dec, right);
}

s21_super_decimal s21_super_bitwise_sub(s21_super_decimal left, s21_super_decimal right) {
    s21_super_decimal result = s21_super_bitwise_add(left, s21_super_to_complement(right));
    return result;
}

s21_super_decimal s21_super_int_div(s21_super_decimal dividend, s21_super_decimal divisor,
                                    s21_super_decimal* remainder) {
    *remainder = dec_to_supdec(s21_decimal_get_const(S21_ZERO));
    for (int N = 192; N > 0; N--) {
        s21_super_bit_shift_left(remainder);
        s21_super_decimal_set_bit(remainder, 0, s21_super_decimal_get_bit(dividend, 191));
        s21_super_bit_shift_left(&dividend);
        *remainder = s21_super_bitwise_sub(*remainder, divisor);
        if (s21_super_decimal_get_bit(*remainder, 191) == 1) {
            s21_super_decimal_set_bit(&dividend, 0, 0);
            *remainder = s21_super_bitwise_add(*remainder, divisor);
        } else {
            s21_super_decimal_set_bit(&dividend, 0, 1);
        }
    }
    return dividend;
}
