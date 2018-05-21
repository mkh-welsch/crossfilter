/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#include "detail/crossfilter_impl.hpp"
#include <numeric>
namespace {
static constexpr int REMOVED_INDEX = -1;
}

template<typename T>
inline std::vector<T> CrossFilterImpl<T>::bottom(std::size_t low, std::size_t high, int64_t k,
                                                 int64_t offset,
                                                 const std::vector<std::size_t> &indexes,
                                                 const std::vector<std::size_t> &empty) {
    std::vector<T> result;
    auto highIter = indexes.begin();
    auto lowIter = indexes.begin();
    std::advance(highIter, high);
    std::advance(lowIter, low);
    int64_t resultSize = 0;
    for(auto p = empty.begin(); p < empty.end() && resultSize < k; p++) {
      if(filters.zero(*p)) {
        if (offset-- > 0)
          continue;
        result.push_back(data[*p]);
        resultSize++;
      }
    }
    if (lowIter > highIter)
      return result;
    for (; lowIter < highIter && resultSize < k; lowIter++) {
      if (filters.zero(*lowIter)) {
        if (offset-- > 0)
          continue;
        result.push_back(data[*lowIter]);
        resultSize++;
      }
    }
    return result;
  }

template<typename T>
inline
std::vector<T> CrossFilterImpl<T>::top(std::size_t low, std::size_t high, int64_t k,
                                       int64_t offset,
                                       const std::vector<std::size_t> &indexes, const std::vector<std::size_t> &empty) {
    std::vector<T> result;
    auto highIter = indexes.begin();
    auto lowIter = indexes.begin();
    std::advance(highIter, high - 1);
    std::advance(lowIter, low);
    int64_t resultSize = 0;
    int64_t skip = offset;

    if  (highIter >= lowIter) {
      for (; highIter >= lowIter && resultSize < k ; highIter--) {
        if (filters.zero(*highIter)) {
          if (skip-- > 0)
            continue;
          result.push_back(data[*highIter]);
          resultSize++;
        }
      }
    }
    if(resultSize < k) {
      for(auto p = empty.begin(); p < empty.end() && resultSize < k; p++) {
        if(filters.zero(*p)) {
          if (skip-- > 0)
            continue;
          result.push_back(data[*p]);
          resultSize++;
        }
      }
    }
    return result;
  }

template<typename T>
inline

typename CrossFilterImpl<T>::connection_type_t CrossFilterImpl<T>:: onChange(std::function<void(const std::string &)> callback) {
  return onChangeSignal.connect(callback);
}
template<typename T>
inline
typename CrossFilterImpl<T>::connection_type_t CrossFilterImpl<T>::connectFilterSlot(std::function<void(std::size_t, int,
                                                              const std::vector<std::size_t> &,
                                                              const std::vector<std::size_t> &, bool)> slot) {
  return filterSignal.connect(slot);
}

template<typename T>
inline
typename CrossFilterImpl<T>::connection_type_t
CrossFilterImpl<T>::connectRemoveSlot(std::function<void(const std::vector<int64_t> &)> slot) {
    return removeSignal.connect(slot);
}

template<typename T>
inline
typename CrossFilterImpl<T>::connection_type_t
CrossFilterImpl<T>::connectAddSlot(std::function<void(data_iterator, data_iterator)> slot) {
  return addSignal.connect(slot);
}
template<typename T>
inline
typename CrossFilterImpl<T>::connection_type_t
CrossFilterImpl<T>::connectPostAddSlot(std::function<void(std::size_t)> slot) {
  return postAddSignal.connect(slot);
}

template<typename T>
inline
void CrossFilterImpl<T>::emitFilterSignal(std::size_t filterOffset, int filterBitNum,
                        const std::vector<std::size_t> & added,
                        const std::vector<std::size_t> & removed) {
  filterSignal(filterOffset, filterBitNum, added, removed, false);
}

template<typename T>
template<typename G>
inline
auto CrossFilterImpl<T>::dimension(G getter)
    -> Dimension<decltype(getter(std::declval<T>())), T, false> {
  auto dimensionFilter = filters.addRow();
  Dimension<decltype(getter(std::declval<record_type_t>())), T, false> dim(this, dimensionFilter, getter);
  return dim;
}

