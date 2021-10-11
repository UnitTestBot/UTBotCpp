/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_TYPEUTILS_H
#define UNITTESTBOT_TYPEUTILS_H

#include <typeinfo>

//NOTE: You can't include that header in files which compile with option -fno-rtti.
//      Please avoid including that in other headers.

namespace TypeUtils {

    /**
    * The function checks whether argument has decayed type T or not
    */
    template <typename T, typename U>
    static bool isSameType(const U &t) {
        return typeid(T) == typeid(t);
    }


    /**
    * The function checks whether argument's decayed type is derived from T
    */
    template <typename T, typename U>
    static bool isDerivedFrom(const U &t) {
        return dynamic_cast<T const *>(&t) != nullptr;
    }
}


#endif //UNITTESTBOT_TYPEUTILS_H
