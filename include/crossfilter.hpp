/*This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.


Signal Empowering Technology presents

 ██████╗██████╗  ██████╗ ███████╗███████╗███████╗██╗██╗  ████████╗███████╗██████╗ 
██╔════╝██╔══██╗██╔═══██╗██╔════╝██╔════╝██╔════╝██║██║  ╚══██╔══╝██╔════╝██╔══██╗
██║     ██████╔╝██║   ██║███████╗███████╗█████╗  ██║██║     ██║   █████╗  ██████╔╝
██║     ██╔══██╗██║   ██║╚════██║╚════██║██╔══╝  ██║██║     ██║   ██╔══╝  ██╔══██╗
╚██████╗██║  ██║╚██████╔╝███████║███████║██║     ██║███████╗██║   ███████╗██║  ██║
 ╚═════╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚══════╝╚═╝     ╚═╝╚══════╝╚═╝   ╚══════╝╚═╝  ╚═╝                                                                                
                                            Copyright (c) 2018 Dmitry Vinokurov <>

                                                                                             

a C++14 port of the famous crossfilter.js lib.


*/

#ifndef CROSSFILTER_H_GUARD
#define CROSSFILTER_H_GUARD

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


#include "detail/crossfilter.hpp"
#include "detail/dimension.hpp"
#include "detail/group.hpp"

#endif
