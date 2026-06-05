#include "simplesquirrel/exceptions.hpp"

namespace ssq {
    void Exception::generate_message(const std::string& msg) {
        std::ostringstream out;
        out << "Squirrel exception: " << msg << " (";

        const char* lasterr;
        if (vm) {
            sq_getlasterror(vm);
            if (sq_gettype(vm, -1) != OT_STRING)
              lasterr = "no detailed info";
            else
              sq_getstring(vm, -1, &lasterr);
            sq_pop(vm, 1);
        }
        else {
            lasterr = "no detailed info";
        }
        out << lasterr << ")";

        message = out.str();
    }
}
