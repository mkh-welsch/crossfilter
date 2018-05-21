/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#include <numeric>
#include <unordered_map>
#include "crossfilter.hpp"
#ifndef USE_STD_SORT
#include "detail/impl/dual_pivot_sort2.hpp"
#endif

template<typename V, typename T, bool isIterable>
inline
DimensionImpl<V, T, isIterable>::DimensionImpl(CrossFilterImpl<T> *cf, std::tuple<std::size_t, int> filterPos_,
                                   std::function<field_type_t(const record_type_t &)> getter_)
    : crossfilter(cf), dimensionOffset(std::get<0>(filterPos_)),
      dimensionBitIndex(std::get<1>(filterPos_)), getter(getter_) {

  slotAdd =  [this](data_iterator begin, data_iterator end) {
    this->add(begin, end);
  };

  slotRemove = [this] (const std::vector<int64_t> &reIndex) {
    this->removeData(reIndex);
  };
  slotPostAdd = [this](std::size_t newDataSize) {
    addSignal(newValues, newIndexes, oldDataSize, newDataSize);
    newValues.clear();
  };


  connectionAdd = crossfilter->connectAddSlot(slotAdd);
  connectionRemove = crossfilter->connectRemoveSlot(slotRemove);
  connectionPostAdd = crossfilter->connectPostAddSlot(slotPostAdd);

  refilter = [](const std::vector<value_type_t> &val) {
    return std::make_tuple<std::size_t, std::size_t>(0, val.size());
  };
  add(crossfilter->data.begin(), crossfilter->data.end());
}

template<typename V, typename T, bool isIterable>
inline
void  DimensionImpl<V, T, isIterable>::dispose() {
  disposeDimensionSignal();
  connectionAdd.disconnect();
  connectionRemove.disconnect();
  filterAll();
}

template<typename V, typename T, bool isIterable>
inline
bool DimensionImpl<V, T, isIterable>::zeroExcept(std::size_t index) {
  return crossfilter->filters.zeroExcept(index,
                                         dimensionOffset,
                                         dimensionBitIndex);
}


template<typename V, typename T, bool isIterable>
inline
bool  DimensionImpl<V, T, isIterable>::onlyExcept(std::size_t index, std::size_t offset, int bitIndex) {

  return crossfilter->filters.onlyExcept(index,
                                         dimensionOffset, dimensionBitIndex, offset, bitIndex);
}

template<typename V, typename T, bool isIterable>
inline
auto
DimensionImpl<V, T, isIterable>::get(std::size_t index)
    -> record_type_t& {
  return crossfilter->data[indexes[index]];
}

template<typename V, typename T, bool isIterable>
inline
// typename DimensionImpl<V, T, isIterable>::record_type_t &
auto
DimensionImpl<V, T, isIterable>::getRaw(std::size_t index) -> record_type_t& {
  return crossfilter->data[index];
}

template<typename V, typename T, bool isIterable>
inline
typename DimensionImpl<V,T,isIterable>::connection_type_t
DimensionImpl<V, T, isIterable>::connectFilterSlot(std::function<void(std::size_t, int,
                                                                               const std::vector<std::size_t> &,
                                                                               const std::vector<std::size_t> &,
                                                                               bool)> slot) {
  return crossfilter->connectFilterSlot(slot);
}


template<typename V, typename T, bool isIterable>
inline
typename DimensionImpl<V,T,isIterable>::connection_type_t
DimensionImpl<V, T, isIterable>::connectAddSlot(std::function<void(const std::vector<value_type_t>&,
                                                                            const std::vector<std::size_t> &,
                                                                            std::size_t, std::size_t)> slot) {
  return addSignal.connect(slot);
}


template<typename V, typename T, bool isIterable>
inline
typename DimensionImpl<V,T,isIterable>::connection_type_t
DimensionImpl<V, T, isIterable>::connectRemoveSlot(std::function<void(const std::vector<int64_t> &)> slot) {
  return removeSignal.connect(slot);
}

template<typename V, typename T, bool isIterable>
inline
typename DimensionImpl<V,T,isIterable>::connection_type_t
DimensionImpl<V, T, isIterable>::connectDisposeSlot(std::function<void()> slot) {
  return disposeDimensionSignal.connect(slot);
}