template<typename T>
template<typename V, typename G>
inline
auto CrossFilterImpl<T>::iterableDimension(G getter) -> Dimension<V, T, true> {
  auto dimensionFilter = filters.addRow();
  Dimension<V, T, true> dim(this, dimensionFilter, getter);
  return dim;
}

template<typename T>
inline
std::vector<uint8_t> & CrossFilterImpl<T>::maskForDimension(std::vector<uint8_t> & mask) const {
  return mask;
}

template<typename T>
template<typename D>
inline
std::vector<uint8_t> & CrossFilterImpl<T>::maskForDimension(std::vector<uint8_t> & mask,
                                                            const D & dim) const {
  std::bitset<8> b = mask[dim.getOffset()];
  b.set(dim.getBitIndex(), true);

  mask[dim.getOffset()] = b.to_ulong();
  return mask;
}

template<typename T>
template<typename D, typename ...Ts>
inline
std::vector<uint8_t> & CrossFilterImpl<T>::maskForDimension(std::vector<uint8_t> & mask,
                                                            const D & dim,
                                                            Ts&... dimensions) const {
  std::bitset<8> b = mask[dim.getOffset()];
  b.set(dim.getBitIndex(), true);
  mask[dim.getOffset()] = b.to_ulong();
  return maskForDimension(mask, dimensions...);
}

template<typename T>
template<typename ...Ts>
inline
std::vector<T> CrossFilterImpl<T>::allFiltered(Ts&... dimensions) const {
  std::vector<uint8_t> mask(filters.size());
  maskForDimension(mask, dimensions...);
  std::vector<T> result;

  for (std::size_t i = 0; i < data.size(); i++) {
    if (filters.zeroExceptMask(i, mask))
      result.push_back(data[i]);
  }
  return result;
}

template<typename T>
template<typename ...Ts>
inline
bool CrossFilterImpl<T>::isElementFiltered(std::size_t index, Ts&... dimensions) const {
  std::vector<uint8_t> mask(filters.size());
  maskForDimension(mask, dimensions...);
  return filters.zeroExceptMask(index, mask);
}

template<typename T>
template<typename C>
inline
CrossFilterImpl<T> & CrossFilterImpl<T>::add(const C &newData) {
  auto oldDataSize = data.size();
  auto begin = std::begin(newData);
  auto end = std::end(newData);

  auto newDataSize = std::distance(begin, end);

  if (newDataSize > 0) {
      data.insert(data.end(), begin, end);
      dataSize += newDataSize;
      filters.resize(dataSize);
      data_iterator nbegin = data.begin();
      std::advance(nbegin, oldDataSize);

      data_iterator nend = nbegin;
      std::advance(nend, newDataSize);

      addSignal(nbegin, nend);
      postAddSignal(newDataSize);

      if(addGroupSignal.num_slots() != 0) {
        // FIXME: remove temporary vector
        std::vector<record_type_t> tmp(begin, end);
        std::vector<std::size_t> indexes(tmp.size());
        std::iota(indexes.begin(),indexes.end(),oldDataSize);
        addGroupSignal(tmp, indexes, oldDataSize, newDataSize);
      }
      
      triggerOnChange("dataAdded");
    }
    return *this;
}

template<typename T>
inline
CrossFilterImpl<T> & CrossFilterImpl<T>::add(const T &newData) {
  auto oldDataSize = data.size();

  auto newDataSize = 1;
  data.push_back(newData);

  dataSize += newDataSize;
  filters.resize(dataSize);

  // FIXME: remove temporary vector
  std::vector<T> tmp;
  tmp.push_back(newData);

  addSignal(tmp.begin(), tmp.end());
  postAddSignal(newDataSize);

  if(addGroupSignal.num_slots() != 0) {
    std::vector<std::size_t> indexes(tmp.size());
    std::iota(indexes.begin(),indexes.end(),oldDataSize);
    addGroupSignal(tmp, indexes, oldDataSize, newDataSize);
  }
  triggerOnChange("dataAdded");
  return *this;
}

template<typename T>
inline
std::size_t CrossFilterImpl<T>::size() const { return data.size(); }

template<typename T>
inline
void CrossFilterImpl<T>::flipBitForDimension(std::size_t index, std::size_t dimensionOffset,
                           int dimensionBitIndex) {
    filters.flip(index, dimensionOffset, dimensionBitIndex);
  }
