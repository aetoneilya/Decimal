#include "core.h"

s21_decimal s21_negate(s21_decimal decimal) {
    if (decimal.value_type == s21_INFINITY) {
        decimal.value_type = s21_NEGATIVE_INFINITY;
    } else if (decimal.value_type == s21_NEGATIVE_INFINITY) {
        decimal.value_type = s21_INFINITY;
    } else if (decimal.value_type == s21_NORMAL_VALUE) {
        s21_decimal_set_sign(&decimal,
                             s21_decimal_get_sign(decimal) ? s21_SIGN_POS_VALUE : s21_SIGN_NEG_VALUE);
    }
    return decimal;
}

s21_decimal s21_floor(s21_decimal decimal) {
    if (decimal.value_type == s21_NORMAL_VALUE) {
        decimal = s21_decimal_squeeze(decimal);
        if (s21_decimal_get_scale(decimal) != 0) {
            while (s21_decimal_get_scale(decimal) > 1) {
                s21_decimal_scale_down(&decimal);
            }

            s21_decimal sign =
                s21_decimal_construct(0, 0, s21_decimal_get_sign(decimal), 0, 0, s21_NORMAL_VALUE);
            decimal = s21_sub(decimal, sign);

            s21_decimal_scale_down(&decimal);
        }
    }
    return decimal;
}

s21_decimal s21_round(s21_decimal decimal) {
    if (s21_decimal_get_scale(decimal) != 0 && decimal.value_type == s21_NORMAL_VALUE) {
        while (s21_decimal_get_scale(decimal) > 1) {
            s21_decimal_scale_down(&decimal);
        }

        s21_decimal round = s21_decimal_construct(0, 0, 5, 0, 1, s21_NORMAL_VALUE);
        s21_decimal_set_sign(&round, s21_decimal_get_sign(decimal));
        decimal = s21_add(decimal, round);

        s21_decimal_scale_down(&decimal);
    }
    return decimal;
}

s21_decimal s21_truncate(s21_decimal decimal) {
    if (decimal.value_type == s21_NORMAL_VALUE) {
        while (s21_decimal_get_scale(decimal) > 0) {
            s21_decimal_scale_down(&decimal);
        }
    }
    return decimal;
}
