/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */


#include "simple_class.h"


Point_2d::Point_2d() : x(0), y(0) {
}

Point_2d::Point_2d(const int& x, const int& y) : x(x), y(y) {
}

unsigned int Point_2d::get_dim() const {
    return 2;
}

int Point_2d::get_x() const {
    return x;
}

int Point_2d::get_y() const {
    return y;
}

Point_2d Point_2d::operator+=(const Point_2d& rhs) {
    this->x += rhs.x;
    this->y += rhs.y;
    return *this;
}

Point_2d operator+(const Point_2d& lhs, const Point_2d& rhs) {
    return Point_2d(lhs.x + rhs.x, lhs.y + rhs.y);
}

void set_to_zero(Point_2d& point) {
    if (point.x < 0) {
        point.x = 0;
    }
    if (point.x > 0) {
        point.x = 0;
    }
    if (point.y < 0) {
        point.y = 0;
    }
    if (point.y > 0) {
        point.y = 0;
    }
}

void set_abs_by_ref(Point_2d& point) {
    if (point.x < 0) {
        point.x = -1 * point.x;
    }
    if (point.y < 0) {
        point.y *= -1;
    }
}

void Point_2d::abs() {
    if (x < 0) {
        x = -1 * x;
    }
    if (y < 0) {
        y *= -1;
    }
}