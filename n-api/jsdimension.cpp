#include "jsdimension.hpp"
#include "jsfeature.hpp"

static napi_value  create_feature(napi_env env, jsfeature * feature) {
  napi_value f_this;
  NAPI_CALL(napi_create_object(env, &f_this));
  napi_wrap(env, f_this, reinterpret_cast<void *>(feature),
                      jsfeature::Destructor,
                      nullptr, // finalize_hint
                      &feature->wrapper);
  add_function(env, f_this,jsfeature::all, "all");
  add_function(env, f_this,jsfeature::top, "top");
  add_function(env, f_this,jsfeature::value,"value");
  add_function(env, f_this,jsfeature::size,"size");
  add_function(env, f_this,jsfeature::order,"order");
  add_function(env, f_this,jsfeature::order_natural,"order_natural");
  return f_this;
}




template<typename T, typename V>
static auto make_key(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & lambda_ref) {

  return [obj, this_ref,
          lambda_ref](V v) -> T {
           napi_value this_value;
           NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
           napi_value lambda_value;
           NAPI_CALL(napi_get_reference_value(obj->env_, lambda_ref, &lambda_value));

           napi_value value = convert_from<V>(obj->env_, v);
           napi_value result;
            napi_call_function(obj->env_, this_value, lambda_value, 1, &value,
                                                   &result);
           //assert(status == napi_ok);
           return convert_to<T>(obj->env_, result);};
}
template<typename T>
static auto make_value(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & value_ref) {

  return [obj, this_ref,
          value_ref](void * v) -> T {
           napi_value this_value;
           NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
           napi_value value_value;
           NAPI_CALL(napi_get_reference_value(obj->env_, value_ref, &value_value));

           napi_value value = extract_value(obj->env_, v, obj->filter_type);
           napi_value result;
           NAPI_CALL(napi_call_function(obj->env_, this_value, value_value, 1, &value,
                                        &result));
           return convert_to<T>(obj->env_, result);};
}

template<typename T>
static auto make_add(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & add_ref) {

  return [obj, this_ref,
               add_ref](T & r, void* const & rec, bool b ) -> T {
                napi_value args[3];
                args[0] = convert_from(obj->env_,r);
                //NAPI_CALL(napi_get_reference_value(obj->env_, rec, &args[1]));
                args[1] = extract_value(obj->env_, rec, obj->filter_type);
                args[2] = convert_from(obj->env_, b);
                napi_value this_value;
                NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
                napi_value add_;
                NAPI_CALL(napi_get_reference_value(obj->env_, add_ref, &add_));

                napi_value result;
                NAPI_CALL(napi_call_function(obj->env_, this_value, add_, 3, args,
                                             &result));
                return convert_to<T>(obj->env_, result);};
}
template<typename T>
static auto make_remove(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & remove_ref) {
  return [obj, this_ref,
          remove_ref](T & r,  void* const & rec, bool b ) -> T {
           napi_value args[3];
           args[0] = convert_from(obj->env_,r);
           //NAPI_CALL(napi_get_reference_value(obj->env_, rec, &args[1]));
           args[1] = extract_value(obj->env_, rec, obj->filter_type);
           args[2] = convert_from(obj->env_, b);
           napi_value this_value;
           NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
           napi_value remove_;
           NAPI_CALL(napi_get_reference_value(obj->env_, remove_ref, &remove_));
           
           napi_value result;
           NAPI_CALL(napi_call_function(obj->env_, this_value, remove_, 3, args,
                                        &result));
           return convert_to<T>(obj->env_, result);};
}
template<typename T>
static auto make_init(napi_env env, jsdimension * obj, napi_ref & this_ref, napi_ref & init_ref) {
  return [obj, this_ref,
          init_ref]() -> T {
           napi_value this_value;
           NAPI_CALL(napi_get_reference_value(obj->env_, this_ref, &this_value));
           napi_value init_;
           NAPI_CALL(napi_get_reference_value(obj->env_, init_ref, &init_));

           napi_value result;
           NAPI_CALL(napi_call_function(obj->env_, this_value, init_, 0, nullptr,
                                        &result));
           return convert_to<T>(obj->env_, result);};

}
template<typename K, typename R, typename D , bool, typename I >
  static napi_value feature_count_(napi_env env, js_function& jsf, jsdimension * obj, int key_type) {
    napi_ref this_ref;
    NAPI_CALL(napi_create_reference(env, jsf.jsthis, 1, &this_ref));
    napi_ref lambda_ref;
    NAPI_CALL(napi_create_reference(env, jsf.args[1], 1, &lambda_ref));

    auto & d = cast_dimension<D, I>(obj->dim);
    jsfeature * feature = new jsfeature();
    feature->key_type = key_type;
    feature->value_type = is_int64;
    feature->dim_type = obj->dim_type;
    feature->is_iterable = std::is_same<I,cross::iterable>::value;
    using value_type = typename std::remove_reference<decltype(d)>::type::value_type_t;
    feature->ptr = new feature_holder<K,std::size_t, typename std::remove_reference<decltype(d)>::type, false>(d.feature_count(make_key<K,value_type>(env,obj,this_ref,lambda_ref)));
    return create_feature(env, feature);
  }

