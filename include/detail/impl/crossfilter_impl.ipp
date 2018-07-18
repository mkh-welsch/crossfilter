/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#include "detail/crossfilter_impl.hpp"
#include <numeric>
namespace {
static constexpr int REMOVED_INDEX = -1;
}
namespace cross {
namespace impl {

template<typename T, typename H>
inline
typename filter_impl<T,H>::connection_type_t filter_impl<T,H>::connect_filter_slot(std::function<void(std::size_t, int,
                                                                                                      const std::vector<std::size_t> &,
                                                                                                      const std::vector<std::size_t> &, bool)> slot) {
  return filter_signal.connect(slot);
}

template<typename T, typename H>
inline
typename filter_impl<T,H>::connection_type_t
filter_impl<T,H>::connect_remove_slot(std::function<void(const std::vector<int64_t> &)> slot) {
  return remove_signal.connect(slot);
}

template<typename T, typename H>
inline
typename filter_impl<T,H>::connection_type_t
filter_impl<T,H>::connect_add_slot(std::function<void(const std::vector<value_type_t> &,
                                                      const std::vector<std::size_t> &,
                                                      std::size_t, std::size_t)> slot) {
  return add_group_signal.connect(slot);
}

template<typename T, typename H>
inline
typename filter_impl<T,H>::connection_type_t
filter_impl<T,H>::connect_dispose_slot(std::function<void()> slot)  {
  return  dispose_signal.connect(slot);
}

template<typename T, typename H>
inline
typename filter_impl<T,H>::connection_type_t
filter_impl<T,H>:: on_change(std::function<void(const std::string &)> callback) {
  return on_change_signal.connect(callback);
}

template<typename T, typename H>
inline
typename filter_impl<T,H>::connection_type_t
filter_impl<T,H>::connect_add_slot(std::function<void(data_iterator, data_iterator)> slot) {
  return add_signal.connect(slot);
}
template<typename T, typename H>
inline
typename filter_impl<T,H>::connection_type_t
filter_impl<T,H>::connect_post_add_slot(std::function<void(std::size_t)> slot) {
  return post_add_signal.connect(slot);
}

template<typename T, typename H>
inline std::vector<T> filter_impl<T,H>::bottom(std::size_t low, std::size_t high, int64_t k,
                                                 int64_t offset,
                                                 const std::vector<std::size_t> &indexes,
                                                 const std::vector<std::size_t> &empty) {
  std::vector<T> result;
  auto high_iter = indexes.begin();
  auto low_iter = indexes.begin();
  std::advance(high_iter, high);
  std::advance(low_iter, low);
  int64_t result_size = 0;
  for(auto p = empty.begin(); p < empty.end() && result_size < k; p++) {
    if(filters.zero(*p)) {
      if (offset-- > 0)
        continue;
      result.push_back(data[*p]);
      result_size++;
    }
  }
  if (low_iter > high_iter)
    return result;
  for (; low_iter < high_iter && result_size < k; low_iter++) {
    if (filters.zero(*low_iter)) {
      if (offset-- > 0)
        continue;
      result.push_back(data[*low_iter]);
      result_size++;
    }
  }
  return result;
}
template<typename T, typename H>
inline
std::vector<T> filter_impl<T,H>::top(std::size_t low, std::size_t high, int64_t k,
                                   int64_t offset,
                                   const std::vector<std::size_t> &indexes, const std::vector<std::size_t> &empty) {
    std::vector<T> result;
    auto high_iter = indexes.begin();
    auto low_iter = indexes.begin();
    std::advance(high_iter, high - 1);
    std::advance(low_iter, low);
    int64_t result_size = 0;
    int64_t skip = offset;

    if  (high_iter >= low_iter) {
      for (; high_iter >= low_iter && result_size < k ; high_iter--) {
        if (filters.zero(*high_iter)) {
          if (skip-- > 0)
            continue;
          result.push_back(data[*high_iter]);
          result_size++;
        }
      }
    }
    if(result_size < k) {
      for(auto p = empty.begin(); p < empty.end() && result_size < k; p++) {
        if(filters.zero(*p)) {
          if (skip-- > 0)
            continue;
          result.push_back(data[*p]);
          result_size++;
        }
      }
    }
    return result;
  }



template<typename T, typename H>
template<typename G>
inline
auto filter_impl<T,H>::dimension(G getter)
    -> cross::dimension<decltype(getter(std::declval<T>())), T, false> {
  auto dimension_filter = filters.add_row();
  cross::dimension<decltype(getter(std::declval<record_type_t>())), T, false> dim(this, dimension_filter, getter);
  dim.add(data.begin(), data.end());
  return dim;
}

template<typename T, typename H>
template<typename V, typename G>
inline
auto filter_impl<T,H>::iterable_dimension(G getter) -> cross::dimension<V, T, true> {
  auto dimension_filter = filters.add_row();
  cross::dimension<V, T, true> dim(this, dimension_filter, getter);
  dim.add(data.begin(), data.end());
  return dim;
}

template<typename T, typename H>
inline
std::vector<uint8_t> & filter_impl<T,H>::mask_for_dimension(std::vector<uint8_t> & mask) const {
  return mask;
}

template<typename T, typename H>
template<typename D>
inline
std::vector<uint8_t> & filter_impl<T,H>::mask_for_dimension(std::vector<uint8_t> & mask,
                                                            const D & dim) const {
  std::bitset<8> b = mask[dim.get_offset()];
  b.set(dim.get_bit_index(), true);

  mask[dim.get_offset()] = b.to_ulong();
  return mask;
}

template<typename T, typename H>
template<typename D, typename ...Ts>
inline
std::vector<uint8_t> & filter_impl<T,H>::mask_for_dimension(std::vector<uint8_t> & mask,
                                                          const D & dim,
                                                          Ts&... dimensions) const {
  std::bitset<8> b = mask[dim.get_offset()];
  b.set(dim.get_bit_index(), true);
  mask[dim.get_offset()] = b.to_ulong();
  return mask_for_dimension(mask, dimensions...);
}

template<typename T, typename H>
template<typename ...Ts>
inline
std::vector<T> filter_impl<T,H>::all_filtered(Ts&... dimensions) const {
  std::vector<uint8_t> mask(filters.size());
  mask_for_dimension(mask, dimensions...);
  std::vector<T> result;

  for (std::size_t i = 0; i < data.size(); i++) {
    if (filters.zero_except_mask(i, mask))
      result.push_back(data[i]);
  }
  return result;
}

template<typename T, typename H>
template<typename ...Ts>
inline
bool filter_impl<T,H>::is_element_filtered(std::size_t index, Ts&... dimensions) const {
  std::vector<uint8_t> mask(filters.size());
  mask_for_dimension(mask, dimensions...);
  return filters.zero_except_mask(index, mask);
}

template<typename T, typename H>
template<typename C>
inline
filter_impl<T,H> & filter_impl<T,H>::add(const C &new_data,  bool allow_duplicates) {
  auto old_data_size = data.size();
  auto begin = std::begin(new_data);
  auto end = std::end(new_data);

  auto new_data_size = std::distance(begin, end);

  if (new_data_size > 0) {
    if(allow_duplicates) {
      std::transform(begin, end, std::back_inserter(data),[this](auto v) {
          auto h = hash(v);
          auto p = hash_table.find(h);
          if(p == hash_table.end())
              hash_table[h] = 1;
          else
              p->second++;
        return v;
      }
        );
      //      data.insert(data.end(), begin, end);
      data_size += new_data_size;
    } else {
      std::size_t count = 0;
      for(auto p = begin; p != end; ++p) {
        auto h = hash(*p);
        if(hash_table.find(h) == hash_table.end()) {
          data.push_back(*p);
          hash_table[h] = 1;
          count++;
        }
      }
      data_size += count;
      new_data_size = count;
    }
    filters.resize(data_size);
    data_iterator nbegin = data.begin();
    std::advance(nbegin, old_data_size);

    data_iterator nend = nbegin;
    std::advance(nend, new_data_size);

    add_signal(nbegin, nend);
    post_add_signal(new_data_size);

    if(add_group_signal.num_slots() != 0) {
      // FIXME: remove temporary vector
      std::vector<record_type_t> tmp(begin, end);
      std::vector<std::size_t> indexes(tmp.size());
      std::iota(indexes.begin(),indexes.end(),old_data_size);
      add_group_signal(tmp, indexes, old_data_size, new_data_size);
    }

    trigger_on_change("dataAdded");
  }
  return *this;
}

template<typename T, typename H>
inline
filter_impl<T,H> & filter_impl<T,H>::add(const T &new_data,bool allow_duplicates) {
  auto old_data_size = data.size();

  auto new_data_size = 1;
  auto h = hash(new_data);
  if(allow_duplicates) {
    data.push_back(new_data);
    auto p = hash_table.find(h);
    if(p == hash_table.end())
        hash_table[h] = 1;
    else
        p->second++;
  } else {
    if(hash_table.find(h) == hash_table.end()) {
      data.push_back(new_data);
      hash_table[h] = 1;
    } else {
      return *this;
    }
  }

  data_size += new_data_size;
  filters.resize(data_size);

  // FIXME: remove temporary vector
  std::vector<T> tmp;
  tmp.push_back(new_data);

  add_signal(tmp.begin(), tmp.end());
  post_add_signal(new_data_size);

  if(add_group_signal.num_slots() != 0) {
    std::vector<std::size_t> indexes(tmp.size());
    std::iota(indexes.begin(),indexes.end(),old_data_size);
    add_group_signal(tmp, indexes, old_data_size, new_data_size);
  }
  trigger_on_change("dataAdded");
  return *this;
}


template<typename T, typename H>
inline
void filter_impl<T,H>::flip_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                           int dimension_bit_index) {
    filters.flip(index, dimension_offset, dimension_bit_index);
  }
template<typename T, typename H>
inline
void filter_impl<T,H>::set_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                                           int dimension_bit_index) {
  filters.set(index, dimension_offset, dimension_bit_index);
}
template<typename T, typename H>
inline
void filter_impl<T,H>::reset_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                                            int dimension_bit_index) {
  filters.reset(index, dimension_offset, dimension_bit_index);
}

