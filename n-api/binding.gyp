{
  "targets": [
    {
      "target_name": "<(module_name)",
      "sources": [ "feature_cast.cpp","jsfilter.cpp", "bindings.cpp", "jsdimension.cpp",  "jsdimension_top_bottom.cpp", "jsdimension_filter.cpp", "utils.cpp",
                   "jsfeature.cpp", "jsfeature_all.cpp", "jsfeature_size.cpp", "jsfeature_value.cpp", "jsfeature_order.cpp", "jsfeature_order_natural.cpp"],
      "include_dirs": [ "../../crossfilter/include"],
      "cflags": ['-O3', '-std=c++14'],
      'cflags_cc!': [ '-fno-rtti', '-fno-exceptions' ],
      'conditions': [
['OS=="mac"', {
'xcode_settings': {
'GCC_ENABLE_CPP_RTTI': 'YES',
'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
'OTHER_CFLAGS': [
"-std=c++14",
"-stdlib=libc++",
"-mmacosx-version-min=10.12"
],
}
}]
],
      "defines": ['CROSS_FILTER_MULTI_THREAD', 'CROSS_FILTER_USE_THREAD_POOL'],
      'product_dir': '<(module_path)'
    }
  ]
}

}