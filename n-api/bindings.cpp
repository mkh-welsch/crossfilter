#include <node_api.h>
#include "jsfilter.hpp"
#include "utils.hpp"

napi_value New(napi_env env, napi_callback_info info) {
  napi_status status;

  // napi_value target;
  // NAPI_CALL(napi_get_new_target(env, info, &target));
  //  bool is_constructor = target != nullptr;
  js_function jsf = extract_function(env, info, 1);
  crossfilter * obj;
  if(jsf.args.empty()) {
    obj = new crossfilter(env, jsf.jsthis, nullptr);
  } else {
    napi_valuetype valuetype;
    napi_typeof(env, jsf.args[0], &valuetype);
    if(valuetype == napi_function) {
      obj = new crossfilter(env, jsf.jsthis, nullptr);
    } else {
      obj = new crossfilter(env, jsf.jsthis, nullptr);
    }
  }
  status = napi_wrap(env,
                     jsf.jsthis,
                     reinterpret_cast<void*>(obj),
                     crossfilter::Destructor,
                     nullptr,  // finalize_hint
                     &obj->wrapper);
  obj->env_ = env;
  assert(status == napi_ok);
  return jsf.jsthis;
  
}

// napi_value InitDim(napi_env env, napi_value exports) {
//   napi_property_descriptor properties[] = {
//     DECLARE_NAPI_METHOD("top", jsdimension::top)
//   };
//   napi_value cons;
//   napi_status status =
//       napi_define_class(env, "dimension", NAPI_AUTO_LENGTH, crossfilter::dimension_int64, nullptr, 1,properties, &cons);
//   assert(status == napi_ok);

//   status = napi_create_reference(env, cons, 1, &dimension::constructor);
//   assert(status == napi_ok);

//   status = napi_set_named_property(env, exports, "dimension", cons);
//   assert(status == napi_ok);
//   return exports;
// }

napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_property_descriptor properties[] = {
    DECLARE_NAPI_METHOD("remove", crossfilter::remove),
    DECLARE_NAPI_METHOD("add", crossfilter::add),
    DECLARE_NAPI_METHOD("size", crossfilter::size),
    DECLARE_NAPI_METHOD("all", crossfilter::all),
    DECLARE_NAPI_METHOD("dimension", crossfilter::dimension),
    DECLARE_NAPI_METHOD("feature", crossfilter::feature),
    DECLARE_NAPI_METHOD("feature_count", crossfilter::feature_count),
    DECLARE_NAPI_METHOD("feature_sum", crossfilter::feature_sum),
    DECLARE_NAPI_METHOD("all_filtered", crossfilter::all_filtered),
    DECLARE_NAPI_METHOD("is_element_filtered", crossfilter::is_element_filtered)
  };

  //InitDim(env, exports);

  napi_value cons;

  status =
      napi_define_class(env, "crossfilter", NAPI_AUTO_LENGTH, New, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons);
  assert(status == napi_ok);

  status = napi_create_reference(env, cons, 1, &crossfilter::constructor);
  assert(status == napi_ok);

  status = napi_set_named_property(env, exports, "crossfilter", cons);
  assert(status == napi_ok);
  return exports;
}


NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
