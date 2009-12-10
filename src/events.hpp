/*
 * Copyright (c) 2009 Ash Berlin
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef zest_events_hhp
#define zest_events_hhp

#include "server.hpp"

#include <flusspferd/class_description.hpp>
#include <boost/system/error_code.hpp>
#include <boost/intrusive/treap_set.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace zest {


// Setup setTimeout et al., default asio instace and what have you
void setup_event_loop(flusspferd::object exports, flusspferd::object require);


FLUSSPFERD_CLASS_DESCRIPTION(
  event_loop,
    (full_name, "zest.ASIO")
    (constructor_name, "ASIO")
    (methods,
      ("run", bind, run)
      ("run_one", bind, run_one)
      ("poll", bind, poll)
      ("poll_one", bind, poll_one)

      ("setTimeout", bind, set_timeout)
      ("clearTimeout", bind, clear_timeout)
    )
  )
{
protected:
  struct timer : public boost::asio::deadline_timer,
                 public boost::intrusive::bs_set_base_hook<>
  {
    int id;
    flusspferd::root_object &cb;
    flusspferd::arguments &args;

    // Bit of anbomination, but we keep ourselves alive!
    boost::shared_ptr<timer> life_guard_;


    explicit timer(boost::asio::io_service &svc,
                   boost::asio::deadline_timer::duration_type td, 
                   int id,
                   flusspferd::root_object &cb, flusspferd::arguments &args);

    friend bool priority_order(timer const &a, timer const &b) {
      return a.expires_at() < b.expires_at();
    }
    friend bool operator< (timer const &a, timer const &b) {
      return a.id < b.id;
    }

    ~timer();
  };


  shared_io_service io_service_ptr_;

  typedef boost::intrusive::treap_set<timer> timer_treap;
  timer_treap timers_;
  int timer_counter_;

  void on_timer(boost::system::error_code const &e, boost::shared_ptr<timer> t);
  void simple_timeout();

public:
  event_loop(flusspferd::object const &o, flusspferd::call_context &x);
  event_loop(flusspferd::object const &o);
  virtual ~event_loop();

  int run();
  int run_one();
  int poll();
  int poll_one();

  void set_timeout(flusspferd::call_context &x);
  void clear_timeout(int timer_id) {}


  // Used in default ctor forms for Zest et al.
  static shared_io_service get_default_io_service();
  
};

} // namespace zest

#endif