template<typename K, typename R, typename D, bool ,typename I >
  static napi_value feature_(napi_env env, js_function& jsf, jsdimension * obj, int key_type, int value_type) {
    jsfeature * feature = new jsfeature();
    feature->key_type = key_type;
    feature->value_type = value_type;
    feature->dim_type = obj->dim_type;
    feature->is_iterable = std::is_same<I,cross::iterable>::value;

    napi_ref this_ref;
    NAPI_CALL(napi_create_reference(env, jsf.jsthis, 1, &this_ref));
    napi_ref key_ref;
    napi_ref remove_ref;
    napi_ref add_ref;
    napi_ref init_ref;
    NAPI_CALL(napi_create_reference(env, jsf.args[2], 1, &key_ref));
    NAPI_CALL(napi_create_reference(env, jsf.args[3], 1, &add_ref));
    NAPI_CALL(napi_create_reference(env, jsf.args[4], 1, &remove_ref));
    NAPI_CALL(napi_create_reference(env, jsf.args[5], 1, &init_ref));
    auto & d = cast_dimension<D, I>(obj->dim);
    using value_type_t = typename std::remove_reference<decltype(d)>::type::value_type_t;
    feature->ptr = new feature_holder<K,R,typename std::remove_reference<decltype(d)>::type, false>(d.feature(make_add<R>(env, obj, this_ref, add_ref),
                                                                                                              make_remove<R>(env, obj, this_ref, remove_ref),
                                                                                                              make_init<R>(env, obj, this_ref, init_ref),
                                                                                                              make_key<K,value_type_t>(env, obj, this_ref, key_ref)));
    return create_feature(env, feature);
  }


template<typename K, typename R, typename D, bool is_group_all, typename I >
  static std::enable_if_t<!(is_group_all && std::is_same<K,uint64_t>::value), napi_value> feature_all_(napi_env env, js_function& jsf, jsdimension * obj, int key_type, int value_type) {
    return nullptr;
  }

