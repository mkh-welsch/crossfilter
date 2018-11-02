#include "jsdimension.hpp"
#include "jsfeature.hpp"


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

template<typename K, typename V, typename D, typename I >
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
    return jsdimension::create_feature(env,feature);
  }

#define CALL_DISPATCH_DIM_4(ktype, vtype, dtype, itype, fn, ...)  \
    if(itype) {                                                         \
      return fn<ktype,vtype,js_array<dtype>, cross::iterable>(__VA_ARGS__);  \
    } else {                                                            \
      return fn<ktype,vtype,dtype, cross::non_iterable>(__VA_ARGS__); \
    }                                                                   

#define CALL_DISPATCH_DIM_3(ktype, vtype, dtype, itype, fn, ...) \
  switch(dtype) {                                                       \
    case is_int64:                                                      \
      CALL_DISPATCH_DIM_4(ktype, vtype, int64_t, itype,fn, __VA_ARGS__); \
    case is_int32:                                                      \
      CALL_DISPATCH_DIM_4(ktype, vtype, int32_t, itype,fn, __VA_ARGS__); \
    case is_bool:                                                       \
      CALL_DISPATCH_DIM_4(ktype, vtype, bool, itype,fn, __VA_ARGS__); \
    case is_double:                                                     \
      CALL_DISPATCH_DIM_4(ktype, vtype, double, itype,fn, __VA_ARGS__); \
    case is_string:                                                     \
      CALL_DISPATCH_DIM_4(ktype, vtype, std::string,  itype,fn, __VA_ARGS__); \
    case is_uint64:                                                     \
      CALL_DISPATCH_DIM_4(ktype, vtype, uint64_t,  itype,fn, __VA_ARGS__); \
  }

#define CALL_DISPATCH_DIM_2(ktype, vtype, dtype, itype,fn, ...)  \
      switch(vtype) {                                   \
        case is_int64:                                  \
          CALL_DISPATCH_DIM_3(ktype, int64_t, dtype,itype, fn, __VA_ARGS__); \
          break;                                                   \
        case is_int32:                                  \
          CALL_DISPATCH_DIM_3(ktype, int32_t, dtype, itype,fn, __VA_ARGS__); \
          break;                                                   \
        case is_bool:                                   \
          CALL_DISPATCH_DIM_3(ktype, bool, dtype, itype, fn, __VA_ARGS__); \
          break;                                               \
        case is_double:                                 \
          CALL_DISPATCH_DIM_3(ktype, double, dtype, itype, fn, __VA_ARGS__); \
          break;                                                 \
        case is_uint64:                                                 \
          CALL_DISPATCH_DIM_3(ktype, uint64_t, dtype, itype, fn, __VA_ARGS__); \
        break;                                                          \
      }

#define CALL_DISPATCH_DIM(ktype, vtype, dtype, itype, fn, ...)    \
  switch(ktype) {                                                       \
    case is_int64:                                                      \
      CALL_DISPATCH_DIM_2(int64_t, vtype, dtype,itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_int32:                                                      \
      CALL_DISPATCH_DIM_2(int32_t, vtype, dtype,itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_bool:                                                       \
      CALL_DISPATCH_DIM_2(bool, vtype, dtype,itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_double:                                                     \
      CALL_DISPATCH_DIM_2(double, vtype, dtype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_string:                                                     \
      CALL_DISPATCH_DIM_2(std::string, vtype, dtype, itype, fn, __VA_ARGS__); \
      break;                                                            \
    case is_uint64:                                                     \
      CALL_DISPATCH_DIM_2(uint64_t, vtype, dtype, itype, fn, __VA_ARGS__); \
      break;                                                            \
  }
 napi_value  jsdimension::feature_sum(napi_env env, napi_callback_info info) {
    auto jsf = extract_function(env,info,4);
    jsdimension * obj = get_object<jsdimension>(env, jsf.jsthis);
    int32_t key_type = convert_to<int32_t>(env, jsf.args[0]);
    int32_t value_type = convert_to<int32_t>(env, jsf.args[1]);
    CALL_DISPATCH_DIM(key_type, value_type, obj->dim_type, obj->is_iterable, feature_sum_,env,jsf,obj,key_type, value_type);
    return nullptr;
  }