template<typename V, typename T, bool isIterable>
template<bool Enable>
inline
std::enable_if_t<isIterable && Enable>
DimensionImpl<V, T, isIterable>::add(data_iterator begin, data_iterator end) {

  newValues.clear();
  newIndexes.clear();
  
  std::size_t newDataSize = std::distance(begin, end);
  oldDataSize = crossfilter->size() - newDataSize;
  std::vector<std::size_t> unsortedIndex;
  std::size_t l = 0;
  std::vector<std::size_t> newIterablesIndexCount(newDataSize);

  for (std::size_t index1 = 0; index1 < newDataSize; index1++) {
    auto k = getter(*(begin + index1));

    if (k.empty()) {
      newIterablesIndexCount[index1] = 0;
      iterablesEmptyRows.push_back(index1 + oldDataSize);
      continue;
    }
    auto ks = k.size();
    newIterablesIndexCount[index1] = ks;
    for (std::size_t j = 0; j < ks; ++j) {
      newValues.push_back(k[j]);
      //      unsortedIndex[l] = index1;
      unsortedIndex.push_back(index1 + oldDataSize);
      l++;
    }
  }
  std::vector<std::size_t> newIndex(newValues.size());


  std::iota(newIndex.begin(), newIndex.end(), 0);
#ifndef USE_STD_SORT
  DualPivotsort2::quicksort<std::size_t, value_type_t>(newIndex, 0, newIndex.size(),
                                                       [this](auto r) { return newValues[r];});
#else
  std::sort(newIndex.begin(), newIndex.end(),
            [this](auto lhs, auto rhs) {
              return newValues[lhs] < newValues[rhs];
            });
#endif
  std::vector<value_type_t> tmpValues;
  std::vector<std::size_t> tmpIndex;

  for (auto &i : newIndex) {
    tmpValues.push_back(newValues[i]);
    tmpIndex.push_back(unsortedIndex[i]);
    //    tmpIndex.push_back(i);
  }
  std::swap(tmpValues, newValues);
  std::swap(tmpIndex, newIndexes);


  auto bounds = refilter(newValues);
  auto lo1 = std::get<0>(bounds);
  auto hi1 = std::get<1>(bounds);

  std::unordered_map<std::size_t, std::size_t> newFilterStatus;
  if(refilterFunctionFlag) {
    for(std::size_t i = 0; i < newValues.size(); i++) {
      if(!refilterFunction(newValues[i]))
        newFilterStatus[newIndexes[i]] += 1;
    }
  } else {
    for (std::size_t i = 0; i < lo1; ++i) {
      newFilterStatus[newIndexes[i]] += 1;
    }
    auto nvs = newValues.size();
    
    for (std::size_t i = hi1; i < nvs; ++i) {
      newFilterStatus[newIndexes[i]] += 1;
    }
  }

  auto itics = iterablesIndexCount.size();
  
  for(auto & f : newFilterStatus) {
    // if all elements of iterable data field filtered out - set filter bit
    if(f.second == newIterablesIndexCount[f.first])
      crossfilter->setBitForDimension(f.first + itics, dimensionOffset, dimensionBitIndex);
  }
  newDataSize = (isIterable) ? l : newDataSize;

  if (values.empty()) {
    values = newValues;
    indexes = newIndexes;
    std::swap(iterablesIndexCount, newIterablesIndexCount);
    low = lo1;
    high = hi1;
    return;
  }

  std::vector<value_type_t> oldValues;
  std::swap(values, oldValues);
  std::vector<std::size_t> oldIndexes;
  std::swap(oldIndexes, indexes);

  values.resize(oldValues.size() + newValues.size());
  indexes.resize(oldValues.size() + newValues.size());
  if (isIterable) {
    iterablesIndexCount.insert(iterablesIndexCount.end(), newIterablesIndexCount.begin(), newIterablesIndexCount.end());
  }
  // merge oldValues and newValues
  std::size_t index5 = 0;
  std::size_t i1 = 0;
  std::size_t i2 = 0;
  auto nvs = newValues.size();
  auto ovs = oldValues.size();
  
  for (; i1 < ovs && i2 < nvs; ++index5) {
    if (newValues[i2] < oldValues[i1]) {
      values[index5] = newValues[i2];
      indexes[index5] = newIndexes[i2];
      i2++;
    } else {
      values[index5] = oldValues[i1];
      indexes[index5] = oldIndexes[i1];
      i1++;
    }
  }
  for (; i1 < ovs; i1++, index5++) {
    values[index5] = oldValues[i1];
    indexes[index5] = oldIndexes[i1];
  }

  for (; i2 < nvs; i2++, index5++) {
    values[index5] = newValues[i2];
    indexes[index5] = newIndexes[i2];
  }
  // Bisect again to recompute low and high.
  bounds = refilter(values);
  low = std::get<0>(bounds);
  high = std::get<1>(bounds);
}

