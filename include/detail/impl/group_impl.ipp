/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#include <map>
template <typename Key, typename Reduce, typename Dimension,
          bool isGroupAll>
inline
void GroupImpl<Key, Reduce, Dimension, isGroupAll>::initSlots() {
  auto filterSlot = [this](std::size_t filterOffset, int filterBitNum,
                           const std::vector<std::size_t> &added,
                           const std::vector<std::size_t> &removed, bool notFilter) {
    update(filterOffset, filterBitNum, added, removed, notFilter);
  };

  auto addSlot = [this](const value_vec_t &newData,
                        const std::vector<std::size_t> &newIndexes,
                        std::size_t oldDataSize, std::size_t newDataSize) {
    add(newData, newIndexes, oldDataSize, newDataSize);
  };

  auto removeSlot = [this](const std::vector<int64_t> &removed) {
    remove(removed);
  };
  connectionFilter = dimension->connectFilterSlot(filterSlot);
  connectionAdd = dimension->connectAddSlot(addSlot);
  connectionRemove =  dimension->connectRemoveSlot(removeSlot);
  connectionDimension = dimension->connectDisposeSlot([this]() { dispose();});
}

template <typename Key, typename Reduce, typename Dimension,
          bool isGroupAll>
inline
void GroupImpl<Key, Reduce, Dimension, isGroupAll>::remove(const std::vector<int64_t> &removed) {
  if(isGroupAll)
    return;

  std::vector<int> seenGroups(groups.size(),0);
  std::size_t groupNum = 0;
  std::size_t j = 0;
  std::size_t removeCounter = 0;
  std::size_t sz = groupIndex.size();
  
  for(std::size_t i = 0; i < sz; i++) {
    if(removed[i] != REMOVED_INDEX) {
      groupIndex[j] = groupIndex[i];
      groupIndex.for_each(j,[&seenGroups](auto k) { seenGroups[k] = 1;});
      groupNum += groupIndex.size(j);
      // groupIndex[j] = groupIndex[i];
      // if constexpr(Dimension::getIsIterable()) {
      //     std::size_t sz1 = groupIndex[j].size();
      //     for(std::size_t i0 = 0; i0 < sz1 ; i0++) {
      //       seenGroups[groupIndex[j][i0]] = 1;
      //       groupNum++;
      //     }
      //   } else {
      //   seenGroups[groupIndex[j]] = 1;
      //   groupNum++;
      // }

      j++;

    } else {
      removeCounter++;
    }
  }
  std::vector<group_type_t> oldGroups;
  std::swap(oldGroups,groups);
  groups.reserve(groupNum);
  sz = seenGroups.size();

  for(std::size_t i = 0, k = 0; i < sz; i++) {
    if(seenGroups[i]) {
      seenGroups[i] = k++;
      groups.push_back(oldGroups[i]);
    }
  }
  for(std::size_t i = 0; i < j; i++) {
    groupIndex.set(i,[&seenGroups](auto k) {
        return seenGroups[k];
      });

    // if constexpr(Dimension::getIsIterable()) {
    //     auto sz1 = groupIndex[i].size();
    //     for(std::size_t i0 = 0; i0 < sz1; i0++) {
    //       groupIndex[i][i0] = seenGroups[groupIndex[i][i0]];
    //     }
    //   } else {
    //   groupIndex[i] = seenGroups[groupIndex[i]];
    // }
  }
  groupIndex.resize(groupIndex.size() - removeCounter);
}


template <typename Key, typename Reduce, typename Dimension,
          bool isGroupAll>
inline
void GroupImpl<Key, Reduce, Dimension, isGroupAll>::updateOne(std::size_t filterOffset, int filterBitIndex,
                                                              const std::vector<std::size_t> &added,
                                                              const std::vector<std::size_t> &removed,
                                                              bool notFilter) {
  if ((filterBitIndex > 0 && dimension->dimensionOffset == filterOffset &&
       dimension->dimensionBitIndex == filterBitIndex) ||
      resetNeeded)
    return;

  std::for_each(added.begin(), added.end(),
                [this](auto i) {
                  if(dimension->zeroExcept(i))
                    groups[0].second = add_func(groups[0].second,dimension->getRaw(i),false);
                });

  std::for_each(removed.begin(), removed.end(),
                [this, filterOffset, filterBitIndex, notFilter] (auto i) {
                  if(filterBitIndex < 0) {
                    if(dimension->zeroExcept(i))
                      groups[0].second = remove_func(groups[0].second,dimension->getRaw(i),notFilter);
                  } else {
                    if(dimension->onlyExcept(i,filterOffset, filterBitIndex))
                      groups[0].second = remove_func(groups[0].second,dimension->getRaw(i),notFilter);
                  }
                });
}

