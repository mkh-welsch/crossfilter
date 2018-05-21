#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE CrossFilterJSTest
#include "utils.hpp"
#include <boost/test/unit_test.hpp>
#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <utility>
#include <cmath>
#include "crossfilter.hpp"

auto Infinity = std::numeric_limits<int>::max();

struct Fixture {
  CrossFilter<Record> cr;
  Dimension<std::string,Record,false> date;
  Dimension<int,Record,false> quantity;
  Dimension<int,Record,false> total;
  Dimension<int,Record,false> tip;
  Dimension<std::string,Record,false> type;
  //  Dimension<int,CrossFilterImpl<Record>,true> tags;
  Fixture() {
    Record data[] = {
      {"2011-11-14T16:17:54Z", 2,  190,  100,  "tab",  {1,2,3}},
      { "2011-11-14T16:20:19Z",  2,  190,  100,  "tab", {1,3}},
      { "2011-11-14T16:28:54Z",  1,  300,  200,  "visa", {2,4,5}},
      { "2011-11-14T16:30:43Z",  2,  90,   0,  "tab", {2,3,4}},
      { "2011-11-14T16:48:46Z",  2,  90,   0,  "tab", {1,2,3}},
      { "2011-11-14T16:53:41Z",  2,  90,   0,  "tab", {1,3}},
      { "2011-11-14T16:54:06Z",  1,  100,  0,  "cash", {2,4,5}},
      { "2011-11-14T17:02:03Z",  2,  90,   0,  "tab", {2,3,4}},
      { "2011-11-14T17:07:21Z",  2,  90,   0,  "tab", {1,2,3}},
      { "2011-11-14T17:22:59Z",  2,  90,   0,  "tab", {}},
      { "2011-11-14T17:25:45Z",  2,  200,  0,  "cash", {2,4,5}},
      { "2011-11-14T17:29:52Z",  1,  200,  100,  "visa", {-1, 0, 10, 10}},
      { "2011-11-14T17:33:46Z",  2,  190,  100,  "tab", {1,2,3}},
      { "2011-11-14T17:33:59Z",  2,  90,   0,  "tab", {1,3}},
      { "2011-11-14T17:38:40Z",  2,  200,  100,  "visa", {2,4,5}},
      { "2011-11-14T17:52:02Z",  2,  90,   0,  "tab", {2,3,4}},
      { "2011-11-14T18:02:42Z",  2,  190,  100,  "tab", {1,2,3}},
      { "2011-11-14T18:02:51Z",  2,  190,  100,  "tab", {1,3}},
      { "2011-11-14T18:12:54Z",  1,  200,  100,  "visa", {2,4,5}},
      { "2011-11-14T18:14:53Z",  2,  100,  0,  "cash", {2,3,4}},
      { "2011-11-14T18:45:24Z",  2,  90,   0,  "tab", {2,4,5}},
      { "2011-11-14T19:00:31Z",  2,  190,  100,  "tab", {2,3,4}},
      { "2011-11-14T19:04:22Z",  2,  90,   0,  "tab", {1,2,3}},
      { "2011-11-14T19:30:44Z",  2,  90,   0,  "tab", {1,3}},
      { "2011-11-14T20:06:33Z",  1,  100,  0,  "cash", {2,4,5}},
      { "2011-11-14T20:49:07Z",  2,  290,  200,  "tab", {2,4,5}},
      { "2011-11-14T21:05:36Z",  2,  90,   0,  "tab", {2,3,4}},
      { "2011-11-14T21:18:48Z",  4,  270,  0,  "tab", {1,2,3}},
      { "2011-11-14T21:22:31Z",  1,  200,  100,  "visa", {1,3}},
      { "2011-11-14T21:26:30Z",  2,  190,  100,  "tab", {2,4,5}},
      { "2011-11-14T21:30:55Z",  2,  190,  100,  "tab", {2,3,4}},
      { "2011-11-14T21:31:05Z",  2,  90,   0,  "tab", {1,2,3}},
      { "2011-11-14T22:30:22Z",  2,  89,   0,  "tab", {1,3}},
      { "2011-11-14T22:34:28Z",  2,  190,  100,  "tab", {2,4,5}},
      { "2011-11-14T22:48:05Z",  2,  91,   0,  "tab", {2,4,5}},
      { "2011-11-14T22:51:40Z",  2,  190,  100,  "tab", {2,3,4}},
      { "2011-11-14T22:58:54Z",  2,  100,  0,  "visa", {2,3,4}},
      { "2011-11-14T23:06:25Z",  2,  190,  100,  "tab", {1,2,3}},
      { "2011-11-14T23:07:58Z",  2,  190,  100,  "tab", {1,3}},
      { "2011-11-14T23:16:09Z",  1,  200,  100,  "visa", {2,4,5}},
      { "2011-11-14T23:21:22Z",  2,  190,  100,  "tab", {2,4,5}},
      { "2011-11-14T23:23:29Z",  2,  190,  100,  "tab", {2,3,4}},
      { "2011-11-14T23:28:54Z",  2,  190,  100,  "tab", {1,2,3}}
    };
    cr.add(data);
    date = cr.dimension([](auto r) { return r.date;});
    total = cr.dimension([](auto r) { return r.total;});
    quantity = cr.dimension([](auto r) { return r.quantity;});
    type = cr.dimension([](auto r) { return r.type;});
    tip = cr.dimension([](auto r) { return r.tip;});
  }
};
// BOOST_AUTO_TEST_CASE(groupAll_value_after_removing_all_data) {
//   struct Rec2 {
//     double foo;
//     int bar;
//   };
  
