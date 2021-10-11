/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef TRAINGLE_HPP
#define TRAINGLE_HPP

#include "abstract_shape.hpp"

class Triangle : Abstract_shape {
public:
    Point_2d vertex[3];

    Triangle();
    Triangle(Point_2d a, Point_2d b, Point_2d c);

    Shapes get_shape_type() const override;
    unsigned int get_vertex_count() const override;
    Point_2d* get_vertex() override;
    double get_perimeter() override;
    double get_area() override;
};


#endif //TRAINGLE_HPP