template <typename Key, typename Reduce, typename Dimension,
          bool isGroupAll>
inline
void GroupImpl<Key, Reduce, Dimension, isGroupAll>::update(std::size_t filterOffset, int filterBitIndex,
                                                           const std::vector<std::size_t> &added,
                                                           const std::vector<std::size_t> &removed,
                                                           bool notFilter) {
  if (isGroupAll && isFlatIndex) {
    updateOne(filterOffset, filterBitIndex, added, removed, notFilter);
    return;
  }

  if ((dimension->dimensionOffset == filterOffset &&
       dimension->dimensionBitIndex == filterBitIndex) ||
      resetNeeded)
    return;

  for(auto i : added) {
    if(dimension->zeroExcept(i)) {
      auto & elem = dimension->getRaw(i);
      groupIndex.for_each(i, [this,elem](auto j) {
          auto & g = groups[j];
          g.second = add_func(g.second, elem, false);
        });
      // if constexpr(Dimension::getIsIterable()) {
      // for(auto j : groupIndex[i] ){
      //   auto & g = groups[j];
      //   g.second = add_func(g.second, dimension->getRaw(i), false);
      // }
      //   } else {
      //   auto & g = groups[groupIndex[i]];
      //   g.second = add_func(g.second, dimension->getRaw(i), false);
      // }
    }
  }

  for(auto i : removed) {
    if(filterBitIndex < 0) {
      if(!dimension->zeroExcept(i))
        continue;
    } else {
      if(!dimension->onlyExcept(i, filterOffset, filterBitIndex))
        continue;
    }
    auto & elem = dimension->getRaw(i);
    groupIndex.for_each(i, [this,elem,notFilter](auto j) {
        auto & g = groups[j];
        g.second = remove_func(g.second, elem, notFilter);
      });

    // if constexpr(Dimension::getIsIterable()) {
    //     for(auto j : groupIndex[i]) {
    //       auto & g = groups[j];
    //       g.second = remove_func(g.second, dimension->getRaw(i), notFilter);
    //     }
    //   } else {
    //   auto & g = groups[groupIndex[i]];
    //   g.second = remove_func(g.second, dimension->getRaw(i), notFilter);
    // }
  }
}

template <typename Key, typename Reduce, typename Dimension,
          bool isGroupAll>
