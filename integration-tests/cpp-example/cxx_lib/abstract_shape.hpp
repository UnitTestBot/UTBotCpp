#ifndef ABSTRACT_SHAPE_HPP
#define ABSTRACT_SHAPE_HPP

#include "point_2d.hpp"

class Abstract_shape {
public:
    enum Shapes {
        Triangle
    };

    virtual Shapes get_shape_type() const = 0;
    virtual unsigned int get_vertex_count() const = 0;
    virtual Point_2d* get_vertex() = 0;
    virtual double get_perimeter() = 0;
    virtual double get_area() = 0;
};


#endif //ABSTRACT_SHAPE_HPP
