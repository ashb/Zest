
#include "reactor.hpp"

#include <flusspferd/create_on.hpp>
#include <flusspferd/tracer.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>

#include <iostream>

using namespace flusspferd;
using namespace zest;

using namespace boost::posix_time;
using boost::phoenix::val;
namespace phoenix = boost::phoenix;
namespace args = boost::phoenix::arg_names;

namespace zest {

void setup_reactor(object exports, object require) {

  load_class<reactor>(exports);

  create<reactor>(
    param::_container = exports,
    param::_name = "reactor"
  );

  // TODO: Setup a preload for 'event' or 'reactor' module
}

} // namespace zest

boost::asio::io_service svc;

// JS ctor
reactor::reactor(object const &proto, call_context &x)
  : base_type(proto),
    io_service_ptr_(new boost::asio::io_service()),
    timer_counter_(0)
{ }

// C++ ctor
reactor::reactor(object const &proto)
  : base_type(proto),
    io_service_ptr_(get_default_io_service()),
    timer_counter_(0)
{ }

reactor::~reactor()
{ }

void reactor::trace(tracer &trc) {

  BOOST_FOREACH(timer &t, timers_) {
    trc("timer.cb", t.cb);
    for( arguments::iterator i = t.args.begin(), e = t.args.end(); i != e; ++i) {
      trc("timer.args[]", *i);
    }
  }
}

// The Default io_service to share between things
shared_io_service reactor::get_default_io_service() {
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

int reactor::run() {
  return io_service_ptr_->run();
}

int reactor::run_one() {
  return io_service_ptr_->run_one();
}

int reactor::poll() {
  return io_service_ptr_->poll();
}

int reactor::poll_one() {
  return io_service_ptr_->poll_one();
}

void reactor::reset() {
  return io_service_ptr_->reset();
}

void reactor::setup_timer(call_context &x, bool repeat) {
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
      args.push_back(*iter);
    }
  }

  boost::shared_ptr<timer> t(new timer(*io_service_ptr_, delay, ++timer_counter_, cb, args, repeat));
  timers_.insert(*t);

  t->bind(this);

  x.result = t->id;
}

void reactor::clear_timeout(int id) {
  // Hmmm how the hell do you use the key comparison form!
  //boost::function<void (timer*)> disposer(bind(&timer::cancel, args::arg1));
  //timers_.erase_and_dispose(id, timer_treap::key_compare(), disposer);

  for(timer_treap::iterator i = timers_.top(); i != timers_.end(); ++i) {
    if (i->id == id) {
      timers_.erase(i);
      i->cancel();
      return;
    }
  }
}

void reactor::on_timer(boost::system::error_code const &e, boost::shared_ptr<timer> t) {
  if (e == boost::asio::error::operation_aborted) {
    return;
  }

  // Erase (and re-insert if we need to so the heap is right)
  timers_.erase(*t);

  // In browsers (or at least Safari) setInterval is based on when it last
  // fired, not when it last compelted.
  if (t->repeat != timer::zero_duration) {
    t->expires_from_now(t->repeat);
    t->bind(this);
    timers_.insert(*t);
  }

  try {
    t->cb.call(t->args);
  }
  catch (...) {
  }
}

/* static */
boost::asio::deadline_timer::duration_type reactor::timer::zero_duration;

reactor::timer::timer(boost::asio::io_service &svc, duration_type td, 
        int id_, flusspferd::object &cb_, flusspferd::arguments &args_, bool r
) : boost::asio::deadline_timer(svc, td),
    id(id_),
    cb(cb_),
    args(args_),
    repeat(r ? td : duration_type())
{ }

void reactor::timer::bind(reactor *loop) {

  boost::shared_ptr<reactor::timer> t = shared_from_this();
  boost::function<void( boost::system::error_code const &)> f(
    phoenix::bind(&reactor::on_timer, loop, args::arg1, val(t))
  );
  async_wait(f);
}