template<typename V, typename T, bool isIterable>
template<bool Enable>
inline
std::enable_if_t<!isIterable && Enable>
DimensionImpl<V, T, isIterable>::add(data_iterator begin, data_iterator end) {
  oldDataSize = values.size();

  std::size_t newDataSize = std::distance(begin, end);

  newIndexes.resize(newDataSize);
  newValues.clear();
  newValues.reserve(newIndexes.size());

  std::iota(newIndexes.begin(), newIndexes.end(), values.size());


#ifndef USE_STD_SORT
  auto vs = values.size();
  DualPivotsort2::quicksort<std::size_t, value_type_t>(newIndexes, 0, newIndexes.size(),
                                                       [&begin, &vs, this](auto r) { return getter(*(begin + r - vs));});
#else
  auto vs = values.size();
  std::sort(newIndexes.begin(), newIndexes.end(),
            [begin, this, vs](auto lhs, auto rhs) {
              return getter(*(begin + lhs - values.size())) < getter(*(begin + rhs - vs));
            });
#endif

  for (auto &i : newIndexes) {
    newValues.push_back(getter(*(begin + i - values.size())));
  }
  auto bounds = refilter(newValues);
  auto lo1 = std::get<0>(bounds);
  auto hi1 = std::get<1>(bounds);
  if (refilterFunction) {
    for (std::size_t index2 = 0; index2 < newDataSize; ++index2) {
      if (!refilterFunction(newValues[index2])) {
        crossfilter->setBitForDimension(newIndexes[index2],
                                       dimensionOffset, dimensionBitIndex);
      }
    }
  } else {
    for (std::size_t i = 0; i < lo1; ++i) {
      crossfilter->setBitForDimension(newIndexes[i], dimensionOffset, dimensionBitIndex);
    }
    for (std::size_t i = hi1; i < newDataSize; ++i) {
      crossfilter->setBitForDimension(newIndexes[i], dimensionOffset, dimensionBitIndex);
    }
  }

  if (values.empty()) {
    values = newValues;
    indexes = newIndexes;
    low = lo1;
    high = hi1;
    return;
  }
  std::vector<value_type_t> oldValues;
  std::swap(values, oldValues);
  std::vector<std::size_t> oldIndexes;
  std::swap(oldIndexes, indexes);
  values.resize(oldValues.size() + newValues.size());
  indexes.resize(oldValues.size() + newValues.size());

  // merge oldValues and newValues
  std::size_t index5 = 0;
  std::size_t i1 = 0;
  std::size_t i2 = 0;
  auto ovs = oldValues.size();
  auto nvs = newValues.size();
  

  for (; i1 < ovs && i2 < nvs; ++index5) {
    if (newValues[i2] < oldValues[i1]) {
      values[index5] = newValues[i2];
      indexes[index5] = newIndexes[i2];
      i2++;
    } else {
      values[index5] = oldValues[i1];
      indexes[index5] = oldIndexes[i1];
      i1++;
    }
  }
  for (; i1 < ovs; i1++, index5++) {
    values[index5] = oldValues[i1];
    indexes[index5] = oldIndexes[i1];
  }

  for (; i2 < nvs; i2++, index5++) {
    values[index5] = newValues[i2];
    indexes[index5] = newIndexes[i2];
  }
  // Bisect again to recompute low and high.
  bounds = refilter(values);
  low = std::get<0>(bounds);
  high = std::get<1>(bounds);
}

