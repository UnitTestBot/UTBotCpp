#include "triangle.h"
#include "point_2d.hpp"
#include <cmath>


Triangle::Triangle() {
    for(int i = 0; i < 3; i++) {
        vertex[i] = {0, 0};
    }
}

Triangle::Triangle(Point_2d a, Point_2d b, Point_2d c) {
    vertex[0] = a;
    vertex[1] = b;
    vertex[2] = c;
}

Abstract_shape::Shapes Triangle::get_shape_type() const {
    return Abstract_shape::Triangle;
}

unsigned int Triangle::get_vertex_count() const {
    return 3;
}

Point_2d* Triangle::get_vertex() {
    return vertex;
}

double Triangle::get_perimeter() {
    return get_dist(vertex[0], vertex[1]) + get_dist(vertex[1], vertex[2]) + get_dist(vertex[2], vertex[0]);
}

double Triangle::get_area() {
    double side1 = get_dist(vertex[0], vertex[1]);
    double side2 = get_dist(vertex[1], vertex[2]);
    double side3 = get_dist(vertex[2], vertex[0]);
    auto p = this->get_perimeter();
    return sqrt(p * (p - side1) * (p - side2) * (p - side3));
}
