#include "simplesquirrel/object.hpp"
#include "simplesquirrel/function.hpp"
#include "simplesquirrel/class.hpp"
#include "simplesquirrel/enum.hpp"
#include "simplesquirrel/table.hpp"
#include <squirrel.h>
#include <forward_list>
#include <cassert>
#include <limits>

namespace ssq {
    Table::Table():Object() {
            
    }

    Table::Table(const Object& object):Object(object) {
        if (object.getType() != Type::TABLE) throw TypeException("bad cast", "TABLE", object.getTypeStr());
    }

    Table::Table(HSQUIRRELVM vm):Object(vm) {
        sq_newtable(vm);
        sq_getstackobj(vm, -1, &obj);
        sq_addref(vm, &obj);
        sq_pop(vm,1); // Pop table
    }

    Table::Table(const Table& other):Object(other) {
            
    }

    Table::Table(Table&& other) NOEXCEPT :Object(std::forward<Table>(other)) {
            
    }

    Function Table::findFunc(const char* name) const {
        Object object = Object::find(name);
        return Function(object);
    }

    Class Table::findClass(const char* name) const {
        Object object = Object::find(name);
        return Class(object);
    }

    Table Table::findTable(const char* name) const {
        Object object = Object::find(name);
        return Table(object);
    }

    Table Table::addTable(const char* name) {
        assert(sizeof(name) < static_cast<size_t>(std::numeric_limits<SQInteger>::max()));
        Table table(vm);
        sq_pushobject(vm, obj);
        sq_pushstring(vm, name, strlen(name));
        detail::push<Object>(vm, table);
        if(SQ_FAILED(sq_newslot(vm, -3, SQFalse))) {
            throw RuntimeException(vm, "Failed to add table '" + std::string(name) + "'!");
        }
        sq_pop(vm,1); // pop table
        return table;
    }

    Table Table::getOrCreateTable(const char* name) {
        assert(sizeof(name) < static_cast<size_t>(std::numeric_limits<SQInteger>::max()));
        try {
            return find(name).toTable();
        }
        catch (const NotFoundException&) {
            return addTable(name);
        }
        catch (const TypeException&) {
            return addTable(name);
        }
    }

    bool Table::hasEntry(const char* name) const {
        assert(sizeof(name) < static_cast<size_t>(std::numeric_limits<SQInteger>::max()));
        sq_pushobject(vm, obj);
        sq_pushstring(vm, name, static_cast<SQInteger>(strlen(name)));
        if(SQ_FAILED(sq_get(vm, -2))) {
            sq_pop(vm, 1); // pop table
            return false;
        }
        sq_pop(vm, 2); // pop result and table
        return true;
    }

    void Table::rename(const char* old_name, const char* new_name) {
        assert(sizeof(old_name) < static_cast<size_t>(std::numeric_limits<SQInteger>::max()));
        assert(sizeof(new_name) < static_cast<size_t>(std::numeric_limits<SQInteger>::max()));

        const SQInteger old_top = sq_gettop(vm);
        sq_pushobject(vm, obj);

        sq_pushstring(vm, new_name, -1);
        sq_pushstring(vm, old_name, -1);
        if (SQ_FAILED(sq_deleteslot(vm, -3, SQTrue))) {
            sq_settop(vm, old_top);
            throw RuntimeException(vm, "Cannot delete table entry for rename!");
        }
        if(SQ_FAILED(sq_newslot(vm, -3, SQFalse))) {
            sq_settop(vm, old_top);
            throw RuntimeException(vm, "Cannot create renamed table entry!");
        }

        sq_settop(vm, old_top);
    }

    void Table::remove(const char* name) {
        assert(sizeof(name) < static_cast<size_t>(std::numeric_limits<SQInteger>::max()));
        sq_pushobject(vm, obj);
        sq_pushstring(vm, name, static_cast<SQInteger>(strlen(name)));
        sq_deleteslot(vm, -2, SQFalse);
        sq_pop(vm, 1); // pop table
    }

    void Table::clear() {
        sq_pushobject(vm, obj);
        sq_clear(vm, -1);
        sq_pop(vm, 1); // pop table
    }

    void Table::setDelegate(Table& table) {
        const SQInteger old_top = sq_gettop(vm);
        sq_pushobject(vm, obj);
        sq_pushobject(vm, table.getRaw());
        if (SQ_FAILED(sq_setdelegate(vm, -2))) {
            sq_settop(vm, old_top);
            throw ssq::RuntimeException(vm, "Cannot set table as table delegate!");
        }
        sq_settop(vm, old_top);
    }

    size_t Table::size() const {
        sq_pushobject(vm, obj);
        SQInteger s = sq_getsize(vm, -1);
        sq_pop(vm, 1);
        return static_cast<size_t>(s);
    }

    std::vector<std::string> Table::getKeys() const {
        const SQInteger old_top = sq_gettop(vm);
        sq_pushobject(vm, obj);

        std::vector<std::string> keys;

        sq_pushnull(vm); // push iterator
        while (SQ_SUCCEEDED(sq_next(vm, -2)))
        {
            // -1 is the value and -2 is the key
            const char* key;
            if (SQ_FAILED(sq_getstring(vm, -2, &key)))
                throw RuntimeException(vm, "Cannot get string value for table entry key!");
            else
                keys.push_back(key);

            sq_pop(vm, 2); // pop key and value of this iteration
        }

        sq_settop(vm, old_top);
        return keys;
    }

    std::map<std::string, ssq::Object> Table::convertRaw() const {
        const SQInteger old_top = sq_gettop(vm);
        sq_pushobject(vm, obj);

        std::map<std::string, ssq::Object> map;

        sq_pushnull(vm); // push iterator
        while (SQ_SUCCEEDED(sq_next(vm, -2)))
        {
            // -1 is the value and -2 is the key
            const char* key;
            if (SQ_FAILED(sq_getstring(vm, -2, &key)))
                throw RuntimeException(vm, "Cannot get string value for table entry key!");
            else
                map.insert({ key, detail::pop<Object>(vm, -1) });

            sq_pop(vm, 2); // pop key and value of this iteration
        }

        sq_settop(vm, old_top);
        return map;
    }

    Table& Table::operator = (const Table& other){
        Object::operator = (other);
        return *this;
    }

    Table& Table::operator = (Table&& other) NOEXCEPT {
        Object::operator = (std::forward<Table>(other));
        return *this;
    }
}
