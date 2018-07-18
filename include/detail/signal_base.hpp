/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2018 Dmitry Vinokurov */

#ifndef SIGNAL_BASE_H_GUARD
#define  SIGNAL_BASE_H_GUARD
#define USE_NOD_SIGNALS = 1

#if defined  USE_NOD_SIGNALS
#include "3dparty/nod.hpp"
namespace signals {
template <typename F> using signal = nod::signal<F>;
using connection = nod::connection;
}
#else
#include <boost/signals2.hpp>
namespace signals {
template <typename F> using signal = boost::signals2::signal<F>;
using connection = boost::signals2::connection;
}
#endif

#include <memory>
template<typename Signal, typename Connection>
struct MovableSignal {
  std::unique_ptr<Signal> signal;
  MovableSignal()
      :signal(std::make_unique<Signal>()) {}

  MovableSignal(MovableSignal && s)
      :signal(std::move(s.signal)) {}

  MovableSignal & operator = (MovableSignal && s) {
    if(&s == this)
      return *this;
    std::swap(signal,s.signal);
    return *this;
  }

  template<typename F>
  Connection connect(const F & f) {
    return signal->connect(f);
  }
  template<typename ...Args>
  void operator()(Args&&... args) {
    signal->operator()(args...);
    
  }
  int num_slots() {
#if defined USE_NOD_SIGNALS
    return signal->slot_count();
#else
    return signal->num_slots();
#endif

  }
};

#endif
