#pragma once

#include <functional>
#include <iostream>
#include <stdexcept>

#include "blackhole/config.hpp"
#include "blackhole/detail/util/meta.hpp"

namespace blackhole {

namespace log {

typedef std::function<void()> exception_handler_t;

class default_exception_handler_t {
public:
    void operator()() const {
        try {
            throw;
        } catch (const std::exception& err) {
            std::cout << "logging core error occurred: " << err.what() << std::endl;
        } catch (...) {
            std::cout << "logging core error occurred: unknown" << std::endl;
        }
        //First we make a message about core error and then rethrow in debug
#ifdef BLACKHOLE_DEBUG
        throw;
#endif
    }
};

namespace aux {

template<typename Handler>
class launcher {
protected:
    typedef Handler handler_type;
    typedef void exception_type;

    handler_type handler;

public:
    launcher(const handler_type& handler) :
        handler(handler)
    {}

    void operator()() {
        throw;
    }
};

template<typename Exception, typename Base>
class intermediate : public Base {
protected:
    typedef typename Base::handler_type handler_type;
    typedef Exception exception_type;

public:
    intermediate(const handler_type& handler) :
        Base(handler)
    {
        static_assert(
            (!std::is_base_of<typename Base::exception_type, exception_type>::value),
            "can't build correct exception handling hierarchy"
        );
    }

    void operator()() {
        try {
            Base::operator()();
        } catch (const Exception& err) {
            this->handler(err);
        }
    }
};

template<typename TypeList, typename Handler>
struct handler_hierarchy {
    typedef typename boost::mpl::fold<
        TypeList,
        aux::launcher<Handler>,
        boost::mpl::bind<
            boost::mpl::quote2<aux::intermediate>,
            boost::mpl::_2,
            boost::mpl::_1
        >
    >::type type;
};

} // namespace aux

template<typename TypeList, typename Handler>
class exception_handler : public aux::handler_hierarchy<TypeList, Handler>::type {
    typedef typename aux::handler_hierarchy<TypeList, Handler>::type base_type;

public:
    exception_handler(Handler handler) :
        base_type(handler)
    {}

    void operator()() {
        base_type::operator()();
    }
};

namespace aux {

template<typename Handler, typename... Args>
struct handler_maker {
    typedef exception_handler<
        typename boost::mpl::reverse<
            typename blackhole::aux::util::vector::from_variadic<Args...>::type
        >::type,
        Handler
    > type;
};

} // namespace aux

namespace exception {

template<typename Handler>
struct handler_factory_t {
    template<typename... Args>
    static
    typename aux::handler_maker<Handler, Args...>::type
    make() {
        typedef typename aux::handler_maker<Handler, Args...>::type handler_type;
        return handler_type(Handler());
    }
};

} // namespace exception

} // namespace log

} // namespace blackhole
