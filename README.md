# crossfilter
cross::filter adds OLAP functionality on top of an underlying concurrent std::vector. 

At first crossfilter is a theadsafe drop in replacement for std::vector.

On top of the basic functionality of std::vector you can can define some kind of filtered views, for example the top 10 based on a multi-dimensional filter setup, which are than automatically updated (!) during usual data operations on the data. So you could build a lightweight and header only (M)OLAP in memory query database for ultra fast analysis big data sets and complex and real time filtering tasks.

## quick instructions

At first we have some abritray structures data called Record

```C++
struct Record
{
    std::string date;
    int quantity;
    int total;
    int tip;
    std::string type;
    std::vector<std::string> productIDS;
};
```

Lets store some of theses records in cross::vector and investigate the new possibilities…

```C++
cross::filter<Record> payments = {
    {"2011-11-14T16:17:54Z", 2, 190, 100, "tab", {"001", "002"}},
    {"2011-11-14T16:20:19Z", 2, 190, 100, "tab", {"001", "005"}},
    {"2011-11-14T16:28:54Z", 1, 300, 200, "visa", {"004", "005"}},
    {"2011-11-14T16:30:43Z", 2, 90, 1, "tab", {"001", "002"}},
    {"2011-11-14T16:48:46Z", 2, 90, 2, "tab", {"005"}},
    {"2011-11-14T16:53:41Z", 2, 90, 3, "tab", {"001", "004", "005"}},
    {"2011-11-14T16:54:06Z", 1, 100, 4, "cash", {"001", "002", "003", "004", "005"}},
    {"2011-11-14T16:58:03Z", 2, 90, 5, "visa", {"001"}},
    {"2011-11-14T17:07:21Z", 2, 90, 6, "tab", {"004", "005"}},
    {"2011-11-14T17:22:59Z", 2, 90, 7, "tab", {"001", "002", "004", "005"}}
};
```
you can use all functionalties of std::vector<> with cross::filter<>, but operations are thread safe in comparison to std::vector

To use the special crossfilters functionality, you have to prepare „dimensions“ on the record sets. A dimension is just a sortable column by regarding the record set as table with single entries. 

You build a dimension by giving an accressor function as generic lambda.

```C++
auto totals = payments.dimension([](auto r) { return r.total;});
```

Now, you can filter the record set in regarding to this dimension ...

```C++
totals.filter(90); 							    // filter out every record, where total is 90
```
... and give back the filtered results as copys

```C++
auto filtered_results = payments.all_filtered();
    // "2011-11-14T16:30:43Z", 2, 90, 1, "tab", {"001", "002"}
    // "2011-11-14T16:48:46Z", 2, 90, 2, "tab", {"005"}
    // "2011-11-14T16:53:41Z", 2, 90, 3, "tab", {"001", "004", "005"}
    // "2011-11-14T16:58:03Z", 2, 90, 5, "visa", {"001"}
    // "2011-11-14T17:07:21Z", 2, 90, 6, "tab", {"004", "005"}
    // "2011-11-14T17:22:59Z", 2, 90, 7, "tab", {"001", "002", "004", "005"}
```

Now, lets add another dimension and filter and get the combined results of both filters
```C++
auto tips = payments.dimension([](auto r) { return r.tip; });
tips.filter(2, 7);                              // filter by range (2 ...... 6.999)
filtered_results = payments.all_filtered();
    // "2011-11-14T16:48:46Z", 2, 90, 2, "tab", {"005"}
    // "2011-11-14T16:53:41Z", 2, 90, 3, "tab", {"001", "004", "005"}
    // "2011-11-14T16:58:03Z", 2, 90, 5, "tab", {"001"}
    // "2011-11-14T17:07:21Z", 2, 90, 6, "visa", {"004", "005"}
```

Dimension can be a combination of different rentries as long the accessor function returs a sortable type. however, you can make type conversion on the fly.

```C++
auto products = payments.dimension([](auto r) { return r.productIDS.size(); });
products.filter([](auto d) { return d >= 2; });     // filter by custom function
filtered_results = payments.all_filtered();
    // "2011-11-14T16:53:41Z", 2, 90, 3, "tab", {"001", "004", "005"}
    // "2011-11-14T17:07:21Z", 2, 90, 6, "visa", {"004", "005"}
```

If you have a not sortable type, but just are interested on specific entries a workaroud is to make them binary sortable.

```C++
auto no_tabs = payments.dimension([](auto r) { return r.type != std::string("tab") ? 1 : 0; });
no_tabs.filter(1); 
filtered_results = payments.all_filtered();
    // "2011-11-14T17:07:21Z", 2, 90, 6, "visa", {"004", "005"}

```

Ok, that is just simple filtering, but the real power of cross::filter keep is to keep this filters updated if you add or change data to payments.


```C++
payments.add({"2011-11-14T17:20:20Z", 4, 90, 2, "cash", {"001", "002"}});
filtered_results = payments.all_filtered();
    // "2011-11-14T17:07:21Z", 2, 90, 6, "visa", {"004", "005"}
    // "2011-11-14T17:20:20Z", 4, 90, 2, "cash", {"001", "002"}
```

## define dimensions

you have to prepare a dimension for everything you want to filter with cross::filter 

```C++
auto dim = payments.dimension([](auto r) { return r.total;});
```

Inside cross::filter a new hashmap is build as kind of ghost copy, which keeps the totals incrementally sorted  [90,90,90,90,90,90,100,190,190,300] when changing payments or add a new or delete a record. So every dimension needs memmory space. If you don't need a dimension, remove it.



## filter

You can get a copy of all filtered data with

```C++
filtered_results = payments.all_filtered();
```

however, you can always get a copy of all records by not regarding the filters

```C++
auto all_records = filtered_results1 = payments.all();
```

If you want only the top or the bottom ones, you can use specific methods. But in this case you have to adress payments directly, but a specic dimension, so cross::filter knows, how to order the records for you.

```C++
auto top_filtered1 = totals.top(2);
    //                           ^
    // "2011-11-14T16:54:06Z" 1 100 4 cash {"001", "002", "003", "004", "005"}
    // "2011-11-14T17:20:20Z" 4 90 2 cash {"001", "002"}

auto top_filtered2 = tips.bottom(3);
    //                             `´
    // "2011-11-14T17:20:20Z" 4 90 2 cash {"001", "002"}
    // "2011-11-14T16:53:41Z" 2 90 3 tab {"001", "004", "005"}
    // "2011-11-14T16:54:06Z" 1 100 4 cash {"001", "002", "003", "004", "005"}

```



You can set filters on three different ways.

```C++
totals.filter(80) 							// by value
totals.filter(90,100); 						// by range (90 <= x < 100)
totals.filter([](auto r) { return (r.total <80); }) 	// by custom function, which returns a boolean
```

You can reset the individual filter or change it every time
```C++
no_tabs.filter();
totals.filter(100);

 filtered_results = payments.all_filtered();
    // "2011-11-14T16:54:06Z", 1, 100, 4, "cash", {"001", "002", "003", "004", "005"}

```

## grouping (aggregated records)

docu coming soon...

## run
cross::filter work head only. just include the header
```C++
#include "crossfilter.hpp"
```

and compile for example with
```terminal
clang++ ./demo/demo1.cpp -o .demo -std=c++14
```

however tests & benchmark require a build system
```console
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ../
$ make (currently broken)
$ make test
$ make benchmark
$ ../benchmark/benchBin/benchmark1
```