template<typename V, typename T, bool isIterable>
inline
void  DimensionImpl<V, T, isIterable>::removeDataIterable(const std::vector<int64_t> &reIndex) {
  if (isIterable) {
    std::size_t i1 = 0;

    for (std::size_t i0 = 0; i0 < iterablesEmptyRows.size(); i0++) {
      if (reIndex[iterablesEmptyRows[i0]] != REMOVED_INDEX) {
        iterablesEmptyRows[i1] = reIndex[iterablesEmptyRows[i0]];
        i1++;
      }
    }
    iterablesEmptyRows.resize(i1);
    i1 = 0;
    for (std::size_t i0 = 0; i0 < iterablesIndexCount.size(); i0++) {
      if (reIndex[i0] != REMOVED_INDEX) {
        iterablesIndexCount[i1] = iterablesIndexCount[i0];
        i1++;
      }
    }
    iterablesIndexCount.resize(i1);
  }


  // Rewrite our index, overwriting removed values
  std::size_t n0 = values.size();
  std::size_t j = 0;
  for (std::size_t i = 0; i < n0; ++i) {
    auto cindex = indexes[i];
    if (reIndex[cindex] != REMOVED_INDEX) {
      if (i != j) {
        values[j] = values[i];
        indexes[j] = reIndex[cindex];
      }
      ++j;
    }
  }
  values.resize(j);
  indexes.resize(j);

  // Bisect again to recompute lo0 and hi0.
  auto bounds = refilter(values);
  low = std::get<0>(bounds);
  high = std::get<1>(bounds);

  removeSignal(reIndex);
}

template<typename V, typename T, bool isIterable>
inline
void  DimensionImpl<V, T, isIterable>::removeData(const std::vector<int64_t> &reIndex) {
  if (isIterable) {
    removeDataIterable(reIndex);
    return;
  }

  // Rewrite our index, overwriting removed values
  std::size_t n0 = values.size();
  std::size_t j = 0;
  std::vector<int64_t> nReIndex(n0);
  for(std::size_t i = 0; i < n0; i++) {
    if(reIndex[indexes[i]] == REMOVED_INDEX)
      nReIndex[i] = -1;
    else
      nReIndex[i] = indexes[i];
  }
  for (std::size_t i = 0; i < n0; ++i) {
    auto oldDataIndex = indexes[i];
    if (reIndex[oldDataIndex] != REMOVED_INDEX) {
      if (i != j)
        values[j] = values[i];
      indexes[j] = reIndex[oldDataIndex];
      ++j;
    }
  }
  values.resize(j);
  indexes.resize(j);
  // Bisect again to recompute lo0 and hi0.
  auto bounds = refilter(values);
  low = std::get<0>(bounds);
  high = std::get<1>(bounds);

  removeSignal(reIndex);
}

template<typename V, typename T, bool isIterable>
inline
std::tuple<std::size_t, std::size_t>
DimensionImpl<V, T, isIterable>::refilterRange(const value_type_t & left,
                                                        const value_type_t & right,
                                                        const std::vector<value_type_t> & data) {
  auto filterLow = std::lower_bound(data.begin(), data.end(), left);
  auto filterHigh = std::lower_bound(data.begin(), data.end(), right);
  std::size_t filterLowIndex = std::distance(data.begin(), filterLow);
  std::size_t filterHighIndex = std::distance(data.begin(), filterHigh);
  return std::make_tuple<std::size_t, std::size_t>(
      std::move(filterLowIndex), std::move(filterHighIndex));
}

template<typename V, typename T, bool isIterable>
inline
std::tuple<std::size_t, std::size_t>
DimensionImpl<V, T, isIterable>::refilterExact(const value_type_t & value,
                                                        const std::vector<value_type_t> & data) {
  auto filters = std::equal_range(data.begin(), data.end(), value);
  std::size_t filterLowIndex = std::distance(data.begin(), filters.first);
  std::size_t filterHighIndex = std::distance(data.begin(), filters.second);

  return std::make_tuple<std::size_t, std::size_t>(
      std::move(filterLowIndex), std::move(filterHighIndex));
}

