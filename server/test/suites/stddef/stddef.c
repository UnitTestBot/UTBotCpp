#include <stddef.h>

_Bool iswprint(wchar_t* wc) {
    return *wc >= ' ' && *wc <= '~';
}