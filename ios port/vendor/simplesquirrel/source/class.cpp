#include "simplesquirrel/object.hpp"
#include "simplesquirrel/class.hpp"
#include "simplesquirrel/exceptions.hpp"
#include "simplesquirrel/function.hpp"
#include <squirrel.h>
#include <forward_list>

namespace ssq {
	Class::Class() :Object(), tableSet(), tableGet() {

    }

    Class::Class(HSQUIRRELVM vm) :Object(vm), tableSet(), tableGet() {

    }

    Class::Class(const Object& object) : Object(object.getHandle()), tableSet(), tableGet() {
        if (object.getType() != Type::CLASS) throw TypeException("bad cast", "CLASS", object.getTypeStr());
        if (vm != nullptr && !object.isEmpty()) {
            obj = object.getRaw();
            sq_addref(vm, &obj);
        }
    }

    Class::Class(const Class& other) :Object(other), tableSet(other.tableSet), tableGet(other.tableGet) {

    }

    Class::Class(Class&& other) NOEXCEPT : Object(std::forward<Class>(other)),
        tableSet(std::forward<Object>(other.tableSet)),
        tableGet(std::forward<Object>(other.tableGet)) {

    }

    void Class::swap(Class& other) NOEXCEPT {
        if (this != &other) {
            Object::swap(other);
            tableSet.swap(other.tableSet);
            tableGet.swap(other.tableGet);
        }
    }

    Function Class::findFunc(const char* name) const {
        Object object = Object::find(name);
        return Function(object);
    }

    Class& Class::operator = (const Class& other) {
        if (this != &other) {
            Class o(other);
            swap(o);
        }
        return *this;
    }

    Class& Class::operator = (Class&& other) NOEXCEPT {
        if (this != &other) {
            swap(other);
        }
        return *this;
    }

    void Class::findTable(const char* name, Object& table, SQFUNCTION dlg) const {
        // Check if the table has been referenced
        if(!table.isEmpty()) {
            return;
        }
            
        // Find the table
        sq_pushobject(vm, obj);
        sq_pushstring(vm, name, strlen(name));

        if (SQ_FAILED(sq_get(vm, -2))) {
            // Does not exist
            sq_pop(vm, 1);

            // Create one
            table = Object(vm);
            sq_newtable(vm);
            sq_getstackobj(vm, -1, &table.getRaw());
            sq_addref(vm, &table.getRaw());

            // Set the root table as a delegate
            sq_pushroottable(vm);
            if (SQ_FAILED(sq_setdelegate(vm, -2))) {
                sq_pop(vm, 1); // Pop table
                throw ssq::RuntimeException(vm, "Cannot set root table as class table delegate!");
            }

            sq_pop(vm, 1); // Pop table

            sq_pushobject(vm, obj); // Push class obj
            sq_pushstring(vm, name, strlen(name));
            sq_pushobject(vm, table.getRaw());
            sq_newclosure(vm, dlg, 1);

            if(SQ_FAILED(sq_newslot(vm, -3, false))) {
                throw RuntimeException(vm, "Failed to create table '" + std::string(name) + "'!");
            }

            sq_pop(vm, 1); // Pop class obj

        } else {
            // Return one
            table = Object(vm);
            sq_getstackobj(vm, -1, &table.getRaw());
            sq_addref(vm, &table.getRaw());
            sq_pop(vm, 2);
        }
    }

    SQInteger Class::dlgGetStub(HSQUIRRELVM vm) {
        // To get an entry from the "_get" table, first search for it, without employing delegation.
        // If found, that means it's a member variable, exposed from C++.
        sq_push(vm, 2); // Push entry key
        if (!SQ_SUCCEEDED(sq_rawget(vm, -2))) {
            // If nothing was found, perform a second search, this time employing delegation.
            // If found, the entry originates from the root table, added as a delegate, meaning it should only be pushed to the stack.
            sq_push(vm, 2); // Push entry key
            if (SQ_SUCCEEDED(sq_get(vm, -2))) {
                return 1; // Result is pushed in the stack
            }

            return sq_throwerror(vm, ("Variable not found: " + detail::pop<std::string>(vm, 2)).c_str());
        }

        // Push 'this'
        sq_push(vm, 1);

        // Call the getter
        if (SQ_FAILED(sq_call(vm, 1, SQTrue, SQTrue))) {
            return sq_throwerror(vm, Exception(vm, "Error calling \"" + detail::pop<std::string>(vm, 2) + "\" getter!").what());
        }
        return 1;
    }

    SQInteger Class::dlgSetStub(HSQUIRRELVM vm) {
        // To get an entry from the "_set" table, first search for it, without employing delegation.
        // If found, that means it's a member variable, exposed from C++.
        sq_push(vm, 2); // Push entry key
        if (!SQ_SUCCEEDED(sq_rawget(vm, -2))) {
            // If nothing was found, perform a second search, this time employing delegation.
            // If found, the entry originates from the root table, added as a delegate, meaning it should only be pushed to the stack.
            sq_push(vm, 2); // Push entry key
            if (SQ_SUCCEEDED(sq_get(vm, -2))) {
                return 1; // Result is pushed in the stack
            }

            return sq_throwerror(vm, ("Variable not found: " + detail::pop<std::string>(vm, 2)).c_str());
        }

        // Push 'this'
        sq_push(vm, 1);

        // Push the value
        sq_push(vm, 3);

        // Call the setter
        if (SQ_FAILED(sq_call(vm, 2, SQTrue, SQTrue))) {
            return sq_throwerror(vm, Exception(vm, "Error calling \"" + detail::pop<std::string>(vm, 2) + "\" setter!").what());
        }
        return 1;
    }
}
