#define typeid(x) _Generic((x), \
        _Bool: 0,               \
         char: 1,               \
          int: 2,               \
        float: 3,               \
      default: 4)

int get_typeid(unsigned short int casen) {
    switch (casen) {
        case 0:
            return typeid((_Bool const) 0);
        case 1:
            return typeid((char) 'c');
        case 2:
            return typeid(24);
        case 3:
            return typeid(42.f);
        default:
            return typeid("char const *");
    }
}
