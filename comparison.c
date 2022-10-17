#include "core.h"

int s21_bits_compare(s21_decimal* first, s21_decimal* second) {
    for (int i = HIGH; i >= LOW; i--) {
        if (first->bits[i] > second->bits[i]) return 1;
        if (first->bits[i] < second->bits[i]) return -1;
    }
    return 0;
}

int s21_super_bits_compare(s21_super_decimal* first, s21_super_decimal* second) {
    for (int i = 5; i >= LOW; i--) {
        if (first->bits[i] > second->bits[i]) return 1;
        if (first->bits[i] < second->bits[i]) return -1;
    }
    return 0;
}

int s21_is_less(s21_decimal left, s21_decimal right) {
    s21_bool is_less = s21_false;
    if (left.value_type != s21_NORMAL_VALUE || right.value_type != s21_NORMAL_VALUE) {
        if ((left.value_type == s21_NORMAL_VALUE && right.value_type == s21_INFINITY) ||
            (left.value_type == s21_NEGATIVE_INFINITY && (right.value_type == s21_NORMAL_VALUE ||
                                                          right.value_type == s21_INFINITY))) {
            is_less = s21_true;
        }
    } else {
        s21_decimal_equalize_scales(&left, &right, 0);
        if (s21_decimal_get_sign(left) == s21_decimal_get_sign(right)) {
            unsigned int i = s21_NUM_RBORDER + 1UL;
            while (i && s21_decimal_get_bit(left, i - 1) == s21_decimal_get_bit(right, i - 1)) {
                i--;
            }
            if (i && ((s21_decimal_is_positive(left) && s21_decimal_get_bit(right, i - 1)) ||
                      (s21_decimal_is_negative(left) && s21_decimal_get_bit(left, i - 1)))) {
                is_less = s21_true;
            }
        } else if (s21_decimal_is_negative(left) && !(s21_decimal_is_zero(left) &&
                                                      s21_decimal_is_zero(right))) {
            is_less = s21_true;
        }
    }
    return is_less;
}

int s21_is_less_or_equal(s21_decimal left, s21_decimal right) {
    s21_bool is_less_or_equal = s21_false;
    if (left.value_type != s21_NORMAL_VALUE || right.value_type != s21_NORMAL_VALUE) {
        if ((left.value_type == s21_NORMAL_VALUE && right.value_type == s21_INFINITY) ||
            (left.value_type == s21_INFINITY && right.value_type == s21_INFINITY) ||
            (left.value_type == s21_NEGATIVE_INFINITY && right.value_type != s21_NAN)) {
            is_less_or_equal = s21_true;
        }
    } else {
        is_less_or_equal = !s21_is_greater(left, right);
    }
    return is_less_or_equal;
}

int s21_is_greater(s21_decimal left, s21_decimal right) {
    s21_bool is_greater = s21_false;
    if (left.value_type != s21_NORMAL_VALUE || right.value_type != s21_NORMAL_VALUE) {
        if ((left.value_type == s21_NORMAL_VALUE && right.value_type == s21_NEGATIVE_INFINITY) ||
            (left.value_type == s21_INFINITY && (right.value_type == s21_NEGATIVE_INFINITY ||
                                                 right.value_type == s21_NORMAL_VALUE))) {
            is_greater = s21_true;
        }
    } else {
        s21_decimal_equalize_scales(&left, &right, 0);
        if (s21_decimal_get_sign(left) == s21_decimal_get_sign(right)) {
            unsigned int i = s21_NUM_RBORDER + 1UL;
            while (i && s21_decimal_get_bit(left, i - 1) == s21_decimal_get_bit(right, i - 1)) {
                i--;
            }
            if (i && ((s21_decimal_is_positive(left) && s21_decimal_get_bit(left, i - 1)) ||
                      (s21_decimal_is_negative(left) && s21_decimal_get_bit(right, i - 1)))) {
                is_greater = s21_true;
            }
        } else if (s21_decimal_is_positive(left) && !(s21_decimal_is_zero(left) &&
                                                      s21_decimal_is_zero(right))) {
            is_greater = s21_true;
        }
    }
    return is_greater;
}

int s21_is_greater_or_equal(s21_decimal left, s21_decimal right) {
    s21_bool is_greater_or_equal = s21_false;
    if (left.value_type != s21_NORMAL_VALUE || right.value_type != s21_NORMAL_VALUE) {
        if ((left.value_type == s21_NORMAL_VALUE && right.value_type == s21_NEGATIVE_INFINITY) ||
            (left.value_type == s21_INFINITY && right.value_type != s21_NAN) ||
            (left.value_type == s21_NEGATIVE_INFINITY && right.value_type == s21_NEGATIVE_INFINITY)) {
            is_greater_or_equal = s21_true;
        }
    } else {
        is_greater_or_equal = !s21_is_less(left, right);
    }
    return is_greater_or_equal;
}

int s21_is_equal(s21_decimal left, s21_decimal right) {
    s21_bool is_equal = s21_false;
    if (left.value_type != s21_NORMAL_VALUE || right.value_type != s21_NORMAL_VALUE) {
        if ((left.value_type == s21_INFINITY && right.value_type == s21_INFINITY) ||
            (left.value_type == s21_NEGATIVE_INFINITY && right.value_type == s21_NEGATIVE_INFINITY)) {
            is_equal = s21_true;
        }
    } else {
        s21_decimal_equalize_scales(&left, &right, 0);
        if (s21_decimal_get_sign(left) == s21_decimal_get_sign(right)) {
            unsigned int i = s21_NUM_RBORDER + 1UL;
            while (i && s21_decimal_get_bit(left, i - 1) == s21_decimal_get_bit(right, i - 1)) {
                i--;
            }
            if (!i) {
                is_equal = s21_true;
            }
        } else if (s21_decimal_is_zero(left) && s21_decimal_is_zero(right)) {
            is_equal = s21_true;
        }
    }
    return is_equal;
}

int s21_is_not_equal(s21_decimal left, s21_decimal right) {
    s21_bool is_not_equal = s21_false;
    if (left.value_type != s21_NORMAL_VALUE || right.value_type != s21_NORMAL_VALUE) {
        if ((left.value_type == s21_INFINITY && right.value_type == s21_NEGATIVE_INFINITY) ||
            (left.value_type == s21_NEGATIVE_INFINITY && right.value_type == s21_INFINITY)) {
            is_not_equal = s21_true;
        }
    } else {
        is_not_equal = !s21_is_equal(left, right);
    }
    return is_not_equal;
}
