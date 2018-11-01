var cross = require("../lib/index");
var ctypes = cross.ctypes;
var filter = new cross.crossfilter();
var assert = require('assert');
console.log("hello");
var vows = require("vows");

var data = new cross.crossfilter();
console.log(data);

data.add([
    {a:1, b:[2,3]},
    {a:2, b:[3,4]},
    {a:3, b:[4,5,6]},
    {a:4, b:[6,7,8]},
    {a:6, b:[9,10]},
    {a:8, b:[11]},
    {a:10, b:[12,13]},
    {a:11, b:[]}
]);
var dim1 = data.dimension(ctypes.int64, function(d) { return d.b;}, true);
//console.log(dim1.top(2));
// data.dim2 = data.dimension(ctypes.int32, function(d) { return d.b;});
// data.dim3 = data.dimension(ctypes.bool, function(d) { return d.b.map(function(v) { return v %2 == 0;});});
// data.dim4 = data.dimension(ctypes.string, function(d) { return d.b.map(function(v) { return v + "";});});
//var f1 = dim1.feature_count(ctypes.int32, function(d) { return d;});
//var f2 = dim1.feature(ctypes.int64, ctypes.int64, function(d) { return d;}, function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});
var f1 = dim1.feature_all(ctypes.int64,function(r, rec) { return r + 1;},function(r, rec) { return r - 1;}, function() { return 0;});


console.log(f1.all());

// var f1 = dim1.feature_count(ctypes.string, function(d) { return "" + d;});
// console.log(f1.all());
// var data = new cross.crossfilter();
// data.add([
//     {a:1, b:2},
//     {a:2, b:3},
//     {a:3, b:4},
//     {a:4, b:5},
//     {a:6, b:7},
//     {a:8, b:9},
//     {a:10, b:11},
//     {a:11, b:12}
// ]);

// var dim = data.dimension(ctypes.int32, function(d) {return d.a;});
// var f1 = dim.feature_count(ctypes.bool, function(d) { return d % 2 == 0;});
// console.log(f1);

