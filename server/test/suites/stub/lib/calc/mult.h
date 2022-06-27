#ifndef MODULE_PROJECT_MULT_H
#define MODULE_PROJECT_MULT_H

struct Vector {
    int x, y;
};

int dot(struct Vector a, struct Vector b);

typedef struct Vector Vector2;

Vector2 vec2_sum(Vector2 a, Vector2 b);

int mult(int a, int b);

#endif //MODULE_PROJECT_MULT_H
