#pragma once

#include <boost/asio/error.hpp>
#include <boost/asio/post.hpp>
#include <boost/range/begin.hpp> // needed by spawn
#include <boost/range/end.hpp> // needed by spawn
#include <boost/asio/spawn.hpp>
#include <boost/intrusive/list.hpp>
#include <upnp/cancel.h>
#include <upnp/namespaces.h>

namespace upnp {

class ConditionVariable {
    using Sig = void(boost::system::error_code);

    using IntrusiveHook = boost::intrusive::list_base_hook
        <boost::intrusive::link_mode
            <boost::intrusive::auto_unlink>>;

    struct WaitEntry : IntrusiveHook {
        std::function<Sig> handler;
    };

    using IntrusiveList = boost::intrusive::list
        <WaitEntry, boost::intrusive::constant_time_size<false>>;

public:
    ConditionVariable(const net::executor&);

    ConditionVariable(const ConditionVariable&) = delete;
    ConditionVariable& operator=(const ConditionVariable&) = delete;

    ~ConditionVariable();

    net::executor get_executor() { return _exec; }

    void notify(const boost::system::error_code& ec
                    = boost::system::error_code());

    void wait(cancel_t&, net::yield_context yield);
    void wait(net::yield_context yield);

private:
    net::executor _exec;
    IntrusiveList _on_notify;
};

inline
ConditionVariable::ConditionVariable(const net::executor& exec)
    : _exec(exec)
{
}

inline
ConditionVariable::~ConditionVariable()
{
    notify(net::error::operation_aborted);
}

inline
void ConditionVariable::notify(const boost::system::error_code& ec)
{
    while (!_on_notify.empty()) {
        auto& e = _on_notify.front();
        net::post(_exec, [h = std::move(e.handler), ec] () mutable { h(ec); });
        _on_notify.pop_front();
    }
}

inline
void ConditionVariable::wait(cancel_t& cancel, net::yield_context yield)
{
    auto work = net::make_work_guard(_exec);

    WaitEntry entry;

    net::async_completion<net::yield_context, Sig> init(yield);
    entry.handler = std::move(init.completion_handler);
    _on_notify.push_back(entry);

    auto slot = cancel.connect([&] {
        assert(entry.is_linked());
        if (entry.is_linked()) entry.unlink();

        net::post(_exec, [h = std::move(entry.handler)] () mutable {
            h(net::error::operation_aborted);
        });
    });

    return init.result.get();
}

inline
void ConditionVariable::wait(boost::asio::yield_context yield)
{
    cancel_t dummy_cancel;
    wait(dummy_cancel, yield);
}

} // ouinet namespace