//   CrossFilter<Rec2> data;
//   auto foo = data.dimension([](auto d) { return -d.foo; });
//   auto bar = data.dimension([](auto d) { return d.bar; });
//   auto foos = foo.groupReduceSum([](auto d) { return d.foo; },
//                                  [](auto d) ->int{ return std::floor(d);}
//                                  );
//   bar.filterExact(1);
//   std::vector<Rec2> d(5);
//   for (int i = 0; i < 2; i++) {
//     for(int j = 0; j < 5; j++) {
//       d[j] = Rec2{i+j/10.0, i % 2};
//       std::cout << j + i*10 << ":" << d[j].foo << ',' << d[j].bar << std::endl;
//     }
//     data.add(d);
//   }
//   std::cout << "-=-------------------------------" << std::endl;
  
//   auto t = foo.top(100);
//   for(auto & i : t) {
//     std::cout << i.foo << ',' << i.bar << std::endl;
    
//   }
//   foos.top(1);
  
//   BOOST_TEST(foos.top(1) == (std::vector<std::pair<int,double>> {{ -998, 8977.5}}),boost::test_tools::per_element());
  
// }

// BOOST_AUTO_TEST_CASE(test2) {
//   CrossFilter<int> data;
//   auto foo = data.dimension([](auto d) { return -d; });
//   auto bar = data.dimension([](auto d) { return d % 2 == 1; });
//   bar.filterExact(true);
//   data.add(std::vector<int>{1,2,3,4,5,6});
//   auto foos = foo.groupReduceSum([](auto d) { return d; });
//   //  auto foos = foo.group();
  

//   auto all2 = foos.all2();
//   for(auto a : all2) {
//     std::cout << a.first << ',' << a.second << std::endl;
//   }
//   std::cout << "--------" << std::endl;
  
//   auto all = foos.all();
//   for(auto a : all) {
//     std::cout << a.first << ',' << a.second << std::endl;
//   }

// }

struct RecIterable {
  std::vector<int> foo;
  int bar;
};


BOOST_TEST_DONT_PRINT_LOG_VALUE(RecIterable)

// BOOST_AUTO_TEST_CASE(can_remove_records_while_filtering_on_iterable_dimension) {
//   CrossFilter<RecIterable> data;

