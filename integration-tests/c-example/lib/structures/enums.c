#include "enums.h"
#include <stdarg.h>
#include <stdio.h>

int enumSignToInt(enum Sign s) {
    if (s == ZERO) {
      return 0;
    }
    if (s == NEGATIVE) {
        return -1;
    } else {
        return 1;
    } 
}

enum Sign intToSign(int a) {
    if (a == 0) {
        return ZERO;
    }

    if (a > 0) {
        return POSITIVE;
    }
    return NEGATIVE;
}

int structWithSignToInt(struct EnumStruct st) {
    return enumSignToInt(st.s);
}

int sumSignArray(struct EnumArrayWrapper enWrapper) {
    int res = 0;
    for (int i = 0; i < 5; i++) {
        res += enumSignToInt(enWrapper.signs[i]);
    }

    return res;
}

int enumSignPointerToInt(enum Sign *s) {
    return enumSignToInt(*s);
}

enum Sign* intToSignPointer(int a) {
    static enum Sign s;
    s = intToSign(a);
    return &s;
}

int getSignValue(enum Sign s) {
    switch (s) {
        case NEGATIVE:
            return 0;
        case ZERO:
            return 1;
        case POSITIVE:
            return 2;
        default:
            return -1;
    }
}

struct EnumWithinRecord {
  enum { OPEN, FF_FOUND, ON_HOLD, CLOSED } e;
};

static int enumWithinRecord(struct EnumWithinRecord record) {
  switch (record.e) {
  case OPEN:
    return 1;
  case CLOSED:
    return -1;
  default:
    return 0;
  }
}
