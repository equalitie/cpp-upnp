#pragma once

#include <functional>
#include <boost/intrusive/list.hpp>

namespace upnp {

class cancel_t {
private:
    template<class K>
    using List = boost::intrusive::list<K, boost::intrusive::constant_time_size<false>>;
    using Hook = boost::intrusive::list_base_hook<boost::intrusive::link_mode<boost::intrusive::auto_unlink>>;

public:
    class Connection : public Hook
    {
    public:
        Connection() = default;

        Connection(Connection&& other)
            : _slot(std::move(other._slot))
            , _call_count(other._call_count)
        {
            other._call_count = 0;
            other.swap_nodes(*this);
        }

        Connection& operator=(Connection&& other) {
            _slot = std::move(other._slot);
            _call_count = other._call_count;
            other._call_count = 0;
            other.swap_nodes(*this);
            return *this;
        }

        size_t call_count() const { return _call_count; }

        operator bool() const { return call_count() != 0; }

    private:
        friend class cancel_t;
        std::function<void()> _slot;
        size_t _call_count = 0;
    };

public:
    cancel_t()                    = default;

    cancel_t(const cancel_t&)            = delete;
    cancel_t& operator=(const cancel_t&) = delete;

    cancel_t(cancel_t& parent)
        : _parent_connection(parent.connect(call_to_self()))
    {}

    cancel_t(cancel_t&& other)
        : _connections(std::move(other._connections))
        , _call_count(other._call_count)
    {
        other._call_count = 0;

        if (other._parent_connection._slot) {
            _parent_connection = std::move(other._parent_connection);
            _parent_connection._slot = call_to_self();
        }
    }

    cancel_t& operator=(cancel_t&& other)
    {
        _connections = std::move(other._connections);
        _call_count = other._call_count;
        other._call_count = 0;

        if (other._parent_connection._slot) {
            _parent_connection = std::move(other._parent_connection);
            _parent_connection._slot = call_to_self();
        }

        return *this;
    }

    void operator()()
    {
        ++_call_count;

        auto connections = std::move(_connections);
        for (auto& connection : connections) {
            try {
                ++connection._call_count;
                connection._slot();
            } catch (std::exception& e) {
                assert(0);
            }
        }
    }

    size_t call_count() const { return _call_count; }

    operator bool() const { return call_count() != 0; }

    Connection connect(std::function<void()> slot)
    {
        Connection connection;
        connection._slot = std::move(slot);
        _connections.push_back(connection);
        return connection;
    }

    size_t size() const { return _connections.size(); }

private:
    std::function<void()> call_to_self() {
        return [&]() { (*this)(); };
    }

private:
    List<Connection> _connections;
    size_t _call_count = 0;
    Connection _parent_connection;
};

} // namespace