//   auto fooDimension = data.iterableDimension([](auto d) { return d.foo;});
//   auto fooGroup = fooDimension.group();
//   //  auto allBarSum = data.groupAllReduceSum([](auto d) { return d.bar;});
//   //  auto fooBarSum = fooDimension.groupReduceSum([](auto d) { return d.bar;});


//   data.add(std::vector<RecIterable>{
//       {std::vector<int>{1,2,3},1},
//       {std::vector<int>{1,2},2},
//       {std::vector<int>{2,3},4}
//     });
//   fooGroup.all();
//   //   allBarSum.value();

//   //   BOOST_CHECK_EQUAL(allBarSum.value(),7);
//   BOOST_TEST(fooGroup.all() == (std::vector<std::pair<int,std::size_t>> {
//         {  1,  2 },
//         {  2,  3 },
//         {  3,  2 }
//       }), boost::test_tools::per_element());
//   // BOOST_TEST(fooBarSum.all() == (std::vector<std::pair<int,int>> {
//   //       {  1,  3 },
//   //       {  2,  7 },
//   //       {  3,  5 }
//   //     }), boost::test_tools::per_element());
//   fooDimension.filter(3);
//   //  BOOST_CHECK_EQUAL(allBarSum.value(), 5);

//   data.remove([] (auto d, int) {
//       return std::find(d.foo.begin(), d.foo.end(), 1) != d.foo.end();

//     });
//    // BOOST_CHECK_EQUAL(allBarSum.value(), 4);
//   BOOST_TEST(fooGroup.all() == (std::vector<std::pair<int,std::size_t>> {
//         {  2,  1 },
//         {  3,  1 }
//       }), boost::test_tools::per_element());
//   // BOOST_TEST(fooBarSum.all() == (std::vector<std::pair<int,int>> {
//   //       {  2,  4 },
//   //       {  3,  4 }
//   //     }), boost::test_tools::per_element());

//   fooDimension.filterAll();
//    // BOOST_CHECK_EQUAL(allBarSum.value(), 4);
//   data.remove([] (auto, int) {
//       return true;
//     });
//   BOOST_TEST(fooDimension.top(Infinity).empty());
  
// }

// BOOST_AUTO_TEST_CASE(group_reduce2_test) {
//   struct Rec2 {
//     int foo;
//     int val;
//   };
  
//   CrossFilter<Rec2> data;
//   data.add(std::vector<Rec2>{{ 1,  2}, { 2,  2}, { 3,  2}, { 3,  2}});
//   auto foo = data.dimension([](auto d) { return d.foo; });
//   auto bar = data.dimension([](auto d) { return d.foo; });
//   auto val = data.dimension([](auto d) { return d.val; });
//   auto groupMax = bar.group([](auto &p,auto v, bool n){
//       if(n) {
//         p += v.val;
//       }
//       return p;
//     },
//     [](auto &p,auto v,bool n) {
//       if(n) {
//         p -= v.val;
//       }
//       return p;
//     },
//     []() {
//       return 0;
//     });
//   auto groupSum = bar.groupReduceSum([](auto d) { return d.val; });

//   // gives reduce functions information on lifecycle of data element
//   // on group creation
//   BOOST_CHECK_EQUAL(groupMax.all(), groupSum.all());

//   // on filtering
//   foo.filterRange(1, 3);
//   {
//     auto groupMaxAll =groupMax.all();
//     auto groupSumAll = groupSum.all();
//     std::vector<std::pair<int,int>> t1{{ 1, 2 }, { 2, 2 }, {  3,  4 }};
//     std::vector<std::pair<int,int>> t2 {{ 1,  2 }, {2, 2 }, { 3,0 }};
    
//     BOOST_CHECK(check_collections_equal(groupMaxAll, t1));
//     BOOST_CHECK(check_collections_equal(groupSumAll,t2));
//   }
  
//   foo.filterAll();

