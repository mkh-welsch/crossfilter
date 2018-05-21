/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef TYPE_TRAITS_H_GUARD
#define  TYPE_TRAITS_H_GUARD
#include <type_traits>
#include <iterator>
template<typename T>
struct extract_container_value {
  using type = typename std::decay<decltype(*std::begin(std::declval<T>()))>::type;
};

template<typename T>
using extract_container_value_t = typename extract_container_value<T>::type;


namespace detail {
struct getter_type {};
}
#endif
