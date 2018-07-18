/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef CROSSFILTER_DETAIL_H_GUARD
#define CROSSFILTER_DETAIL_H_GUARD
#include <functional>
#include <vector>
#include <type_traits>
#include "detail/crossfilter_impl.hpp"
namespace cross {

template<typename T, typename F>
struct extract_value {
  using type = typename std::decay<decltype(*std::begin(std::declval<F>()(
      std::declval<T>())))>::type;
};
template<typename T>  struct trivial_hash {
  std::size_t operator()(const T &) const {
    return std::size_t();
  }
};
template <typename, typename, bool> struct dimension;
template <typename, typename, typename, typename, bool, bool> struct feature;

template <typename T, typename Hash = trivial_hash<T>> struct filter: private impl::filter_impl<T,Hash> {
 public:
  using this_type_t = filter<T>;
  using impl_type_t = impl::filter_impl<T,Hash>;
  using record_type_t = typename impl::filter_impl<T,Hash>::record_type_t;
  using value_type_t = typename impl::filter_impl<T,Hash>::value_type_t;
  template<typename U> using dimension_t = dimension<U, T, false>;
  template<typename U> using iterable_dimension_t = dimension<U, T, true>;
  using data_iterator = typename impl::filter_impl<T,Hash>::data_iterator;
  using base_type_t = impl::filter_impl<T,Hash>;
  using connection_type_t = typename impl::filter_impl<T,Hash>::connection_type_t;
  template<typename F> using signal_type_t = typename impl::filter_impl<T,Hash>::template signal_type_t<F>;
  
  static constexpr bool get_is_iterable() {
    return false;
  }

  /**
     Create crossfilter with empty data
   */
  filter() {}

  /**
     Create crossfilter and add data from 'data'
     \param[in]  data Container of elements with type T, must support std::begin/end concept
   */
  template<typename C>
  explicit filter(const C & data)
      :impl_type_t(std::begin(data), std::end(data)) {}



  /**
     Add all data from container to crossfilter
     \param[in] new_data Container of elements with type T, must support std::begin/end concept
   */
  template<typename C>
  typename std::enable_if<!std::is_same<C, T>::value, filter&>::type
  add(const C &new_data, bool allow_duplicates = true);

  /**
     Add one element of type T to crossfilter
     \param[in] new_data - element to be added to crossfilter
   */
  filter &add(const T &new_data, bool allow_duplicates = true);

  /**
     Returns the number of records in the crossfilter, independent of any filters
   */
  std::size_t size() const;

  /**
     Returns all of the raw records in the crossfilter, independent of any filters
   */
  std::vector<T> all() const {
    return impl_type_t::all();
  }

  /**
     Returns all of the raw records in the crossfilter, with filters applied.
     Can optionally ignore filters that are defined on  dimension list [dimensions]
   */
  template<typename ...Ts>
  std::vector<T> all_filtered(Ts&... dimensions) const {
    return impl_type_t::all_filtered(dimensions...);
  }

  /**
     Check if the data record at index i is excluded by the current filter set.
     Can optionally ignore filters that are defined on  dimension list [dimensions]
   */
  template<typename ...Ts>
  bool is_element_filtered(std::size_t index, Ts&... dimensions) const {
    return impl_type_t::is_element_filtered(index,dimensions...);
  }

  /**
     Constructs a new dimension using the specified value accessor function.
   */
  template<typename F>
  auto dimension(F getter) -> cross::dimension<decltype(getter(std::declval<record_type_t>())), T, false> {
    return impl_type_t::dimension(getter);
  }

  /**
     Constructs a new dimension using the specified value accessor function.
     Getter function must return container of elements.
  */
  template<typename F>
  auto iterable_dimension(F getter) ->cross::dimension<decltype(getter(std::declval<record_type_t>())), T, true> {
    using value_type = decltype(getter(record_type_t()));
    return impl_type_t::template iterable_dimension<value_type>(getter);
  }


  /**
     removes all records matching the predicate (ignoring filters).
  */
  void remove(std::function<bool(const T&, int)> predicate);

  /**
     Removes all records that match the current filters
  */
  void remove();

  /**
     A convenience function for grouping all records and reducing to a single value,
     with given reduce functions.
   */
  template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
  auto
  feature(
      AddFunc add_func_,
      RemoveFunc remove_func_,
      InitialFunc initial_func_) -> feature<std::size_t,
                                            decltype(initial_func_()), T, value_type_t, true, false>;

  /**
     A convenience function for grouping all records and reducing to a single value,
     with predefined reduce functions to count records
  */

  auto feature_count() ->  cross::feature<std::size_t, std::size_t, T, value_type_t, true, false>;

  /**
     A convenience function for grouping all records and reducing to a single value,
     with predefined reduce functions to sum records
  */
  template<typename G>
  auto feature_sum(G value) -> cross::feature<std::size_t, decltype(value(record_type_t())), T, value_type_t,true, false>;

  // /**
  //    Equivalent ot groupAllReduceCount()
  // */
  // cross::feature<std::size_t, std::size_t, this_type_t, true> feature();

  /**
     Calls callback when certain changes happen on the Crossfilter. Crossfilter will pass the event type to callback as one of the following strings:
     * dataAdded
     * dataRemoved
     * filtered
   */
  connection_type_t onChange(std::function<void(const std::string &)> callback) {
    return impl_type_t::on_change(callback);
  }

};
}
#include "impl/crossfilter.ipp"


#endif