//   // on adding data after group creation
//   data.add({1, 2});
//   BOOST_CHECK(check_collections_equal(groupMax.all(), groupSum.all()));

//   auto t = groupMax.all();
//   std::cout << "groupMax1=";
//   boost::test_tools::tt_detail::print_log_value<std::vector<std::pair<int,int>>>()(std::cout,t);
//   std::cout << std::endl;

//   // on adding data when a filter is in place
//   foo.filterRange(1,3);
//   data.add({ 3, 1});
//   t = groupMax.all();
//   std::cout << "groupMax2=";
//   boost::test_tools::tt_detail::print_log_value<std::vector<std::pair<int,int>>>()(std::cout,t);
//   std::cout << std::endl;

//   BOOST_TEST(t == (std::vector<std::pair<int,int>>{{1,4 }, { 2,2 }, { 3,5 }}), boost::test_tools::per_element());

//   BOOST_TEST(groupSum.all() == (std::vector<std::pair<int,int>>{{ 1,4 }, { 2, 2 }, { 3, 0 }}), boost::test_tools::per_element());

//   foo.filterAll();

//   // on removing data after group creation
//   val.filter(1);
//   data.remove();
//   BOOST_TEST(groupMax.all() == (std::vector<std::pair<int,int>>{{1,4 },{ 2,2 },{ 3,4 }}),boost::test_tools::per_element());
//   BOOST_TEST(groupSum.all() ==  (std::vector<std::pair<int,int>>{{1,0 },{ 2,0 },{ 3,0 }}),boost::test_tools::per_element());

//   val.filterAll();
//   BOOST_TEST(groupMax.all() ==  groupSum.all(),boost::test_tools::per_element());

// }

BOOST_AUTO_TEST_CASE(can_remove_records_while_filtering_on_iterable_dimension) {
  CrossFilter<RecIterable> data;

  auto fooDimension = data.iterableDimension([](auto d) { return d.foo;});
  //  auto fooGroup = fooDimension.group();
  //  auto allBarSum = data.groupAllReduceSum([](auto d) { return d.bar;});
  auto fooBarSum = fooDimension.groupReduceSum([](auto d) { return d.bar;});

  data.add(std::vector<RecIterable>{
      {std::vector<int>{1,2,3},1},
      {std::vector<int>{1,2},2},
      {std::vector<int>{2,3},4}
    });
  //   fooGroup.all();
  //   allBarSum.value();

  //   BOOST_CHECK_EQUAL(allBarSum.value(),7);
  // BOOST_TEST(fooGroup.all() == (std::vector<std::pair<int,std::size_t>> {
  //     {  1,  2 },
  //     {  2,  3 },
  //     {  3,  2 }
  //   }), boost::test_tools::per_element());
  BOOST_TEST(fooBarSum.all() == (std::vector<std::pair<int,int>> {
        {  1,  3 },
        {  2,  7 },
        {  3,  5 }
      }), boost::test_tools::per_element());

  fooDimension.filter(3);
  //  BOOST_CHECK_EQUAL(allBarSum.value(), 5);

  data.remove([] (auto d, int) {
      return std::find(d.foo.begin(), d.foo.end(), 1) != d.foo.end();

    });
  //  BOOST_CHECK_EQUAL(allBarSum.value(), 4);
  // BOOST_TEST(fooGroup.all() == (std::vector<std::pair<int,std::size_t>> {
  //       {  2,  1 },
  //       {  3,  1 }
  //     }), boost::test_tools::per_element());
  BOOST_TEST(fooBarSum.all() == (std::vector<std::pair<int,int>> {
        {  2,  4 },
        {  3,  4 }
      }), boost::test_tools::per_element());

  fooDimension.filterAll();
  //  BOOST_CHECK_EQUAL(allBarSum.value(), 4);
  data.remove([] (auto, int) {
      return true;
    });
  BOOST_TEST(fooDimension.top(Infinity).empty());

 }
