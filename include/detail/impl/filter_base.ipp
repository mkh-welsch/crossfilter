template<typename T>
inline
typename filter_base<T>::connection_type_t filter_base<T>::connect_filter_slot(std::function<void(std::size_t, int,
                                                                                                  const std::vector<std::size_t> &,
                                                                                                  const std::vector<std::size_t> &, bool)> slot) {
  return filter_signal.connect(slot);
}

template<typename T>
inline
typename filter_base<T>::connection_type_t
filter_base<T>::connect_remove_slot(std::function<void(const std::vector<int64_t> &)> slot) {
    return remove_signal.connect(slot);
}


template<typename T>
inline
typename filter_dim_base<T>::connection_type_t filter_dim_base<T>:: on_change(std::function<void(const std::string &)> callback) {
  return on_change_signal.connect(callback);
}

template<typename T>
inline
typename filter_dim_base<T>::connection_type_t
filter_dim_base<T>::connect_add_slot(std::function<void(data_iterator, data_iterator)> slot) {
  return add_signal.connect(slot);
}
template<typename T>
inline
typename filter_dim_base<T>::connection_type_t
filter_dim_base<T>::connect_post_add_slot(std::function<void(std::size_t)> slot) {
  return post_add_signal.connect(slot);
}

// template<typename T>
// inline
// void filter_dim_base<T>::emit_filter_signal(std::size_t filter_offset, int filter_bit_num,
//                                             const std::vector<std::size_t> & added,
//                                             const std::vector<std::size_t> & removed) {
//   filter_signal(filter_offset, filter_bit_num, added, removed, false);
// }

template<typename T, typename V>
inline
typename filter_feature_base<T,V>::connection_type_t
filter_feature_base<T,V>::connect_add_slot(std::function<void(const std::vector<value_type_t> &,
                                                              const std::vector<std::size_t> &,
                                                              std::size_t, std::size_t)> slot) {
  return add_group_signal.connect(slot);
}