template<typename V, typename T, bool isIterable>
template<bool Enable>
inline
std::enable_if_t<isIterable && Enable>
DimensionImpl<V, T, isIterable>::doFilter(std::size_t filterLowIndex, std::size_t filterHighIndex) {
  std::unordered_map<std::size_t, std::size_t> filterStatusAdded;
  std::unordered_map<std::size_t, std::size_t> filterStatusRemoved;
  // std::vector<std::size_t> added;
  // std::vector<std::size_t> removed;
  added.clear();
  removed.clear();
  
  filterStatusAdded.reserve((filterHighIndex - filterLowIndex)/2);
  filterStatusRemoved.reserve((filterHighIndex - filterLowIndex)/2);
  if(refilterFunctionFlag) {
    // reset previous filter
    refilterFunctionFlag = false;
    for(std::size_t i = 0; i < indexes.size(); i++) {
      if(i >= filterLowIndex && i < filterHighIndex) {
        if(crossfilter->getBitForDimension(indexes[i],dimensionOffset, dimensionBitIndex)) {
          filterStatusAdded[indexes[i]] += 1;
        }
      } else {
        if(!crossfilter->getBitForDimension(indexes[i],dimensionOffset, dimensionBitIndex)) {
          filterStatusRemoved[indexes[i]] += 1;
        }
      }
    }
  } else {

  if (filterLowIndex < low) {
    for (auto i = filterLowIndex; i < std::min(low, filterHighIndex); ++i) {
      auto ind = indexes[i];
      auto & status = filterStatusAdded[ind];
      if(status == 0)  {
        if(crossfilter->getBitForDimension(ind,dimensionOffset, dimensionBitIndex)) {
          status += 1;
        }
      } else {
        status += 1;
      }

    }
  } else if (filterLowIndex > low) {
    for (auto i = low; i < std::min(filterLowIndex, high); ++i) {
      filterStatusRemoved[indexes[i]] +=1;
    }
  }
  // Fast incremental update based on previous hi index.
  if (filterHighIndex > high) {
    for (auto i = std::max(filterLowIndex, high); i < filterHighIndex; ++i) {
      auto ind = indexes[i];
      auto & status = filterStatusAdded[ind];
      if(status == 0)  {
        if(crossfilter->getBitForDimension(indexes[i],dimensionOffset, dimensionBitIndex)) {
          status += 1;
        }
      } else {
        status += 1;
      }
    }
  } else if (filterHighIndex < high) {
    for (auto i = std::max(low, filterHighIndex); i < high; ++i) {
      filterStatusRemoved[indexes[i]] += 1;
    }
  }
  }
  if(filterLowIndex == 0 && filterHighIndex == values.size()) {
    for(auto i : iterablesEmptyRows) {
      if(crossfilter->getBitForDimension(i,dimensionOffset,dimensionBitIndex))
        filterStatusAdded[i] = 0;
    }
  } else {
    for(auto i : iterablesEmptyRows) {
      if(!crossfilter->getBitForDimension(i,dimensionOffset,dimensionBitIndex))
        filterStatusRemoved[i] = 0;
    }
  }
  added.reserve(filterStatusAdded.size()/2);
  removed.reserve(filterStatusRemoved.size()/2);
  for(auto f : filterStatusAdded) {
    if(!isIterable || f.second == iterablesIndexCount[f.first]) {
      crossfilter->resetBitForDimension(f.first,dimensionOffset,dimensionBitIndex);
      added.push_back(f.first);
    }
  }

  for(auto f : filterStatusRemoved) {
    if(!isIterable || f.second == iterablesIndexCount[f.first]) {
      removed.push_back(f.first);
      crossfilter->setBitForDimension(f.first,dimensionOffset,dimensionBitIndex);
    }
  }

  low = filterLowIndex;
  high = filterHighIndex;
  crossfilter->emitFilterSignal(dimensionOffset, dimensionBitIndex, added, removed);
  crossfilter->triggerOnChange("filtered");
  
}
template<typename V, typename T, bool isIterable>
inline void DimensionImpl<V, T, isIterable>::filter1(std::vector<std::size_t> & added, std::vector<std::size_t>& removed, std::size_t filterLowIndex, std::size_t filterHighIndex) {
  if (filterLowIndex < low) {
    for (auto i = filterLowIndex; i < std::min(low, filterHighIndex); ++i) {
      auto ind = indexes[i];
      if(crossfilter->getBitForDimension(ind,dimensionOffset, dimensionBitIndex)) {
        added.push_back(ind);
      }
    }
  } else if (filterLowIndex > low) {
    for (auto i = low; i < std::min(filterLowIndex, high); ++i) {
      removed.push_back(indexes[i]);
    }
  }
}
template<typename V, typename T, bool isIterable>
inline void DimensionImpl<V, T, isIterable>::filter2(std::vector<std::size_t> & added, std::vector<std::size_t>& removed, std::size_t filterLowIndex, std::size_t filterHighIndex) {
  // Fast incremental update based on previous hi index.
  if (filterHighIndex > high) {
    for (auto i = std::max(filterLowIndex, high); i < filterHighIndex; ++i) {
      auto ind = indexes[i];
      if(crossfilter->getBitForDimension(ind,dimensionOffset, dimensionBitIndex)) {
        added.push_back(ind);
      }
    }
  } else if (filterHighIndex < high) {
    for (auto i = std::max(low, filterHighIndex); i < high; ++i) {
      removed.push_back(indexes[i]);
    }
  }
}