template<typename T>
inline
void CrossFilterImpl<T>::setBitForDimension(std::size_t index, std::size_t dimensionOffset,
                                        int dimensionBitIndex) {
  filters.set(index, dimensionOffset, dimensionBitIndex);
}
template<typename T>
inline
void CrossFilterImpl<T>::resetBitForDimension(std::size_t index, std::size_t dimensionOffset,
                                            int dimensionBitIndex) {
  filters.reset(index, dimensionOffset, dimensionBitIndex);
}

template<typename T>
inline
bool CrossFilterImpl<T>::getBitForDimension(std::size_t index, std::size_t dimensionOffset,
                                            int dimensionBitIndex) {
  return filters.check(index, dimensionOffset, dimensionBitIndex);
}

template<typename T>
inline
auto
CrossFilterImpl<T>::getDataIteratorsForIndexes(std::size_t low, std::size_t high)
    -> std::tuple<data_iterator, data_iterator> {
    auto begin = data.cbegin();
    auto end = data.cbegin();
    std::advance(begin, low);
    std::advance(end, high);
    if (end > data.cend())
      end = data.cend();

    return std::make_tuple<data_iterator, data_iterator>(std::move(begin),
                                                         std::move(end));
  }

template<typename T>
inline
void CrossFilterImpl<T>::removeData(std::function<bool(int)> shouldRemove) {
    std::vector<int64_t> newIndex(dataSize);

    std::vector<std::size_t> removed;

    for (std::size_t index1 = 0, index2 = 0; index1 < dataSize; ++index1) {
      if (shouldRemove(index1)) {
        removed.push_back(index1);
        newIndex[index1] = REMOVED_INDEX;
      } else {
        newIndex[index1] = index2++;
      }
    }

    // Remove all matching records from groups.
    filterSignal(std::numeric_limits<std::size_t>::max(), -1,
                 std::vector<std::size_t>(), removed, true);
    // Update indexes.
    removeSignal(newIndex);

    // Remove old filters and data by overwriting.
    std::size_t index4 = 0;

    for (std::size_t index3 = 0; index3 < dataSize; ++index3) {
      if (newIndex[index3] != REMOVED_INDEX) {
        if (index3 != index4) {
          filters.copy(index4, index3);
          data[index4] = data[index3];
        }
        ++index4;
      }
    }

    dataSize = index4;
    data.resize(dataSize);
    filters.truncate(index4);
    triggerOnChange("dataRemoved");
  }


  // removes all records matching the predicate (ignoring filters).
template<typename T>
inline
void CrossFilterImpl<T>::remove(std::function<bool(const T&, int)> predicate) {
  removeData([&](auto i) { return predicate(data[i], i); });
}

template<typename T>
inline
  // Removes all records that match the current filters
void CrossFilterImpl<T>::remove() {
  removeData([&](auto i) { return filters.zero(i); });
}

template<typename T>
inline
bool CrossFilterImpl<T>::zeroExcept(std::size_t index) {
  return filters.zero(index);
}

template<typename T>
inline
bool  CrossFilterImpl<T>::onlyExcept(std::size_t index, std::size_t offset, int bitIndex) {
  return filters.only(index, offset, bitIndex);
}

template<typename T>
template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
inline
auto  CrossFilterImpl<T>::groupAll(
    AddFunc  add_func_,
    RemoveFunc remove_func_,
    InitialFunc initial_func_) -> Group<std::size_t,
                                        decltype(initial_func_()), CrossFilter<T>, true> {
  using ReduceType = decltype(initial_func_());

  Group<std::size_t, ReduceType, CrossFilter<T>, true> g(this,
                                                             [](const value_type_t& ) { return std::size_t(0);},
                                                             add_func_,
                                                             remove_func_,
                                                             initial_func_);
  std::vector<std::size_t> indexes(data.size());
  std::iota(indexes.begin(),indexes.end(),0);
  g.add(data, indexes, 0,data.size());
  return g;
}

template<typename T>
inline
typename CrossFilterImpl<T>::connection_type_t
CrossFilterImpl<T>::connectAddSlot(std::function<void(const std::vector<value_type_t> &,
                                                      const std::vector<std::size_t> &,
                                                      std::size_t, std::size_t)> slot) {
  return addGroupSignal.connect(slot);
}



