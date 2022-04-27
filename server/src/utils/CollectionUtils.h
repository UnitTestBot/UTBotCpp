/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_COLLECTIONUTILS_H
#define UNITTESTBOT_COLLECTIONUTILS_H

#include "HashUtils.h"
#include "TypeTraitsUtils.h"
#include "utils/path/FileSystemPath.h"
#include "exceptions/IncorrectIndexException.h"

#include <tsl/ordered_map.h>
#include <tsl/ordered_set.h>

#include <algorithm>
#include <list>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace CollectionUtils {
    using std::vector;

    template <typename V>
    using MapFileTo = std::unordered_map<fs::path, V, HashUtils::PathHash>;
    template <typename V>
    using OrderedMapFileTo = tsl::ordered_map<fs::path, V, HashUtils::PathHash>;
    using FileSet = std::unordered_set<fs::path, HashUtils::PathHash>;
    using OrderedFileSet = tsl::ordered_set<fs::path, HashUtils::PathHash>;

    /**
     * @brief Erase from collection all elements are equivalent to given one.
     * @param items collection from which to erase.
     * @param value value to be erased.
     */
    template <typename ContainerT, typename T>
    bool erase(ContainerT &items, T const &value) {
        auto it = std::remove(items.begin(), items.end(), value);
        if (it == items.end()) {
            return false;
        }
        items.erase(it, items.end());
        return true;
    }

    /**
     * @brief Erase all elements are satisfied given predicate.
     * @param items collection from which to erase.
     * @param pred predicate.
     * @returns number of erase elements
     */
    template <typename ContainerT, class Pr>
    size_t erase_if(ContainerT &items, Pr &&pred) {
        auto it = std::remove_if(items.begin(), items.end(), pred);
        size_t erased = std::distance(it, items.end());
        items.erase(it, items.end());
        return erased;
    }

    template <typename ContainerT, class Pr>
    size_t iterative_erase(ContainerT &items, Pr &&pred) {
        size_t erased = 0;
        for (auto it = items.begin(); it != items.end();) {
            if (pred(*it)) {
                items.erase(it++);
                erased++;
            } else {
                ++it;
            }
        }
        return erased;
    }

    template <class Pr, typename ...Args>
    size_t iterative_erase(tsl::ordered_map<Args...> &items, Pr &&pred) {
        tsl::ordered_map<Args...> filtered{items.bucket_count()};
        for (auto it = items.begin(); it != items.end(); it++) {
            if (!pred(it.template value())) {
                filtered.template insert(std::move(*it));
            }
        }
        return items = filtered;
    }

    template <class Pr, typename ...Args>
    size_t erase_if(std::unordered_map<Args...> &map, Pr &&pred) {
        return iterative_erase(map, std::forward<Pr>(pred));
    }
    template <class Pr, typename ...Args>
    size_t erase_if(std::unordered_set<Args...> &set, Pr &&pred) {
        return iterative_erase(set, std::forward<Pr>(pred));
    }
    template <class Pr, typename ...Args>
    size_t erase_if(tsl::ordered_map<Args...> &map, Pr &&pred) {
        size_t erased = 0;
        tsl::ordered_map<Args...> filtered{ map.bucket_count() };
        for (auto it = map.begin(); it != map.end(); it++) {
            if (!pred(it.template value())) {
                filtered.template insert(std::move(*it));
            } else {
                erased++;
            }
        }
        map = filtered;
        return erased;
    }
    template <class Pr, typename ...Args>
    size_t erase_if(tsl::ordered_set<Args...> &set, Pr &&pred) {
        size_t erased = 0;
        tsl::ordered_set<Args...> filtered{ set.bucket_count() };
        for (auto &&it : set) {
            if (!pred(it)) {
                filtered.insert(std::move(it));
            } else {
                erased++;
            }
        }
        set = filtered;
        return erased;
    }

    template <typename V, class Pr>
    size_t erase_if(std::list<V> &list, Pr &&pred) {
        return iterative_erase(list, std::forward<Pr>(pred));
    }

    template <typename ContainerT, class Pr, typename T = typename std::decay<ContainerT>::type>
    T filterOut(ContainerT &&items, Pr &&pr) {
        T result{ std::forward<ContainerT>(items) };
        erase_if(result, std::forward<Pr>(pr));
        return result;
    }

    template <typename ContainerTo, typename ContainerFrom, typename Functor>
    [[nodiscard]] ContainerTo transformTo(ContainerFrom const &items, Functor &&functor) {
        using std::cbegin;
        using std::cend;
        using std::end;

        ContainerTo result;
        result.reserve(items.size());
        std::transform(cbegin(items), cend(items), std::inserter(result, end(result)), functor);
        return result;
    }

    /**
     * @brief Transforms given collection to another by applying functor to each element.
     * @param items collection of type Collection with elements of type T.
     * @param functor wrapper of function to be applied for elements.
     * @return transformed collection of type Collection with elements of type U.
     */
    template <template <typename, typename...> class Container,
        typename Functor,
        typename T,
        typename U = std::invoke_result_t<Functor, T>,
        typename... Ts,
        typename... Us>
    [[nodiscard]] Container<U, Us...> transform(Container<T, Ts...> const &items, Functor &&functor) {
        return transformTo<Container<U, Us...>>(items, functor);
    }

    template <template <typename, typename...> class Container,
        typename Functor,
        typename T,
        typename... Ts
    >
    [[nodiscard]] std::vector<T, Ts...> filterToVector(Container<T, Ts...> const &items, Functor &&functor) {
        std::vector<T, Ts...> res;
        using std::begin;
        using std::end;
        std::copy_if(begin(items), end(items), std::back_inserter(res), functor);
        return res;
    }

    /**
     * @brief Provides all keys of given map.
     * @param map collection with keys of type K and values of type V.
     * @return vector of keys of type T.
     */
    template <template <typename, typename, typename...> class Map, typename K, typename V, typename T = K, typename... Ts>
    [[nodiscard]] vector<T> getKeys(const Map<K, V, Ts...> &map) {
        vector<T> keys;
        keys.reserve(map.size());
        for (const auto &[key, value] : map) {
            keys.push_back(key);
        }
        return keys;
    }

    /**
     * @brief Provides all values of given map.
     * @param map collection with keys of type K and values of type V.
     * @return vector of values of type V.
     */
    template <typename K, typename V, typename HashT>
    [[nodiscard]] vector<V> getValues(const std::unordered_map<K, V, HashT> &map) {
        vector<V> values;
        values.reserve(map.size());
        for (const auto &[key, value] : map) {
            values.push_back(value);
        }
        return values;
    }

    /**
     * @brief Remove all duplicated elements from vector.
     * @param vector with values of type T.
     * @return vector with unique values of type T.
     */
    template <typename T>
    [[nodiscard]] vector<T> removeDuplicates(const vector<T>& v) {
        vector<T> uniqueV = v;
        sort(uniqueV.begin(), uniqueV.end());
        uniqueV.erase(std::unique(uniqueV.begin(), uniqueV.end()), uniqueV.end());
        return uniqueV;
    }

    template <typename T, template <typename, typename, typename...> class Map, typename K, typename V, typename... Ts>
    [[nodiscard]] vector<T> getKeysAs(const Map<K, V, Ts...> &map) {
        return getKeys<Map, K, V, T, Ts...>(map);
    }

    /**
     * @brief Extends one collection to another.
     * @param to collection to be extended.
     * @param from collection with which to be extended.
     */
    template <typename To, class From>
    void extend(To &to, From &&from) {
        to.reserve(to.size() + from.size());
        to.insert(std::end(to), std::begin(from), std::end(from));
    }
    template <typename T, typename Hash, class From>
    void extend(std::unordered_set<T, Hash> &to, From &&from) {
        to.reserve(to.size() + from.size());
        to.insert(std::begin(from), std::end(from));
    }
    template <typename T, typename Hash, class From>
    void extend(tsl::ordered_set<T, Hash> &to, From &&from) {
        to.reserve(to.size() + from.size());
        to.insert(std::begin(from), std::end(from));
    }

    /**
     * @brief Union two sets.
     * @return The result of union operation.
     */
    template <class T, class Hash>
    std::unordered_set<T, Hash> unionSet(const std::unordered_set<T, Hash> &a,
                                         const std::unordered_set<T, Hash> &b) {
        std::unordered_set<T, Hash> result = a;
        for (const auto& el: b) {
            result.insert(el);
        }
        return result;
    }

    /**
     * @brief Check if element contains in collection.
     * @param items collection in which to be searched.
     * @param value element to be searched.
     * @return true if element contains in collection and false otherwise.
     */
    template <typename ContainerT, typename T>
    typename std::enable_if_t<Utils::has_find<ContainerT, T>::value, bool>
    contains(ContainerT const &items, T const &value) {
        return items.find(value) != items.end();
    }

    template <typename ContainerT, typename T>
    typename std::enable_if_t<!Utils::has_find<ContainerT, T>::value, bool>
    contains(ContainerT const &items, T const &value) {
        return std::find(std::cbegin(items), std::cend(items), value) != std::cend(items);
    }

    template <template <typename, typename, typename...> class Map, typename K, typename V, typename... Ts, typename T>
    bool containsKey(Map<K, V, Ts...> const &map, T const &key) {
        return map.find(key) != map.end();
    }

    template <typename ContainerT, typename T, typename S>
    bool replace(ContainerT &items, T const &from, S &&to) {
        auto it = std::find(std::begin(items), std::end(items), from);
        if (it != std::end(items)) {
            *it = std::forward<S>(to);
            return true;
        }
        return false;
    }

    template <typename Container>
    void unique(Container &items, bool isSorted = false) {
        if (!isSorted) {
            std::sort(items.begin(), items.end());
        }
        items.erase(unique(items.begin(), items.end()), items.end());
    }

    std::vector<int> range(int start, int end, int step);

    template <template <typename, typename, typename...> class Map, typename K, typename V, typename... Ts>
    std::optional<std::reference_wrapper<const V>> getOptionalValue(Map<K, V, Ts...> const &map, K const &key) {
        auto it = map.find(key);
        if (it == map.end()) {
            return std::nullopt;
        } else {
            return std::make_optional(std::cref<V>(it->second));
        }
    }

    template <template <typename, typename, typename...> class Map, typename K, typename V, typename... Ts>
    V getOrDefault(Map<K, V, Ts...> const &map, K const &key, V const &defaultValue) {
        auto it = map.find(key);
        if (it == map.end()) {
            return defaultValue;
        } else {
            return it->second;
        }
    }

    template <template <typename, typename, typename...> class Map, typename K, typename V, typename... Ts>
    std::optional<std::reference_wrapper<V>> getOptionalValue(Map<K, V, Ts...> &map, K const &key) {
        auto it = map.find(key);
        if (it == map.end()) {
            return std::nullopt;
        } else {
            return std::make_optional(std::ref<V>(it->second));
        }
    }

    template <typename K, typename V, typename... Ts>
    std::optional<std::reference_wrapper<V>> getOptionalValue(tsl::ordered_map<K, V, Ts...> &map, K const &key) {
        auto it = map.find(key);
        if (it == map.end()) {
            return std::nullopt;
        } else {
            return std::make_optional(std::ref<V>(it.template value()));
        }
    }

    template <typename Container>
    bool anyTrue(Container const& container) {
        return std::any_of(container.begin(), container.end(), [](bool x) { return x; });
    }
}

#endif //UNITTESTBOT_COLLECTIONUTILS_H
