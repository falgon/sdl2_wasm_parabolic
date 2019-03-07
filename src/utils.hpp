#ifndef INCLUDED_PBM_UTILS_HPP
#define INCLUDED_PBM_UTILS_HPP
#include <cmath>
#ifndef NDEBUG
#   include <iostream>
#endif
#include <type_traits>
#include <utility>

namespace pbm {

template <class F, std::enable_if_t<std::is_same_v<std::invoke_result_t<std::decay_t<F>>, const char*>, std::nullptr_t> = nullptr>
inline void debug_console(const char* 
#ifndef NDEBUG
        cname
#endif
        , const char*
#ifndef NDEBUG
        fname
#endif
        , F&&
#ifndef NDEBUG
        fn
#endif
        )
{
#ifndef NDEBUG
    std::cerr << cname << "::" << fname << "(): " << std::forward<F>(fn)() << std::endl;
#endif
}

} // namespace pbm

#endif