template<typename K, typename R, typename D, bool is_group_all, typename I >
  static std::enable_if_t<(is_group_all && std::is_same<K,uint64_t>::value), napi_value> feature_all_(napi_env env, js_function& jsf, jsdimension * obj, int key_type, int value_type) {
    jsfeature * feature = new jsfeature();
    feature->key_type = key_type;
    feature->value_type = value_type;
    feature->dim_type = obj->dim_type;
    feature->is_iterable = std::is_same<I,cross::iterable>::value;

    napi_ref this_ref;
    NAPI_CALL(napi_create_reference(env, jsf.jsthis, 1, &this_ref));
    napi_ref remove_ref;
    napi_ref add_ref;
    napi_ref init_ref;
    NAPI_CALL(napi_create_reference(env, jsf.args[1], 1, &add_ref));
    NAPI_CALL(napi_create_reference(env, jsf.args[2], 1, &remove_ref));
    NAPI_CALL(napi_create_reference(env, jsf.args[3], 1, &init_ref));
    auto & d = cast_dimension<D, I>(obj->dim);
    feature->ptr = new feature_holder<K,R,typename std::remove_reference<decltype(d)>::type, true>(d.feature_all(make_add<R>(env, obj, this_ref, add_ref),
                                                                                                                 make_remove<R>(env, obj, this_ref, remove_ref),
                                                                                                                 make_init<R>(env, obj, this_ref, init_ref)));
    feature->is_group_all  = true;
    return create_feature(env, feature);
  }

template<typename K, typename V, typename D, bool is_group_all, typename I >
  static napi_value feature_sum_(napi_env env, js_function& jsf, jsdimension * obj, int key_type, int value_type) {
    napi_ref this_ref;
    NAPI_CALL(napi_create_reference(env, jsf.jsthis, 1, &this_ref));
    napi_ref key_ref;
    NAPI_CALL(napi_create_reference(env, jsf.args[2], 1, &key_ref));
    napi_ref value_ref;
    NAPI_CALL(napi_create_reference(env, jsf.args[3], 1, &value_ref));
    auto & d = cast_dimension<D, I>(obj->dim);

    jsfeature * feature = new jsfeature();
    feature->key_type = key_type;
    feature->value_type = value_type;
    feature->dim_type = obj->dim_type;
    feature->is_iterable = std::is_same<I,cross::iterable>::value;
    using value_type_t = typename std::remove_reference<decltype(d)>::type::value_type_t;
    feature->ptr = new feature_holder<K,V,typename std::remove_reference<decltype(d)>::type, false>(std::move(d.feature_sum(make_value<V>(env, obj, this_ref, value_ref),
                                                                                                                            make_key<K,value_type_t>(env, obj, this_ref, key_ref))));
    return create_feature(env,feature);
  }
template<typename D, typename I >
  static napi_value feature_all_count_(napi_env env, js_function& jsf, jsdimension * obj) {
    jsfeature * feature = new jsfeature();
    feature->key_type = is_uint64;
    feature->value_type = is_uint64;
    feature->dim_type = obj->dim_type;
    feature->is_iterable = std::is_same<I,cross::iterable>::value;
    auto & d = cast_dimension<D, I>(obj->dim);
    feature->ptr = new feature_holder<uint64_t,uint64_t,typename std::remove_reference<decltype(d)>::type, true>(d.feature_all_count());
    feature->is_group_all  = true;
    return create_feature(env, feature);
  }



template<typename K, typename R, typename D, bool is_group_all, typename I >
  static std::enable_if_t<!(is_group_all && std::is_same<K,uint64_t>::value), napi_value> feature_all_sum_(napi_env env, js_function& jsf, jsdimension * obj, int key_type, int value_type) {
    return nullptr;
  }
template<typename K, typename R, typename D, bool is_group_all, typename I >
  static std::enable_if_t<(is_group_all && std::is_same<K,uint64_t>::value), napi_value> feature_all_sum_(napi_env env, js_function& jsf, jsdimension * obj, int key_type, int value_type) {
    napi_ref this_ref;
    NAPI_CALL(napi_create_reference(env, jsf.jsthis, 1, &this_ref));
    napi_ref value_ref;
    NAPI_CALL(napi_create_reference(env, jsf.args[1], 1, &value_ref));
    auto & d = cast_dimension<D, I>(obj->dim);

    jsfeature * feature = new jsfeature();
    feature->key_type = key_type;
    feature->value_type = value_type;
    feature->dim_type = obj->dim_type;
    feature->is_iterable = std::is_same<I,cross::iterable>::value;
    feature->ptr = new feature_holder<K,R,typename std::remove_reference<decltype(d)>::type, true>(std::move(d.feature_all_sum(make_value<R>(env, obj, this_ref, value_ref))));
    return create_feature(env,feature);
  }

