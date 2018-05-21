#include <chrono>
#include <functional>
#include <cstdlib>
#include <random>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>
#include "crossfilter.hpp"

struct Payment {
  Payment(const uint64_t & d, double a)
      :date(d), amount(a) {}
  Payment() {}
  uint64_t date;
  double amount;
};
double drandom(int w) {
  auto r = std::rand();
  auto r1 = r/(double)RAND_MAX;
  auto r2 = w*r1;
  auto r3 = std::floor(r2);

  return r3;
}
uint64_t mtime() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}
struct RandomIndex {
  std::vector<double> distr;
  std::random_device rd;
  std::mt19937 gen;
  std::discrete_distribution<> d;
  template<typename I>
  RandomIndex(I begin, I end)
      :distr(begin,end),rd(),gen(rd()),d(distr.begin(),distr.end()) {}
  int operator()() {
    return std::abs(d(gen));
  }
};

uint64_t epoch(const boost::posix_time::ptime & t) {
  boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00.000");
  boost::posix_time::time_duration const diff = t - epoch;
  return diff.total_seconds();
}
auto randomRecentDate(RandomIndex & randomDayOfWeek, RandomIndex &randomHourOfDay, int weeks){
  auto result = [&randomDayOfWeek, &randomHourOfDay, weeks]() {
    auto now = boost::posix_time::ptime(boost::posix_time::second_clock::local_time());
    int day_offset = int(drandom(weeks*7));

    now = now  -  boost::gregorian::days(day_offset);
    now += boost::gregorian::days(randomDayOfWeek());

    return  epoch(boost::posix_time::ptime(now.date(),
                                       boost::posix_time::hours(randomHourOfDay()) +
                                       boost::posix_time::minutes(std::rand()%60) +
                                           boost::posix_time::seconds(std::rand()%60)));
  };
  return result;
}

struct RandomLogNormal {
  std::random_device rd;
  std::mt19937 gen;
  std::normal_distribution<> d;
  RandomLogNormal(double mean, double stddev)
      :gen(rd()),d(mean,stddev) {}
  double operator()()  {
    return std::abs(d(gen));
  }
};

int makedate(uint64_t d) {
  auto pt = boost::posix_time::from_time_t(d);
  auto ymd = pt.date().year_month_day();
  //  20181120
  return ymd.year*10000 + ymd.month*100 + ymd.day;
}
int makedayofweek(uint64_t d) {
  auto pt = boost::posix_time::from_time_t(d);
  return pt.date().day_of_week().as_number();
}
int makehours(uint64_t d) {
  auto pt = boost::posix_time::from_time_t(d);
  return pt.time_of_day().hours();
}




