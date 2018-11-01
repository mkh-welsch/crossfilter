#include "jsfeature.hpp"
//#include "feature_export.hpp"
#include "crossfilter.hpp"
#include "utils.hpp"
#include "feature_cast_extern.hpp"

template<typename V>
static auto make_order_lambda(napi_env env, napi_ref &order, napi_ref &jsthis) {
  return [env, order, jsthis](const V & v) -> V {
           napi_value value= convert_from(env,v);
           napi_value lambda;
           NAPI_CALL(napi_get_reference_value(env, order, &lambda));
           napi_value this_value;
           NAPI_CALL(napi_get_reference_value(env, jsthis, &this_value));
           napi_value result;
           NAPI_CALL(napi_call_function(env, this_value, lambda, 1, &value,
                                        &result));
           return convert_to<V>(env, result);
         };
}
template<typename Key, typename Value, typename DimType, bool is_group_all, typename I>
static  napi_value order_(napi_env env, js_function & jsf, jsfeature * obj) {
  auto & feature = cast_feature<Key,Value, DimType,is_group_all, I>(obj->ptr);
    napi_ref order;
    NAPI_CALL(napi_create_reference(env, jsf.args[0], 1, &order));
    napi_ref jsthis;
    NAPI_CALL(napi_create_reference(env, jsf.jsthis, 1, &jsthis));
    feature.order(make_order_lambda<Value>(env, order, jsthis));
    return nullptr;
  }


napi_value  jsfeature::order(napi_env env, napi_callback_info info) {
  js_function jsf = extract_function(env, info, 1);
  jsfeature* obj = get_object<jsfeature>(env, jsf.jsthis);
  CALL_DISPATCH_F(obj->key_type, obj->value_type, obj->dim_type,obj->is_group_all, obj->is_iterable, order_, env, jsf, obj);
  return nullptr;
}