void jsdimension::Destructor(napi_env env, void* nativeObject, void* finalize_hint) {
    auto obj = reinterpret_cast<jsdimension*>(nativeObject);
    obj->~jsdimension();
  }



#define CALL_DISPATCH_DIM_4(ktype, vtype, dtype, btype, itype, fn, ...)  \
  if(btype) {                                                           \
    if(itype) {                                                         \
      return fn<ktype,vtype,js_array<dtype>,true, cross::iterable>(__VA_ARGS__);  \
    } else {                                                            \
      return fn<ktype,vtype,dtype,true, cross::non_iterable>(__VA_ARGS__); \
    }                                                                   \
  } else {                                                              \
    if(itype) {                                                         \
      return fn<ktype,vtype,js_array<dtype>,false, cross::iterable>(__VA_ARGS__); \
    } else {                                                            \
      return fn<ktype,vtype,dtype,false, cross::non_iterable>(__VA_ARGS__); \
    }                                                                   \
  }

#define CALL_DISPATCH_DIM_3(ktype, vtype, dtype, btype, itype, fn, ...) \
  switch(dtype) {                                                       \
    case is_int64:                                                      \
      CALL_DISPATCH_DIM_4(ktype, vtype, int64_t, btype, itype,fn, __VA_ARGS__); \
    case is_int32:                                                      \
      CALL_DISPATCH_DIM_4(ktype, vtype, int32_t, btype, itype,fn, __VA_ARGS__); \
    case is_bool:                                                       \
      CALL_DISPATCH_DIM_4(ktype, vtype, bool, btype, itype,fn, __VA_ARGS__); \
    case is_double:                                                     \
      CALL_DISPATCH_DIM_4(ktype, vtype, double, btype, itype,fn, __VA_ARGS__); \
    case is_string:                                                     \
      CALL_DISPATCH_DIM_4(ktype, vtype, std::string, btype, itype,fn, __VA_ARGS__); \
    case is_uint64:                                                     \
      CALL_DISPATCH_DIM_4(ktype, vtype, uint64_t, btype, itype,fn, __VA_ARGS__); \
  }

#define CALL_DISPATCH_DIM_2(ktype, vtype, dtype, btype, itype,fn, ...)  \
      switch(vtype) {                                   \
        case is_int64:                                  \
          CALL_DISPATCH_DIM_3(ktype, int64_t, dtype, btype,itype, fn, __VA_ARGS__); \
          break;                                                   \
        case is_int32:                                  \
          CALL_DISPATCH_DIM_3(ktype, int32_t, dtype, btype, itype,fn, __VA_ARGS__); \
          break;                                                   \
        case is_bool:                                   \
          CALL_DISPATCH_DIM_3(ktype, bool, dtype, btype, itype, fn, __VA_ARGS__); \
          break;                                               \
        case is_double:                                 \
          CALL_DISPATCH_DIM_3(ktype, double, dtype, btype, itype, fn, __VA_ARGS__); \
          break;                                                 \
        case is_uint64:                                                 \
          CALL_DISPATCH_DIM_3(ktype, uint64_t, dtype, btype, itype, fn, __VA_ARGS__); \
        break;                                                          \
      }

