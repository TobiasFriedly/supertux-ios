#include "simplesquirrel/object.hpp"
#include "simplesquirrel/args.hpp"
#include "simplesquirrel/class.hpp"
#include "simplesquirrel/instance.hpp"
#include "simplesquirrel/table.hpp"
#include "simplesquirrel/function.hpp"
#include "simplesquirrel/enum.hpp"
#include "simplesquirrel/array.hpp"
#include <squirrel.h>

namespace ssq {
    namespace detail {
        void pushRaw(HSQUIRRELVM vm, const Object& value) {
            sq_pushobject(vm, value.getRaw());
        }
        void pushRaw(HSQUIRRELVM vm, const Class& value) {
            sq_pushobject(vm, value.getRaw());
        }
        void pushRaw(HSQUIRRELVM vm, const Instance& value) {
            sq_pushobject(vm, value.getRaw());
        }
        void pushRaw(HSQUIRRELVM vm, const Table& value) {
            sq_pushobject(vm, value.getRaw());
        }
        void pushRaw(HSQUIRRELVM vm, const Function& value) {
            sq_pushobject(vm, value.getRaw());
        }
        void pushRaw(HSQUIRRELVM vm, const Enum& value) {
            sq_pushobject(vm, value.getRaw());
        }
        void pushRaw(HSQUIRRELVM vm, const Array& value) {
            sq_pushobject(vm, value.getRaw());
        }
        void pushRaw(HSQUIRRELVM vm, const SqWeakRef& value) {
            sq_pushobject(vm, value.getRaw());
        }
    }
}
