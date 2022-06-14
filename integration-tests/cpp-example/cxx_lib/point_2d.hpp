#ifndef POINT_2D_HPP
#define POINT_2D_HPP

class Point_2d {
public:
  int x;
  int y;

  Point_2d();
  Point_2d(const int& x, const int& y);

  unsigned int get_dim() const;
  int get_x() const;
  int get_y() const;

  Point_2d operator+=(const Point_2d& rhs);
  Point_2d operator-=(const Point_2d& rhs);

  friend Point_2d operator+(const Point_2d& lhs, const Point_2d& rhs);
  friend Point_2d operator-(Point_2d lhs, const Point_2d& rhs);

  double get_dist_to_zero() const;
  friend double get_dist(const Point_2d& lhs, const Point_2d& rhs);
  friend void set_to_zero(Point_2d& point);
  friend void set_abs_by_ref(Point_2d& point);
};

Point_2d operator+(const Point_2d& lhs, const Point_2d& rhs);
Point_2d operator-(Point_2d lhs, const Point_2d& rhs);

double get_dist(const Point_2d& lhs, const Point_2d& rhs);

void set_to_zero(Point_2d& point);

void set_abs_by_ref(Point_2d& point);


#endif //POINT_2D_HPP