#define CALL_DISPATCH_DIM(ktype, vtype, dtype, btype, itype, fn, ...)    \
  switch(ktype) {                                                       \
    case is_int64:                                                      \
      CALL_DISPATCH_DIM_2(int64_t, vtype, dtype, btype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_int32:                                                      \
      CALL_DISPATCH_DIM_2(int32_t, vtype, dtype, btype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_bool:                                                       \
      CALL_DISPATCH_DIM_2(bool, vtype, dtype, btype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_double:                                                     \
      CALL_DISPATCH_DIM_2(double, vtype, dtype, btype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_string:                                                     \
      CALL_DISPATCH_DIM_2(std::string, vtype, dtype, btype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_uint64:                                                     \
      CALL_DISPATCH_DIM_2(uint64_t, vtype, dtype, btype, itype, fn, __VA_ARGS__); \
      break;                                                            \
  }

 napi_value  jsdimension::feature_count(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env,info,2);
    jsdimension * obj = get_object<jsdimension>(env, jsf.jsthis);
    int32_t key_type = convert_to<int32_t>(env, jsf.args[0]);
    CALL_DISPATCH_DIM(key_type, is_uint64, obj->dim_type, false, obj->is_iterable, feature_count_, env, jsf, obj, key_type);
    return nullptr;
  }

 napi_value  jsdimension::feature_sum(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env,info,4);
    jsdimension * obj = get_object<jsdimension>(env, jsf.jsthis);
    int32_t key_type = convert_to<int32_t>(env, jsf.args[0]);
    int32_t value_type = convert_to<int32_t>(env, jsf.args[1]);
    CALL_DISPATCH_DIM(key_type, value_type, obj->dim_type, false, obj->is_iterable, feature_sum_,env,jsf,obj,key_type, value_type);
    return nullptr;
  }

napi_value  jsdimension::feature(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env,info,6);
    jsdimension * obj = get_object<jsdimension>(env, jsf.jsthis);
    int32_t key_type = convert_to<int32_t>(env, jsf.args[0]);
    int32_t value_type = convert_to<int32_t>(env, jsf.args[1]);
    CALL_DISPATCH_DIM(key_type, value_type, obj->dim_type, false, obj->is_iterable, feature_,env,jsf,obj,key_type, value_type);
    return nullptr;
  }

 napi_value  jsdimension::feature_all(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env,info,4);
    jsdimension * obj = get_object<jsdimension>(env, jsf.jsthis);
    int32_t key_type = is_uint64;
    int32_t value_type = convert_to<int32_t>(env, jsf.args[0]);
    CALL_DISPATCH_DIM(key_type, value_type, obj->dim_type, true, obj->is_iterable, feature_all_,env,jsf,obj,key_type, value_type);
    return nullptr;
  }

napi_value  jsdimension::feature_all_count(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env,info,0);
    jsdimension * obj = get_object<jsdimension>(env, jsf.jsthis);
    switch(obj->dim_type) {
      case is_int64:
        return (obj->is_iterable) ? feature_all_count_<js_array<int64_t>, cross::iterable>(env,jsf, obj) : feature_all_count_<int64_t, cross::non_iterable>(env,jsf, obj);
        break;
      case is_int32:
        return (obj->is_iterable) ? feature_all_count_<js_array<int32_t>, cross::iterable>(env,jsf, obj) : feature_all_count_<int32_t, cross::non_iterable>(env,jsf, obj);
        break;
      case is_bool:
        return (obj->is_iterable) ? feature_all_count_<js_array<bool>, cross::iterable>(env,jsf, obj) : feature_all_count_<bool, cross::non_iterable>(env,jsf, obj);
        break;
      case is_double:
        return (obj->is_iterable) ? feature_all_count_<js_array<double>, cross::iterable>(env,jsf, obj) : feature_all_count_<double, cross::non_iterable>(env,jsf, obj);
        break;
      case is_string:
        return (obj->is_iterable) ? feature_all_count_<js_array<std::string>, cross::iterable>(env,jsf, obj) : feature_all_count_<std::string, cross::non_iterable>(env,jsf, obj);
        break;

    }
    return nullptr;
}


napi_value  jsdimension::feature_all_sum(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env,info,2);
    jsdimension * obj = get_object<jsdimension>(env, jsf.jsthis);
    int32_t key_type = is_uint64;
    int32_t value_type = convert_to<int32_t>(env, jsf.args[0]);
    CALL_DISPATCH_DIM(key_type, value_type, obj->dim_type, true, obj->is_iterable, feature_all_sum_,env,jsf,obj,key_type, value_type);
    return nullptr;
  }
