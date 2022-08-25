#ifndef UNITTESTBOT_SIMPLE_CLASS_H
#define UNITTESTBOT_SIMPLE_CLASS_H

class Point_2d {
  int x;
public:
  int y;

  Point_2d();
  Point_2d(const int& x, const int& y);

  unsigned int get_dim() const;
  int get_x() const;
  int get_y() const;

  void abs();

  Point_2d operator+=(const Point_2d& rhs);


  unsigned&& on_one_line(Point_2d&& point);

  friend Point_2d operator+(const Point_2d& lhs, const Point_2d& rhs);
  friend void set_to_zero(Point_2d& point);
  friend void set_abs_by_ref(Point_2d& point);
};

Point_2d operator+(const Point_2d& lhs, const Point_2d& rhs);

void set_to_zero(Point_2d& point);

void set_abs_by_ref(Point_2d& point);

#endif //UNITTESTBOT_SIMPLE_CLASS_H
