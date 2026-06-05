#include "simplesquirrel/array.hpp"
#include "simplesquirrel/exceptions.hpp"
#include <squirrel.h>
#include <forward_list>

namespace ssq {
    Array::Array(HSQUIRRELVM vm, size_t len):Object(vm) {
        sq_newarray(vm, len);
        sq_getstackobj(vm, -1, &obj);
        sq_addref(vm, &obj);
        sq_pop(vm,1); // Pop array
    }

    Array::Array(const Object& object):Object(object) {
        if (object.getType() != Type::ARRAY) throw TypeException("bad cast", "ARRAY", object.getTypeStr());
    }

    Array::Array(const Array& other):Object(other) {
            
    }

    Array::Array(Array&& other) NOEXCEPT :Object(std::forward<Array>(other)) {
            
    }

    size_t Array::size() {
        sq_pushobject(vm, obj);
        SQInteger s = sq_getsize(vm, -1);
        sq_pop(vm, 1);
        return static_cast<size_t>(s);
    }

    std::vector<Object> Array::convertRaw() const {
        return convert<Object>();
    }

    void Array::pop() {
        sq_pushobject(vm, obj);
        const size_t s = static_cast<size_t>(sq_getsize(vm, -1));
        if(s == 0) {
            sq_pop(vm, 1);
            throw RuntimeException(vm, "Failed to pop empty array!");
        }

        if(SQ_FAILED(sq_arraypop(vm, -1, SQFalse))) {
            sq_pop(vm, 1);
            throw RuntimeException(vm, "Failed to pop value from the back of array!");
        }
        sq_pop(vm, 1);
    }

    void Array::clear() {
        sq_pushobject(vm, obj);
        sq_clear(vm, -1);
        sq_pop(vm, 1);
    }

    Array& Array::operator = (const Array& other){
        Object::operator = (other);
        return *this;
    }

    Array& Array::operator = (Array&& other) NOEXCEPT {
        Object::operator = (std::forward<Array>(other));
        return *this;
    }
}
