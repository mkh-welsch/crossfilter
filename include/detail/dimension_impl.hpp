/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef DIMENSION_IMPL_H_GUARD
#define DIMENSION_IMPL_H_GUARD
#include <vector>
#include <functional>
#include <utility>
#include <tuple>
//#include <boost/signals2.hpp>
#include "detail/type_traits.hpp"
#include "detail/crossfilter_impl.hpp"

namespace trait {
template <typename, bool> struct cond_type;
template<typename T> struct cond_type<T, true> { using type = extract_container_value_t<T>;};
template<typename T> struct cond_type<T, false> { using type = T;};
}

template <typename Key, typename Reduce, typename Dimension, bool> struct Group;
template<typename Value, bool isIterable>
struct val {
  struct value_type1 : std::enable_if<isIterable, typename extract_container_value_t<Value>::type> {};
  struct value_type2 : std::enable_if<!isIterable, Value> {};
};

template<typename T> struct CrossFilterImpl;

template <typename V, typename T, bool isIterable> struct DimensionImpl {
  using value_type_t = typename trait::cond_type<V, isIterable>::type;
  using parent_type_t = typename CrossFilterImpl<T>::this_type_t;
  using field_type_t = V;
  using record_type_t = typename CrossFilterImpl<T>::record_type_t;
  using this_type_t = Dimension<V, T, isIterable>;
  using data_iterator = typename CrossFilterImpl<T>::data_iterator;

  template<typename F> using signal_type_t = typename CrossFilterImpl<T>::template signal_type_t<F>;
  using connection_type_t = typename CrossFilterImpl<T>::connection_type_t;
  
  template <typename Key, typename Reduce, typename Dimension, bool>
  friend struct Group;

  std::vector<value_type_t> values;
  std::vector<std::size_t> indexes;
  std::vector<value_type_t> newValues;
  std::vector<std::size_t> newIndexes;
  std::vector<std::size_t> added;
  std::vector<std::size_t> removed;
  std::size_t oldDataSize = 0;

  CrossFilterImpl<T> * crossfilter = nullptr;
  std::size_t dimensionOffset;
  int dimensionBitIndex;

  std::size_t low = 0;
  std::size_t high = 0;

  // iterable stuff
  std::vector<std::size_t> iterablesIndexCount;
  std::vector<std::size_t> iterablesEmptyRows;


  std::function<std::tuple<std::size_t, std::size_t>(
      const std::vector<value_type_t> &values)>
      refilter;

  std::function<field_type_t(const record_type_t &)> getter;

  std::function<bool(value_type_t)> refilterFunction;

  bool refilterFunctionFlag = false;

  std::function<void(std::size_t)> slotPostAdd;
  connection_type_t connectionPostAdd;

  std::function<void(data_iterator, data_iterator)> slotAdd;
  connection_type_t connectionAdd;

  std::function<void(const std::vector<int64_t> &)> slotRemove;
  connection_type_t connectionRemove;

  signal_type_t<void(const std::vector<int64_t> &)> removeSignal;
  signal_type_t<void()> disposeDimensionSignal;
  signal_type_t<void(const std::vector<value_type_t> &,
                               const std::vector<std::size_t> &, std::size_t, std::size_t)>  addSignal;

  template<bool Enable = true>
  std::enable_if_t<isIterable && Enable> add(data_iterator begin, data_iterator end);

  template<bool Enable = true>
  std::enable_if_t<!isIterable && Enable> add(data_iterator begin, data_iterator end);

  //  void  addIterable(data_iterator begin, data_iterator end);

  bool zeroExcept(std::size_t index);

  bool onlyExcept(std::size_t index, std::size_t offset, int bitIndex);

  record_type_t &get(std::size_t index);

  record_type_t &getRaw(std::size_t index);

  connection_type_t connectFilterSlot(
      std::function<void(std::size_t filterOffset, int filterBitNum,
                         const std::vector<std::size_t> &,
                         const std::vector<std::size_t> &, bool)> slot);

  connection_type_t
  connectAddSlot(std::function<void(const std::vector<value_type_t> &,
                                    const std::vector<std::size_t> &, std::size_t, std::size_t)> slot);

  connection_type_t
  connectRemoveSlot(std::function<void(const std::vector<int64_t> &)> slot);

  connection_type_t
  connectDisposeSlot(std::function<void()> slot);

  void removeData(const std::vector<int64_t> &reIndex);

  void removeDataIterable(const std::vector<int64_t> &reIndex);

  std::tuple<std::size_t, std::size_t>
  refilterRange(const value_type_t & left, const value_type_t &right,
                const std::vector<value_type_t> &data);

  std::tuple<std::size_t, std::size_t>
  refilterExact(const value_type_t &value, const std::vector<value_type_t> &data);

  template<bool Enable = true>
  std::enable_if_t<isIterable && Enable>
  doFilter(std::size_t filterLowIndex, std::size_t filterHighIndex);
  template<bool Enable = true>
  std::enable_if_t<!isIterable && Enable>
  doFilter(std::size_t filterLowIndex, std::size_t filterHighIndex);

 public:
  DimensionImpl() {}

  DimensionImpl(CrossFilterImpl<T> *cf, std::tuple<std::size_t, int> filterPos_,
            std::function<field_type_t(const record_type_t &)> getter_);

  DimensionImpl(DimensionImpl<V, T, isIterable> && dim)
      : values(dim.values), indexes(dim.indexes), crossfilter(dim.crossfilter),
        dimensionOffset(dim.dimensionOffset), dimensionBitIndex(dim.dimensionBitIndex),
        low(dim.low), high(dim.high), iterablesIndexCount(dim.iterablesIndexCount),
        iterablesEmptyRows(dim.iterablesEmptyRows), refilter(dim.refilter),
        getter(dim.getter), refilterFunctionFlag(dim.refilterFunctionFlag),
        removeSignal(std::move(dim.removeSignal)),
        disposeDimensionSignal(std::move(dim.disposeDimensionSignal)),
        addSignal(std::move(dim.addSignal))  {
    slotAdd =  [this](data_iterator begin, data_iterator end) {
      this->add(begin, end); };

    slotRemove = [this] (const std::vector<int64_t> &reIndex) {
      this->removeData(reIndex);
    };

    slotPostAdd = [this](std::size_t newDataSize) {
      addSignal(newValues, newIndexes, oldDataSize, newDataSize);
      newValues.clear();
    };

    dim.connectionAdd.disconnect();
    connectionAdd = crossfilter->connectAddSlot(slotAdd);
    dim.connectionRemove.disconnect();
    connectionRemove = crossfilter->connectRemoveSlot(slotRemove);
  }

  DimensionImpl & operator = (DimensionImpl<V, T, isIterable> && dim) {
    values = std::move(dim.values);
    indexes = std::move(dim.indexes);
    crossfilter = std::move(dim.crossfilter);
    dimensionOffset = dim.dimensionOffset;
    dimensionBitIndex = dim.dimensionBitIndex;
    low = dim.low;
    high = dim.high;
    iterablesIndexCount = std::move(dim.iterablesIndexCount);
    iterablesEmptyRows = std::move(dim.iterablesEmptyRows);
    refilter = std::move(dim.refilter);
    getter = std::move(dim.getter);
    refilterFunctionFlag = dim.refilterFunctionFlag;
    removeSignal = std::move(dim.removeSignal);
    disposeDimensionSignal = std::move(dim.disposeDimensionSignal);
    addSignal = std::move(dim.addSignal);
    slotAdd =  [this](data_iterator begin, data_iterator end) {
      this->add(begin, end); };

    slotRemove = [this] (const std::vector<int64_t> &reIndex) {
      this->removeData(reIndex);
    };
    slotPostAdd = [this](std::size_t newDataSize) {
      addSignal(newValues, newIndexes, oldDataSize, newDataSize);
      newValues.clear();
    };


    connectionAdd.disconnect();
    dim.connectionAdd.disconnect();
    connectionAdd = crossfilter->connectAddSlot(slotAdd);

    dim.connectionRemove.disconnect();
    connectionRemove.disconnect();
    connectionRemove = crossfilter->connectRemoveSlot(slotRemove);
    dim.connectionPostAdd.disconnect();
    connectionPostAdd.disconnect();
    connectionPostAdd = crossfilter->connectPostAddSlot(slotPostAdd);

    return *this;
  }

  void dispose();

  void filterRange(const value_type_t &left, const value_type_t &right);

  void filterExact(const value_type_t &value);

  void filterAll();

  void filterWithPredicate(std::function<bool(const value_type_t &)> filterFunction);

  std::vector<record_type_t> bottom(int64_t k,
                                    int64_t bottom_offset = 0);

  std::vector<record_type_t> top(int64_t k, int64_t top_offset = 0);

  template <typename K, typename R>
  auto
  group(std::function<K(const value_type_t &)> key,
        std::function<R(R &, const record_type_t &, bool)> add_func_,
        std::function<R(R &, const record_type_t &, bool)> remove_func_,
        std::function<R()> initial_func_) ->   Group<K, R, this_type_t, false>;

  template <typename R>
  Group<std::size_t, R, this_type_t, true> groupAll(
      std::function<R(R &, const record_type_t &, bool)> add_func_,
      std::function<R(R &, const record_type_t &, bool)> remove_func_,
      std::function<R()> initial_func_);

  std::size_t translateIndex(std::size_t index) const {
      return indexes[index];
  }
  static constexpr  bool getIsIterable() {
    return isIterable;
  }
  void filter1(std::vector<std::size_t> & added, std::vector<std::size_t>& removed, std::size_t filterLowIndex, std::size_t filterHighIndex);
  void filter2(std::vector<std::size_t> & added, std::vector<std::size_t>& removed, std::size_t filterLowIndex, std::size_t filterHighIndex);
};

#include "impl/dimension_impl.ipp"

#endif
