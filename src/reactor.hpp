/*
 * Copyright (c) 2009 Ash Berlin
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef zest_reactor_hpp
#define zest_reactor_hpp

#include "types.hpp"

#include <flusspferd/class_description.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/intrusive/treap_set.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace zest {

FLUSSPFERD_CLASS_DESCRIPTION(
  reactor,
    (full_name, "zest.Reactor")
    (constructor_name, "Reactor")
    (methods,
      ("run", bind, run)
      ("run_one", bind, run_one)
      ("poll", bind, poll)
      ("poll_one", bind, poll_one)
      ("reset", bind, reset)

      ("setTimeout", bind, set_timeout)
      ("setInterval", bind, set_interval)
      ("clearTimeout", bind, clear_timeout)
      ("clearInterval", alias, "clearTimeout")
    )
  )
{
protected:
  struct timer : public boost::asio::deadline_timer,
                 public boost::intrusive::bs_set_base_hook<>,
                 public boost::enable_shared_from_this<timer>
  {
    int id;
    flusspferd::object cb;
    flusspferd::arguments args;
    static boost::asio::deadline_timer::duration_type zero_duration;
    boost::asio::deadline_timer::duration_type repeat;

    explicit timer(boost::asio::io_service &svc,
                   boost::asio::deadline_timer::duration_type td, int id,
                   flusspferd::object &cb, flusspferd::arguments &args,
                   bool repeat=false);

    friend bool priority_order(timer const &a, timer const &b) {
      return a.expires_at() < b.expires_at();
    }
    friend bool operator< (timer const &a, timer const &b) {
      return a.id < b.id;
    }

    void bind(reactor *);

  };

  shared_io_service io_service_ptr_;

  // treap - not quite a tree, not quite a heap
  typedef boost::intrusive::treap_set<timer> timer_treap;
  timer_treap timers_;
  int timer_counter_;

  void on_timer(boost::system::error_code const &e, boost::shared_ptr<timer> t);

  void trace(flusspferd::tracer &trc);

  void setup_timer(flusspferd::call_context &x, bool repeat);
public:
  reactor(flusspferd::object const &o, flusspferd::call_context &x);
  reactor(flusspferd::object const &o);
  virtual ~reactor();

  int run();
  int run_one();
  int poll();
  int poll_one();

  void reset();

  void set_timeout(flusspferd::call_context &x) { setup_timer(x, false); }
  void set_interval(flusspferd::call_context &x) { setup_timer(x, true); }

  void clear_timeout(int timer_id);

  // Used in default ctor forms for Zest et al.
  static shared_io_service get_default_io_service();
  
};

} // namespace zest

#endif

