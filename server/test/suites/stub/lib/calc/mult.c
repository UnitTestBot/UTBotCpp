#include "mult.h"

int dot(struct Vector a, struct Vector b) {
    return a.x * b.x + a.y * b.y;
}

Vector2 vec2_sum(Vector2 a, Vector2 b) {
    Vector2 result = {a.x + b.x, a.y + b.y};
    return result;
}

int mult(int a, int b) {
    return a * b;
}
