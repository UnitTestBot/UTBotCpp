typedef enum {
    EVEN,
    ODD
} Parity;

struct WrapperStruct {
    Parity p;
};

struct PointerStruct {
    struct WrapperStruct* wrapperStruct;
};

int isCorrectPointerStruct(struct PointerStruct* s) {
    if (!s || !s->wrapperStruct) {
        return -1;
    }
    Parity parity = s->wrapperStruct->p;
    if (parity == EVEN) {
        return 0;
    }
    if (parity == ODD) {
        return 1;
    }
    return -2;
}