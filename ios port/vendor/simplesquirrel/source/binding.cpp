#include "simplesquirrel/binding.hpp"

namespace ssq {
    namespace detail {
        const std::string Param<bool>::type = "b|n";
        const std::string Param<char>::type = "b|n";
        const std::string Param<signed char>::type = "b|n";
        const std::string Param<short>::type = "b|n";
        const std::string Param<int>::type = "b|n";
        const std::string Param<long>::type = "b|n";
        const std::string Param<unsigned char>::type = "b|n";
        const std::string Param<unsigned short>::type = "b|n";
        const std::string Param<unsigned int>::type = "b|n";
        const std::string Param<unsigned long>::type = "b|n";
#ifdef _SQ64
        const std::string Param<long long>::type = "b|n";
        const std::string Param<unsigned long long>::type = "b|n";
#endif
    }
}
