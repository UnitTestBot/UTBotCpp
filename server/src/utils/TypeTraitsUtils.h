#ifndef UNITTESTBOT_TYPETRAITSUTIL_H
#define UNITTESTBOT_TYPETRAITSUTIL_H

#include <type_traits>

namespace Utils {
    template<typename ContainerT, typename T, typename = void>
    struct has_find : std::false_type {
    };

    template<typename ContainerT, typename T>
    struct has_find<
            ContainerT,
            T,
            std::void_t<decltype(std::declval<ContainerT>().find(std::declval<T>()))>
    > : std::true_type {
    };
} // namespace Utils

#endif //UNITTESTBOT_TYPETRAITSUTIL_H
