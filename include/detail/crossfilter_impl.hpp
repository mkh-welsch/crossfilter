/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef CROSSFILTER_IMPL_H_GUARD
#define CROSSFILTER_IMPL_H_GUARD
#include <vector>
#include <functional>
#include <iostream>
#include <limits>
#include <tuple>
#include <set>
//#include <boost/signals2.hpp>
#include "detail/signal_base.hpp"
//#include "nod/nod.hpp"
#include "detail/bitarray.hpp"

template <typename Value, typename Crossfilter, bool isIterable> struct Dimension;
template <typename Key, typename Reduce, typename Dimension, bool> struct Group;
template <typename T> struct CrossFilter;

template<typename T> struct CrossFilterImpl {

 public:
  template <typename Value, typename TT, bool isIterable> friend struct Dimension;
  template <typename Key, typename Reduce, typename Dimension, bool> friend struct GroupImpl;

  static constexpr std::size_t dimensionOffset = std::numeric_limits<std::size_t>::max();
  static constexpr int dimensionBitIndex = -1;

  using record_type_t = T;
  using this_type_t = CrossFilterImpl<T>;
  using value_type_t = T;

  using index_vec_t = std::vector<std::size_t>;
  using index_set_t = std::vector<std::size_t>;
  using data_iterator = typename std::vector<T>::const_iterator;

  std::vector<record_type_t> data;
  std::size_t dataSize = 0;
  BitArray filters;

  // template<typename F> using signal_type_t = typename boost::signals2::signal<F>;
  // using connection_type_t = boost::signals2::connection;
  template<typename F> using signal_type_t = MovableSignal<typename signals::signal<F>, signals::connection>;
  using connection_type_t = signals::connection;
  
  signal_type_t<void(const std::string&)> onChangeSignal;
  signal_type_t<void(data_iterator, data_iterator)> addSignal;
  signal_type_t<void(std::size_t)> postAddSignal;
  signal_type_t<void(const std::vector<T> &, const std::vector<std::size_t>&, std::size_t, std::size_t)> addGroupSignal;
  signal_type_t<void(std::size_t, int,
                               const index_set_t &, const index_set_t &, bool)>
      filterSignal;

  signal_type_t<void(const std::vector<int64_t> &)> removeSignal;
  signal_type_t<void()> disposeSignal;
  CrossFilterImpl() {}

  template<typename Iterator>
  CrossFilterImpl(Iterator begin, Iterator end):data(begin, end) {
    dataSize = data.size();
    filters.resize(dataSize);
  }
  std::vector<T> bottom(std::size_t low, std::size_t high, int64_t k,
                        int64_t offset, const index_vec_t &indexes, const index_vec_t &empty);

  std::vector<T> top(std::size_t low, std::size_t high, int64_t k,
                     int64_t offset, const index_vec_t &indexes, const index_vec_t &empty);

  connection_type_t connectFilterSlot(
      std::function<void(std::size_t, int,
                         const index_set_t &, const index_set_t &, bool)>
          slot);

  connection_type_t
  connectRemoveSlot(std::function<void(const std::vector<int64_t> &)> slot);

  connection_type_t
  connectAddSlot(std::function<void(data_iterator, data_iterator)> slot);
  connection_type_t
  connectPostAddSlot(std::function<void(std::size_t)> slot);


  connection_type_t
  connectAddSlot(std::function<void(const std::vector<value_type_t> &, const std::vector<std::size_t> &,
                                    std::size_t, std::size_t)>);
  connection_type_t
  connectDisposeSlot(std::function<void()> slot) {
    return  disposeSignal.connect(slot);
  }
  void emitFilterSignal(std::size_t filterOffset, int filterBitNum,
                        const index_set_t &added, const index_set_t &removed);




  template<typename C>
  CrossFilterImpl<T> & add(const C &newData);

  CrossFilterImpl<T> & add(const T & newData);

  std::size_t size() const;
  std::vector<T> all() const {
    return data;
  }

  std::vector<uint8_t> & maskForDimension(std::vector<uint8_t> & mask) const;

template<typename D>
std::vector<uint8_t> & maskForDimension(std::vector<uint8_t> & mask,
                                        const D & dim) const;

template<typename D, typename ...Ts>
std::vector<uint8_t> & maskForDimension(std::vector<uint8_t> & mask,
                                        const D & dim,
                                        Ts&... dimensions) const;

  template<typename ...Ts>
  std::vector<T> allFiltered(Ts&... dimension) const;

  template<typename ...Ts>
  bool isElementFiltered(std::size_t index, Ts&... dimension) const;


  template<typename G>
  auto dimension(G getter) -> Dimension<decltype(getter(std::declval<record_type_t>())), T, false>;

  template<typename V, typename G>
  auto iterableDimension(G getter) -> Dimension<V, T, true>;


  void flipBitForDimension(std::size_t index, std::size_t dimensionOffset,
                           int dimensionBitIndex);

  void setBitForDimension(std::size_t index, std::size_t dimensionOffset,
                          int dimensionBitIndex);
  void resetBitForDimension(std::size_t index, std::size_t dimensionOffset,
                          int dimensionBitIndex);

  bool getBitForDimension(std::size_t index, std::size_t dimensionOffset,
                          int dimensionBitIndex);

  std::tuple<data_iterator, data_iterator>
  getDataIteratorsForIndexes(std::size_t low, std::size_t high);

  void removeData(std::function<bool(int)> shouldRemove);

  // removes all records matching the predicate (ignoring filters).
  void remove(std::function<bool(const T&, int)> predicate);

  // Removes all records that match the current filters
  void remove();


  bool zeroExcept(std::size_t index);

  bool  onlyExcept(std::size_t index, std::size_t offset, int bitIndex);

  bool zero(std::size_t index) {
    return filters.zero(index);
  }

  bool only(std::size_t index, std::size_t offset, std::size_t bitIndex) {
    return filters.only(index, offset, bitIndex);
  }

  record_type_t & getRaw(std::size_t index) { return data[index]; }

  std::size_t translateIndex(std::size_t index) const {
    return index;
  }


  template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
  auto  groupAll(
      AddFunc  add_func_,
      RemoveFunc remove_func_,
      InitialFunc initial_func_) -> Group<std::size_t,
                                          decltype(initial_func_()), CrossFilter<T>, true>;


  connection_type_t onChange(std::function<void(const std::string &)> callback);

  void triggerOnChange(const std::string & eventName) {
    onChangeSignal(eventName);
  }
  static constexpr  bool getIsIterable() {
    return false;
  }

  void dump() {
    std::cout << "connectionNum, " << addSignal.num_slots() << std::endl;
  }
};
#include "impl/crossfilter_impl.ipp"


#endif