template<typename V, typename T, bool isIterable>
template<bool Enable>
inline
std::enable_if_t<!isIterable && Enable>
DimensionImpl<V, T, isIterable>::doFilter(std::size_t filterLowIndex, std::size_t filterHighIndex) {
  // std::vector<std::size_t> added;
  // std::vector<std::size_t> removed;
  added.clear();
  removed.clear();
  
  added.reserve(indexes.size()/2);
  removed.reserve(indexes.size()/2);
  if(refilterFunctionFlag) {
    // reset previous filter
    refilterFunctionFlag = false;
    for(std::size_t i = 0; i < indexes.size(); i++) {
      if(i >= filterLowIndex && i < filterHighIndex) {
        if(crossfilter->getBitForDimension(indexes[i],dimensionOffset, dimensionBitIndex)) {
          added.push_back(indexes[i]);
        }
      } else {
        if(!crossfilter->getBitForDimension(indexes[i],dimensionOffset, dimensionBitIndex)) {
          removed.push_back(indexes[i]);
        }
      }
    }
  } else {
    filter1(added,removed,filterLowIndex, filterHighIndex);
    filter2(added,removed,filterLowIndex, filterHighIndex);
  }
  for(auto f : added) {
    crossfilter->resetBitForDimension(f,dimensionOffset,dimensionBitIndex);
  }

  for(auto f : removed) {
    crossfilter->setBitForDimension(f,dimensionOffset,dimensionBitIndex);
  }

  low = filterLowIndex;
  high = filterHighIndex;
  crossfilter->emitFilterSignal(dimensionOffset, dimensionBitIndex, added, removed);
  crossfilter->triggerOnChange("filtered");
}


template<typename V, typename T, bool isIterable>
inline
void  DimensionImpl<V, T, isIterable>::filterRange(const value_type_t & left, const value_type_t &right) {
  //  refilterFunctionFlag = false;
  refilter = [left, right, this](const std::vector<value_type_t> &val) {
    return this->refilterRange(left, right, val);
  };
  auto bounds = refilter(values);
  auto filterLowIndex = std::get<0>(bounds);
  auto filterHighIndex = std::get<1>(bounds);
  doFilter(filterLowIndex, filterHighIndex);
}

template<typename V, typename T, bool isIterable>
inline
void  DimensionImpl<V, T, isIterable>::filterExact(const value_type_t & value) {
  refilter = [value, this](const std::vector<value_type_t> &val) {
    return this->refilterExact(value, val);
  };
  auto bounds = refilter(values);
  auto filterLowIndex = std::get<0>(bounds);
  auto filterHighIndex = std::get<1>(bounds);
  doFilter(filterLowIndex, filterHighIndex);
}

template<typename V, typename T, bool isIterable>
inline
void  DimensionImpl<V, T, isIterable>::filterAll() {
  refilter = [](const std::vector<value_type_t> &val) {
    return std::make_tuple<std::size_t, std::size_t>(0, val.size());
  };

  doFilter(0, values.size());
}

