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
template<typename T, typename F>
struct extract_value {
  using type = typename std::decay<decltype(*std::begin(std::declval<F>()(
      std::declval<T>())))>::type;
};

template <typename Value, typename Crossfilter, bool isIterable> struct Dimension;
template <typename Key, typename Reduce, typename Dimension, bool> struct Group;

template <typename T> struct CrossFilter: private CrossFilterImpl<T> {
 public:
  using this_type_t = CrossFilter<T>;
  using record_type_t = typename CrossFilterImpl<T>::record_type_t;
  using value_type_t = typename CrossFilterImpl<T>::value_type_t;
  template<typename U> using dimension_t = Dimension<U, T, false>;
  template<typename U> using iterable_dimension_t = Dimension<U, T, true>;
  using data_iterator = typename CrossFilterImpl<T>::data_iterator;
  using base_type_t = CrossFilterImpl<T>;
  using connection_type_t = typename CrossFilterImpl<T>::connection_type_t;
  template<typename F> using signal_type_t = typename CrossFilterImpl<T>::template signal_type_t<F>;
  
  static constexpr bool getIsIterable() {
    return false;
  }

  /**
     Create crossfilter with empty data
   */
  CrossFilter() {}

  /**
     Create crossfilter and add data from 'data'
     \param[in]  data Container of elements with type T, must support std::begin/end concept
   */
  template<typename C>
  explicit CrossFilter(const C & data)
      :CrossFilterImpl<T>(std::begin(data), std::end(data)) {}



  /**
     Add all data from container to crossfilter
     \param[in] newData Container of elements with type T, must support std::begin/end concept
   */
  template<typename C>
  typename std::enable_if<!std::is_same<C, T>::value, CrossFilter&>::type
  add(const C &newData);

  /**
     Add one element of type T to crossfilter
     \param[in] newData - element to be added to crossfilter
   */
  CrossFilter &add(const T &newData);

  /**
     Returns the number of records in the crossfilter, independent of any filters
   */
  std::size_t size() const;

  /**
     Returns all of the raw records in the crossfilter, independent of any filters
   */
  std::vector<T> all() const {
    return CrossFilterImpl<T>::all();
  }

  /**
     Returns all of the raw records in the crossfilter, with filters applied.
     Can optionally ignore filters that are defined on  dimension list [dimensions]
   */
  template<typename ...Ts>
  std::vector<T> allFiltered(Ts&... dimensions) const {
    return CrossFilterImpl<T>::allFiltered(dimensions...);
  }

  /**
     Check if the data record at index i is excluded by the current filter set.
     Can optionally ignore filters that are defined on  dimension list [dimensions]
   */
  template<typename ...Ts>
  bool isElementFiltered(std::size_t index, Ts&... dimensions) const {
    return CrossFilterImpl<T>::isElementFiltered(index,dimensions...);
  }

  /**
     Constructs a new dimension using the specified value accessor function.
   */
  template<typename F>
  auto dimension(F getter) ->Dimension<decltype(getter(std::declval<record_type_t>())), T, false> {
    return CrossFilterImpl<T>::dimension(getter);
  }

  /**
     Constructs a new dimension using the specified value accessor function.
     Getter function must return container of elements.
  */
  template<typename F>
  auto iterableDimension(F getter) ->Dimension<decltype(getter(std::declval<record_type_t>())), T, true> {
    using value_type = decltype(getter(record_type_t()));
    return CrossFilterImpl<T>::template iterableDimension<value_type>(getter);
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
  groupAll(
      AddFunc add_func_,
      RemoveFunc remove_func_,
      InitialFunc initial_func_) -> Group<std::size_t,
                                          decltype(initial_func_()), this_type_t, true>;

  /**
     A convenience function for grouping all records and reducing to a single value,
     with predefined reduce functions to count records
  */
  Group<std::size_t, std::size_t, this_type_t, true> groupAllReduceCount();

  /**
     A convenience function for grouping all records and reducing to a single value,
     with predefined reduce functions to sum records
  */
  template<typename G>
  auto groupAllReduceSum(G value) -> Group<std::size_t, decltype(value(record_type_t())), this_type_t, true>;

  /**
     Equivalent ot groupAllReduceCount()
  */
  Group<std::size_t, std::size_t, this_type_t, true> groupAll();

  /**
     Calls callback when certain changes happen on the Crossfilter. Crossfilter will pass the event type to callback as one of the following strings:
     * dataAdded
     * dataRemoved
     * filtered
   */
  connection_type_t onChange(std::function<void(const std::string &)> callback) {
    return CrossFilterImpl<T>::onChange(callback);
  }

};

#include "impl/crossfilter.ipp"


#endif
