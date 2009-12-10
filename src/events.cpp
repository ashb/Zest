
#include "events.hpp"
#include "server.hpp"

#include <flusspferd/create_on.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>

using namespace flusspferd;
using namespace zest;

using namespace boost::posix_time;
namespace phoenix = boost::phoenix;

namespace zest {

void setup_event_loop(object exports, object require) {

  load_class<event_loop>(exports);

  create<event_loop>(
    param::_container = exports,
    param::_name = "asio"
  );

  // TODO: Setup a preload for 'event' module
}

} // namespace zest

boost::asio::io_service svc;

// JS ctor
event_loop::event_loop(object const &proto, call_context &x)
  : base_type(proto),
    //io_service_ptr_(new boost::asio::io_service()),
    timer_counter_(0)
{ }

// C++ ctor
event_loop::event_loop(object const &proto)
  : base_type(proto),
    //io_service_ptr_(get_default_io_service()),
    io_service_ptr_(new boost::asio::io_service()),
    timer_counter_(0)
{ }

void event_loop::clear_timer(event_loop::timer *t) {
  std::cout << "clear_timer\n";
  t->cancel();
}

event_loop::~event_loop()
{
  std::cout << "~event_loop\n";
}

// The Default io_service to share between things
shared_io_service event_loop::get_default_io_service() {
  static boost::weak_ptr<boost::asio::io_service> svc;

  shared_io_service ptr = svc.lock();
  if (!ptr) {
    // Not created or gone out of scope. Re-create
    std::cout << "creating shared_io_service\n";
    ptr = shared_io_service(new boost::asio::io_service());
    svc = ptr;
  }

  return ptr;
}

int event_loop::run() {
  return io_service_ptr_->run();
}

int event_loop::run_one() {
  return io_service_ptr_->run_one();
}

int event_loop::poll() {
  return io_service_ptr_->poll();
}

int event_loop::poll_one() {
  return io_service_ptr_->poll_one();
}

void event_loop::reset() {
  return io_service_ptr_->reset();
}

// func[, delay[, arg...]] -> Number
void event_loop::set_timeout(call_context &x) {
  if (x.arg.size() < 1) {
    // 0 is a special timerId for us. it means we did nothing!
    x.result = 0;
    return;
  }

  time_duration delay;
  arguments args;

  arguments::iterator iter = x.arg.begin();
  value v = *iter++;

  if (!v.is_object())
    throw exception("callback is not a function", "TypeError");

  // TODO: either catch the error or make an is_callable() function
  root_object cb(function(v.get_object()));

  if (iter != x.arg.end()) {
    v = *iter++;

    if (!v.is_undefined_or_null());
      delay = milliseconds(v.to_integral_number(63, false));

    for (; iter != x.arg.end(); ++iter) {
      args.push_root(*iter);
    }
  }

  boost::shared_ptr<timer> t(new timer(*io_service_ptr_, delay, ++timer_counter_, cb, args));
  timers_.insert(*t);
  t->async_wait(boost::bind(&event_loop::on_timer, this, boost::asio::placeholders::error, t));
  x.result = t->id;
}

void event_loop::on_timer(boost::system::error_code const &e, boost::shared_ptr<timer> t) {
  if (e == boost::asio::error::operation_aborted) {
    return;
  }

  try {
    t->cb.call(t->args);
  }
  catch (...) {
  }
  timers_.erase(*t);
}

event_loop::timer::timer(boost::asio::io_service &svc, duration_type td, 
        int id_, flusspferd::object &cb_, flusspferd::arguments &args_
) : boost::asio::deadline_timer(svc, td),
    id(id_),
    cb(cb_),
    args(args_)
{ }


event_loop::timer::~timer()
{ }