template<typename V, typename T, bool isIterable>
inline
void  DimensionImpl<V, T, isIterable>::filterWithPredicate(
    std::function<bool(const value_type_t&)> filterFunc) {
  refilter = [](const std::vector<value_type_t> &val) {
    return std::make_tuple<std::size_t, std::size_t>(0, val.size());
  };
  if(!isIterable) {
    refilterFunction = filterFunc;
    refilterFunctionFlag = true;
    // std::vector<std::size_t> added;
    // std::vector<std::size_t> removed;
    added.clear();
    removed.clear();
    
    for (std::size_t i = 0; i < values.size(); i++) {
      if (crossfilter->getBitForDimension(indexes[i], dimensionOffset, dimensionBitIndex)
          && filterFunc(values[i])) {
        added.push_back(indexes[i]);
        crossfilter->resetBitForDimension(indexes[i], dimensionOffset, dimensionBitIndex);
        continue;
      }
      if (!crossfilter->getBitForDimension(indexes[i], dimensionOffset, dimensionBitIndex)
          && !filterFunc(values[i])) {
        removed.push_back(indexes[i]);
        crossfilter->setBitForDimension(indexes[i], dimensionOffset, dimensionBitIndex);
      }
    }
    low = 0;
    high = values.size();
    crossfilter->emitFilterSignal(dimensionOffset, dimensionBitIndex, added, removed);
  } else {
    std::unordered_map<std::size_t, std::size_t> filterStatusAdded;
    std::unordered_map<std::size_t, std::size_t> filterStatusRemoved;
    std::unordered_map<std::size_t, std::vector<std::size_t>> removedIndexes;
    std::unordered_map<std::size_t, std::vector<std::size_t>> addedIndexes;
    // std::vector<std::size_t> added;
    // std::vector<std::size_t> removed;
    added.clear();
    removed.clear();
    
    for(std::size_t i = 0; i < values.size(); i++) {
      bool filterStatus = crossfilter->getBitForDimension(indexes[i], dimensionOffset, dimensionBitIndex);
      auto filterTest = filterFunc(values[i]);
      if(filterStatus == false) { // item is in filter
        if(!filterTest) {
          filterStatusRemoved[indexes[i]] += 1;
          removedIndexes[indexes[i]].push_back(i);
        }
      } else { // item is not in filter
        if(filterTest) {
          filterStatusAdded[indexes[i]] += 1;
          addedIndexes[indexes[i]].push_back(i);
        }
      }
    }
    for(auto i : iterablesEmptyRows) {
      if(!crossfilter->getBitForDimension(i,dimensionOffset,dimensionBitIndex))
        filterStatusRemoved[i] = 0;
    }
    for(auto f : filterStatusAdded) {
      if(f.second != 0) {
        crossfilter->resetBitForDimension(f.first,dimensionOffset,dimensionBitIndex);
        added.push_back(f.first);
      }
    }

    for(auto f : filterStatusRemoved) {
      if(f.second == iterablesIndexCount[f.first]) {
        removed.push_back(f.first);
        crossfilter->setBitForDimension(f.first,dimensionOffset,dimensionBitIndex);
      }
    }
    refilterFunction = filterFunc;
    refilterFunctionFlag = true;
    low = 0;
    high = values.size();
    crossfilter->emitFilterSignal(dimensionOffset, dimensionBitIndex, added, removed);

  }
  crossfilter->triggerOnChange("filtered");
}

template<typename V, typename T, bool isIterable>
inline
auto
DimensionImpl<V, T, isIterable>::bottom(int64_t k, int64_t bottom_offset) ->std::vector<record_type_t> {
  if (low == high && low == crossfilter->size()  && iterablesEmptyRows.empty())
    return std::vector<record_type_t>();
  return crossfilter->bottom(low, high, k, bottom_offset, indexes, iterablesEmptyRows);
}

template<typename V, typename T, bool isIterable>
inline
auto
DimensionImpl<V, T, isIterable>::top(int64_t k, int64_t top_offset) -> std::vector<record_type_t> {
  if (low == high && low == crossfilter->size() && iterablesEmptyRows.empty())
    return std::vector<record_type_t>();

  return crossfilter->top(low, high, k, top_offset, indexes, iterablesEmptyRows);
}

template<typename V, typename T, bool isIterable>
template <typename K, typename R>
inline
auto
DimensionImpl<V, T, isIterable>::group(std::function<K(const value_type_t &)> key,
                                                std::function<R(R &, const record_type_t &, bool)> add_func_,
                                                std::function<R(R &, const record_type_t &, bool)> remove_func_,
                                       std::function<R()> initial_func_)-> Group<K, R, Dimension<V,T,isIterable>, false> {
  Group<K, R, Dimension<V,T,isIterable>, false> g(this, key, add_func_, remove_func_, initial_func_);
  g.add(values, indexes, 0, crossfilter->size());
  return g;
}

template<typename V, typename T, bool isIterable>
template <typename R>
inline
auto
DimensionImpl<V, T, isIterable>::groupAll(
    std::function<R(R &, const record_type_t &, bool)> add_func_,
    std::function<R(R &, const record_type_t &, bool)> remove_func_,
    std::function<R()> initial_func_) -> Group<std::size_t, R, Dimension<V,T,isIterable>, true> {
  Group<std::size_t, R, Dimension<V,T,isIterable>, true> g(
      this, [](const value_type_t &) { return std::size_t(0); }, add_func_, remove_func_, initial_func_);
  g.add(values, indexes, 0, crossfilter->size());
  return g;
}