template<typename T, typename H>
inline
bool filter_impl<T,H>::get_bit_for_dimension(std::size_t index, std::size_t dimension_offset,
                                            int dimension_bit_index) {
  return filters.check(index, dimension_offset, dimension_bit_index);
}

template<typename T, typename H>
inline
auto
filter_impl<T,H>::get_data_iterators_for_indexes(std::size_t low, std::size_t high)
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

template<typename T, typename H>
inline
void filter_impl<T,H>::remove_data(std::function<bool(int)> should_remove) {
    std::vector<int64_t> new_index(data_size);

    std::vector<std::size_t> removed;

    for (std::size_t index1 = 0, index2 = 0; index1 < data_size; ++index1) {
      if (should_remove(index1)) {
        removed.push_back(index1);
        new_index[index1] = REMOVED_INDEX;
        auto h = hash(data[index1]);
        auto p = hash_table.find(h);
        if(--(p->second) == 0) {
            hash_table.erase(p);
        }
      } else {
        new_index[index1] = index2++;
      }
    }

    // Remove all matching records from groups.
    filter_signal(std::numeric_limits<std::size_t>::max(), -1,
                                  std::vector<std::size_t>(), removed, true);
    // Update indexes.
    remove_signal(new_index);

    // Remove old filters and data by overwriting.
    std::size_t index4 = 0;

    for (std::size_t index3 = 0; index3 < data_size; ++index3) {
      if (new_index[index3] != REMOVED_INDEX) {
        if (index3 != index4) {
          filters.copy(index4, index3);
          data[index4] = data[index3];
        }
        ++index4;
      }
    }

    data_size = index4;
    data.resize(data_size);
    filters.truncate(index4);
    trigger_on_change("dataRemoved");
  }


  // removes all records matching the predicate (ignoring filters).
template<typename T, typename H>
inline
void filter_impl<T,H>::remove(std::function<bool(const T&, int)> predicate) {
  remove_data([&](auto i) { return predicate(data[i], i); });
}

template<typename T, typename H>
inline
  // Removes all records that match the current filters
void filter_impl<T,H>::remove() {
  remove_data([&](auto i) { return filters.zero(i); });
}

template<typename T, typename H>
template <typename AddFunc, typename RemoveFunc, typename InitialFunc>
inline
auto  filter_impl<T,H>::feature(
    AddFunc  add_func_,
    RemoveFunc remove_func_,
    InitialFunc initial_func_) -> cross::feature<std::size_t,
                                                 decltype(initial_func_()), T, value_type_t, true, false> {
  using reduce_type_t = decltype(initial_func_());

  cross::feature<std::size_t, reduce_type_t, T, value_type_t, true, false> g(this,
                                                                      [](const value_type_t& ) { return std::size_t(0);},
                                                                      add_func_,
                                                                      remove_func_,
                                                                      initial_func_);
  std::vector<std::size_t> indexes(data.size());
  std::iota(indexes.begin(),indexes.end(),0);
  g.add(data, indexes, 0,data.size());
  return g;
}

} //namespace impl
} //namespace cross
