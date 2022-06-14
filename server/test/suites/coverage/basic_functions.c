#include "basic_functions.h"

int max_(int a, int b) {
  if (a > b) {
    return a;
  }
  else {
    return b;
  }
}

int min(int a, int b) {
  if (a < b) {
    return a;
  }
  else {
    return b;
  }
}

int sqr_positive(int a) {
  if (a < 0) {
    return -1;
  }
  else {
    return a * a;
  }
}