inline
void GroupImpl<Key, Reduce, Dimension, isGroupAll>::add(const value_vec_t &newData,
                                                   const std::vector<std::size_t> &newDataIndex,
                                                   std::size_t oldDataSize, std::size_t newDataSize) {
  if (newData.empty()) {
    groupIndex.resize(groupIndex.size() + newDataSize);
    return;
  }
  dataSize = newData.size() + oldDataSize;
  std::vector<group_type_t> newGroups;
  //  std::vector<group_index_t> newGroupIndex(groupIndex.size() + newDataSize);
  GroupIndex<isFlatIndex> newGroupIndex(groupIndex.size() + newDataSize);
  std::map<key_type_t, std::vector<std::size_t>>  oldIndexes;

  auto sz = groupIndex.size();
  for (std::size_t i = 0; i < sz; i++) {
    groupIndex.for_each(i,[this, i,&oldIndexes](auto k) {
        auto key = groups[k].first;
        oldIndexes[key].push_back(i);
      });

    // if constexpr(Dimension::getIsIterable()) {
    //     for(auto gIndex : groupIndex[i]) {
    //       auto key = groups[gIndex].first;
    //       oldIndexes[key].push_back(i);
    //     }
    //   } else {
    //   auto key = groups[groupIndex[i]].first;
    //   oldIndexes[key].push_back(i);
    // }
  }

  std::size_t i = 0;
  std::size_t i1 = 0;
  std::size_t i2 = 0;
  key_type_t lastKey =
      (groups.empty()) ? key(newData[0]) : groups[0].first;
  auto reduce_value_f = [this](auto & g, std::size_t i) {
    g.second = add_func(g.second, dimension->getRaw(i), true);
    if (!dimension->zeroExcept(i))
      g.second = remove_func(g.second, dimension->getRaw(i), false);
  };
  auto groupSize = groups.size();
  auto newValuesSize = newData.size();

  for (; i1 < groupSize && i2 < newValuesSize;) {
    auto newKey = key(newData[i2]);

    if (newKey < groups[i1].first) {
      if (newGroups.empty() || newGroups.back().first < newKey) {
        auto nGroup = std::make_pair(newKey, reduce_type_t());
        newGroups.push_back(nGroup);
        lastKey = newKey;
      }
      reduce_value_f(newGroups.back(), newDataIndex[i2]);
      newGroupIndex.setIndex(newDataIndex[i2],newGroups.size()-1);

      // if constexpr (Dimension::getIsIterable()) {
      //     newGroupIndex[newDataIndex[i2]].push_back(newGroups.size()-1);
      //   } else {
      //   newGroupIndex[newDataIndex[i2]] = newGroups.size()-1;
      // }
      i2++;
    } else if (newKey > groups[i1].first) {
      newGroups.push_back(groups[i1]);
      auto v = newGroups.size()-1;
      for (auto &j : oldIndexes[groups[i1].first]) {
        newGroupIndex.setIndex(j,v);
        // if constexpr(Dimension::getIsIterable()) {
        //     newGroupIndex[j].push_back(newGroups.size() - 1);
        //   } else {
        //   newGroupIndex[j] = newGroups.size() - 1;
        // }
      }
      lastKey = groups[i1].first;
      i1++;
    } else {
      //        skip new key
      lastKey = groups[i1].first;
      newGroups.push_back(groups[i1]);
      auto gindex = newGroups.size() - 1;;

      for (auto j : oldIndexes[groups[i1].first]) {
        newGroupIndex.setIndex(j,gindex);
        // if constexpr (Dimension::getIsIterable()) {
        //     newGroupIndex[j].push_back(gindex);
        //   } else {
        //   newGroupIndex[j] = gindex;
        // }

      }
      reduce_value_f(newGroups.back(), newDataIndex[i2]);
      newGroupIndex.setIndex(newDataIndex[i2],gindex);
      // if constexpr (Dimension::getIsIterable()) {
      //     newGroupIndex[newDataIndex[i2]].push_back(gindex);
      //   } else {
      //   newGroupIndex[newDataIndex[i2]] = gindex;
      // }
      i1++;
      i2++;
    }
  }
  for (; i1 < groupSize; i1++, i++) {
    newGroups.push_back(groups[i1]);
    auto gindex = newGroups.size() - 1;

    for (auto &j : oldIndexes[groups[i1].first]) {
      newGroupIndex.setIndex(j,gindex);
      // if constexpr(Dimension::getIsIterable()) {
      //     newGroupIndex[j].push_back(gindex);
      //   } else {
      //   newGroupIndex[j] = gindex;
      // }
    }
  }
  for (; i2 < newValuesSize; i2++, i++) {
    auto newKey = key(newData[i2]);
    if (newGroups.empty() || newKey > lastKey) {
      newGroups.push_back(std::make_pair(newKey, reduce_type_t()));
      lastKey = newKey;
    }
    newGroupIndex.setIndex(newDataIndex[i2],newGroups.size() - 1);
    // if constexpr(Dimension::getIsIterable()) {
    //     newGroupIndex[newDataIndex[i2]].push_back(newGroups.size() - 1);
    //   } else {
    //   newGroupIndex[newDataIndex[i2]] = newGroups.size() - 1;
    // }

    auto & g = newGroups[newGroups.size()-1];
    reduce_value_f(g, newDataIndex[i2]);
  }
  std::swap(groups, newGroups);
  std::swap(groupIndex, newGroupIndex);
}

template <typename Key, typename Reduce, typename Dimension,
          bool isGroupAll>
inline
std::vector<typename GroupImpl<Key, Reduce, Dimension, isGroupAll>::group_type_t>
GroupImpl<Key, Reduce, Dimension, isGroupAll>::top(std::size_t k) {
  auto &t = all();
  auto top = select_func(t, 0, groups.size(), k);
  return sort_func(top, 0, top.size());
}

template <typename Key, typename Reduce, typename Dimension,
          bool isGroupAll>
inline
void GroupImpl<Key, Reduce, Dimension, isGroupAll>::resetOne() {
  if (groups.empty())
      groups.push_back(group_type_t());

  group_type_t &g = groups[0];
  g.second = initial_func();
  auto groupIndexSize = groupIndex.size();
  for (std::size_t i = 0; i < groupIndexSize; ++i) {
    g.second = add_func(g.second, dimension->getRaw(i), true);
  }
  for (std::size_t i = 0; i < groupIndexSize; ++i) {
    if (!dimension->zeroExcept(i)) {
      g.second =
          remove_func(g.second, dimension->getRaw(i), false);
    }
  }
}

template <typename Key, typename Reduce, typename Dimension,
          bool isGroupAll>
