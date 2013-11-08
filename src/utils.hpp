#ifndef UTILS_HPP
#define UTILS_HPP

namespace detail 
{
    template<typename T>
    class scope_exit
    {
    public:         
        explicit scope_exit(T&& exitScope) : exitScope_(std::forward<T>(exitScope)){}
        ~scope_exit(){try{exitScope_();}catch(...){}}
    private:
        T exitScope_;
    };          

    template <typename T>
    scope_exit<T> create_scope_exit(T&& exitScope)
    {
        return scope_exit<T>(std::forward<T>(exitScope));
    }
}

#define _UTILS_EXIT_SCOPE_LINENAME_CAT(name, line) name##line
#define _UTILS_EXIT_SCOPE_LINENAME(name, line) _UTILS_EXIT_SCOPE_LINENAME_CAT(name, line)
#define UTILS_SCOPE_EXIT(f) const auto& _UTILS_EXIT_SCOPE_LINENAME(EXIT, __LINE__) = ::detail::create_scope_exit(f)

#endif