int main() {
  uint64_t firstSize = 90000;
  uint64_t secondSize = 10000;
  auto totalSize = firstSize + secondSize;
  double v1[] = {0, .6, .7, .75, .8, .76, 0};
  double v2[] = {0, 0, 0, 0, 0, 0, 0, .2, .5, .7, .85, .9, .8, .69, .72, .8, .78, .7, .3, 0, 0, 0, 0, 0};
  std::random_device rd;
  std::mt19937 gen(rd());
  
  RandomIndex randomDayOfWeek(std::begin(v1), std::end(v1));
  RandomIndex  randomHourOfDay(std::begin(v2), std::end(v2));
  auto randomDate  = randomRecentDate(randomDayOfWeek, randomHourOfDay, 13);
  RandomLogNormal randomAmount(2.5,5);
  auto then = mtime();

  std::vector<Payment> firstBatch;
  firstBatch.reserve(firstSize);

  std::vector<Payment> secondBatch;
  secondBatch.reserve(secondSize);

  for(std::size_t i = 0; i < firstSize; i++) {
    firstBatch.emplace_back(randomDate(),randomAmount());
  }
  for(std::size_t i = 0; i < secondSize; i++) {
    secondBatch.emplace_back(randomDate(),randomAmount());
  }

  std::cout << "Synthesizing " << totalSize << " records:"  << (mtime() - then) << "ms." << std::endl;

  then = mtime();

  CrossFilter<Payment> payments(firstBatch);
   auto all = payments.groupAll();
  auto amount = payments.dimension([](auto d)  {return d.amount;});
  auto amounts = amount.groupReduceCount([](auto d) 
                               {
                                 return std::floor(d);
                               });
  auto date = payments.dimension([](auto d)                                  {
                                   return d.date;
                                 });
  auto dates = date.group([](auto d) 
                          {
                            return makedate(d);
                          });
  auto day = payments.dimension([](auto d) 
                                {
                                  return makedayofweek(d.date);//d.date.date().day_of_week().as_number();
                                });
   auto days = day.group();
  auto hour = payments.dimension([](auto d) 
                                 {
                                   return makehours(d.date);// d.date.time_of_day().hours();
                                 });
   auto hours = hour.group();

  std::cout << "Indexing " << firstSize << " records: " << (mtime() - then) << "ms." << std::endl;
  auto then3 = mtime();
  payments.add(secondBatch);

  std::cout << "Indexing " << secondSize << " records: " << (mtime() - then3) << "ms." << std::endl;
  std::cout << "Total indexing time: " << (mtime() - then) << "ms." << std::endl;

   std::cout << "dates.size()=" << dates.size() << std::endl;
  std::cout << "days.size()=" << days.size() << std::endl;
  std::cout << "hours.size()=" << hours.size() << std::endl;
  std::cout << "amounts.size()=" << amounts.size() << std::endl;
  then = mtime();
    auto today = boost::posix_time::second_clock::local_time();
  int k = 0;

  for(int i = 0; i < 90; ++i) {
    auto ti = epoch(today - boost::gregorian::days(i));
    for(int j = 0; j < i; j++) {
      auto tj = epoch(today - boost::gregorian::days(j));
      date.filterRange(ti,tj);
      k++;
      dates.all();
      days.all();
      hours.all();
      amounts.all();
      all.value();
      date.top(40).size();
    }

  }
  std::cout << "Filtering by date: " << ((mtime() - then) / double(k)) << "ms/op." << std::endl;
  date.filterAll();

  then = mtime();
  k = 0;
  //  for(int l = 0; l < 100; l++) {
  for(int i = 0; i < 7; i++) {
    for(int j = i; j < 7; j++) {
      day.filterRange(i,j);
      k++;
      dates.all();
      days.all();
      hours.all();
      amounts.all();
      all.value();
      date.top(40);
    }
  }
  //  }
  
  std::cout << "Filtering by day: " << ((mtime() - then) / double(k)) << "ms/op." << std::endl;
  day.filterAll();

  then = mtime();
  k = 0;

  for(int i = 0; i < 24; i++) {
    for(int j = i; j < 24; j++) {
      hour.filterRange(i,j);
      k++;
      dates.all();
      days.all();
      hours.all();
      amounts.all();
      all.value();
      date.top(40);
    }
  }
  std::cout << "Filtering by hour: " << ((mtime() - then) / double(k)) << "ms/op." << std::endl;
  hour.filterAll();

  then = mtime();
  k = 0;
  for(int i = 0; i < 35; i++) {
    for(int j = i; j < 35; j++) {
      amount.filterRange(i,j);
      k++;
      dates.all();
      days.all();
      hours.all();
      amounts.all();
      all.value();
      date.top(40);
    }
  }
  std::cout << "Filtering by amount: " << ((mtime() - then) / double(k)) << "ms/op." << std::endl;
  amount.filterAll();

  then = mtime();
  payments.remove([](auto, int i)
                  {
                    return i%10 == 1;
                  });

  std::cout << "Removing  " << totalSize/10 << " records: " <<  (mtime() - then) << "ms." << std::endl;

  std::cout << std::endl;
  std::cout << "Day of Week:" << std::endl;
  for(auto & d : days.all()) {
    std::cout << "       " << d.first << ": " << d.second << std::endl;
  }
  std::cout << std::endl;
  std::cout << "Hour of Day:" << std::endl;
  for(auto & h : hours.all()) {
    std::cout << "       " << h.first << ": " << h.second << std::endl;
  }

  std::cout << std::endl;
  std::cout << "Date:" << std::endl;
  for(auto & h : dates.all()) {
    std::cout << "       " << h.first << ": " << h.second << std::endl;
  }
  std::cout << std::endl;
  std::cout << "Amount:" << std::endl;
  for(auto & h : amounts.all()) {
    std::cout << "       " << int(h.first) << ": " << h.second << std::endl;
  }

  return 0;
}
