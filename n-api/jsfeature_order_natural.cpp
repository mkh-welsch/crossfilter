#include "jsfeature.hpp"
//#include "feature_export.hpp"
#include "crossfilter.hpp"
#include "utils.hpp"
#include "feature_cast_extern.hpp"


template<typename Key, typename Value, typename DimType, bool is_group_all, typename I>
static  napi_value order_natural_(napi_env env, js_function & jsf, jsfeature * obj) {
  auto & feature = cast_feature<Key,Value, DimType, is_group_all, I>(obj->ptr);
  feature.order_natural();
  return nullptr;
}
napi_value  jsfeature::order_natural(napi_env env, napi_callback_info info) {
  js_function jsf = extract_function(env, info, 0);
  jsfeature* obj = get_object<jsfeature>(env, jsf.jsthis);
  CALL_DISPATCH_F(obj->key_type, obj->value_type, obj->dim_type,obj->is_group_all, obj->is_iterable,  order_natural_, env, jsf, obj);
  return nullptr;
}