inline
void GroupImpl<Key, Reduce, Dimension, isGroupAll>::reset() {
  if (isGroupAll && isFlatIndex) {
    resetOne();
    return;
  }
  // Reset all group values.
  for(auto &g: groups) {
    g.second = initial_func();
  }
  auto groupIndexSize = groupIndex.size();
  for (std::size_t i = 0; i < groupIndexSize; ++i) {
    auto & elem = dimension->getRaw(i);
    bool zeroExcept = dimension->zeroExcept(i);
    groupIndex.for_each(i,[this,elem,zeroExcept](auto k) {
        auto &g = groups[k];
        g.second = add_func(g.second, elem, true);
        if (!zeroExcept) {
          g.second =  remove_func(g.second, elem, false);
        }
      });

    // if constexpr(Dimension::getIsIterable()) {
    //     for(auto j : groupIndex[i]) {
    //       auto &g = groups[j];
    //       g.second = add_func(g.second, dimension->getRaw(i), true);
    //       if (!dimension->zeroExcept(i)) {
    //         g.second =
    //             remove_func(g.second, dimension->getRaw(i), false);
    //       }
    //     }
    //   } else {
    //   auto &g = groups[groupIndex[i]];
    //   g.second = add_func(g.second, dimension->getRaw(i), true);
    //   if (!dimension->zeroExcept(i)) {
    //     g.second =
    //         remove_func(g.second, dimension->getRaw(i), false);
    //   }
    // }
  }
}


template <typename Key, typename Reduce, typename Dimension,
          bool isGroupAll>
inline
std::vector<typename GroupImpl<Key, Reduce, Dimension, isGroupAll>::group_type_t> &
GroupImpl<Key, Reduce, Dimension, isGroupAll>::all() {
  if (resetNeeded) {
    reset();
    resetNeeded = false;
  }
  return groups;
}

template <typename K, typename R, typename D,
          bool b>
inline
typename GroupImpl<K, R, D, b>::reduce_type_t GroupImpl<K, R, D, b>::value() {
  if (resetNeeded) {
    reset();
    resetNeeded = false;
  }
  if (groups.empty())
    return initial_func();

  return groups[0].second;
}

template <typename Key, typename Reduce, typename Dimension,
          bool isGroupAll>
inline
GroupImpl<Key, Reduce, Dimension, isGroupAll> &GroupImpl<Key, Reduce, Dimension, isGroupAll>::reduce(
    std::function<reduce_type_t(reduce_type_t &, const record_type_t &, bool)> add_,
    std::function<reduce_type_t(reduce_type_t &, const record_type_t &, bool)> remove_,
    Reduce initial_) {
  add_func = add_;
  remove_func = remove_;
  initial_func = [initial_]() { return reduce_type_t(initial_); };
  resetNeeded = true;
  return *this;
}

template <typename Key, typename Reduce, typename Dimension,
          bool isGroupAll>
inline
GroupImpl<Key, Reduce, Dimension, isGroupAll> &GroupImpl<Key, Reduce, Dimension, isGroupAll>::reduceCount() {
  return reduce([]( reduce_type_t &v, const record_type_t &, bool) { return v + 1; },
                []( reduce_type_t &v, const record_type_t &, bool) { return v - 1; },
                reduce_type_t());
}

template <typename Key, typename Reduce, typename Dimension,
          bool isGroupAll>
inline
GroupImpl<Key, Reduce, Dimension, isGroupAll> &
GroupImpl<Key, Reduce, Dimension, isGroupAll>::reduceSum(std::function<reduce_type_t(const record_type_t &)> value) {
  return reduce(
      [value](reduce_type_t &v, const record_type_t &t, bool) {
        return v + value(t);
      },
      [value](reduce_type_t &v, const record_type_t &t, bool) {
        return v - value(t);
      },
      Reduce());
}

template <typename Key, typename Reduce, typename Dimension,
          bool isGroupAll>
template<typename OrderFunc>
inline
GroupImpl<Key, Reduce, Dimension, isGroupAll> &
GroupImpl<Key, Reduce, Dimension, isGroupAll>::order(OrderFunc value) {
  //  using ph = std::placeholders;
  using T = typename std::decay< decltype(value(std::declval<reduce_type_t>())) >::type;
  select_func = std::bind(Heap<group_type_t, T>::select,
                          std::placeholders::_1,
                          std::placeholders::_2,
                          std::placeholders::_3,
                          std::placeholders::_4,
                          [&value](const group_type_t& g) { return value(g.second);});

  sort_func = std::bind(Heap<group_type_t, T>::sort, std::placeholders::_1,
                        std::placeholders::_2,
                        std::placeholders::_3,
                        [&value](const group_type_t& g) { return value(g.second);});
  return *this;
}
