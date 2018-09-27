#include "../index.hpp"
#include <vector>
#include <iostream>
#include <ThorSerialize/Traits.h>

struct Record
{
    std::string date;
    int quantity;
    int total;
    int tip;
    std::string type;
    std::vector<std::string> productIDS;
};

int main()
{

    std::vector<Record> data = {
        {"2011-11-14T16:17:54Z", 2, 190, 100, "tab", {"001", "002"}},
        {"2011-11-14T16:20:19Z", 2, 190, 100, "tab", {"001", "005"}},
        {"2011-11-14T16:28:54Z", 1, 300, 200, "visa", {"004", "005"}},
        {"2011-11-14T16:30:43Z", 2, 90, 1, "tab", {"001", "002"}},
        {"2011-11-14T16:48:46Z", 2, 90, 2, "tab", {"005"}},
        {"2011-11-14T16:53:41Z", 2, 90, 3, "tab", {"001", "004", "005"}},
        {"2011-11-14T16:54:06Z", 1, 100, 4, "cash", {"001", "002", "003", "004", "005"}},
        {"2011-11-14T16:58:03Z", 2, 90, 5, "tab", {"001"}},
        {"2011-11-14T17:07:21Z", 2, 90, 6, "tab", {"004", "005"}},
        {"2011-11-14T17:22:59Z", 2, 90, 7, "tab", {"001", "002", "004", "005"}}};
    cross::filter<Record> payments(data);

    // build a dimension of "total" entries in the data record
    auto totals = payments.dimension([](auto r) { return r.total; });

auto top_totals = totals.top(3);

for (size_t i=0;i<top_totals.size();++i){
    std::cout << top_totals[i].total << ", ";
}
std::cout << std::endl;

    // extract features based on dimension "totals"
    auto total_count = totals.feature_count();
    auto total_sum = totals.feature_sum([](auto r) { return r.total; });

    // // same for "tips"
        auto tips = payments.dimension([](auto r) { return r.tip; });
        auto tips_count = tips.feature_count();
        auto tips_sum = tips.feature_sum([](auto r) { return r.tip; });

    // plot features
    std::cout << "total_count: " << total_count.value() << ",  total_sum: " << total_sum.value() << std::endl;

    // filter dimension by "total" values between 100 <= x < 300
    totals.filter_range(100, 300);

    // plot features again
    std::cout << "total_count: " << total_count.value() << ",  total_sum: " << total_sum.value() << std::endl;

    tips.filter_range(2, 6);

    // std::cout << "total_count: " << total_count.value() << ",  total_sum: " << total_sum.value() << std::endl;

    // totals.filter_all();

    // std::cout << "total_count: " << total_count.value() << ",  total_sum: " << total_sum.value() << std::endl;

    auto add_f = [](auto r, auto, bool ) { return r + 1 ; };
    auto remove_f = [](auto r, auto, bool ) { return r - 1 ; };
    auto initial_f = []() { return 0; };


    auto total_custom_count = totals.feature(add_f, remove_f, initial_f);

    //   auto addGroup = data.type.feature(
    //   [](auto p, auto, bool ) {
    //     ++p;
    //     return p;
    //   },
    //   [](auto p, auto,bool) {
    //     return p;
    //   },
    //   []() {
    //     return std::size_t(0);
    //   }
    //                                    );

    auto top_totals2 = totals.top(3);

for (size_t i=0;i<top_totals2.size();++i){
    std::cout << top_totals2[i].total << ", ";
}
std::cout << std::endl;

    // std::cout << "total custom count: " << total_custom_count.value() << std::endl;

    // auto minfilter = [] (auto r) { return (r < 80); };

    // totals.filter(minfilter) ;

    // std::cout << "total_count: " << total_count.value() << ",  total_sum: " << total_sum.value() << std::endl;
    

    return 0;
}