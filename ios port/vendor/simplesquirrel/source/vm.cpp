#include <squirrel.h>
#include <sqstdstring.h>
#include <sqstdsystem.h>
#include <sqstdmath.h>
#include <sqstdaux.h>
#include <sqstdblob.h>
#include <sqstdio.h>
#include <forward_list>
#include <cassert>
#include <cstdarg>
#include <cstring>
#include <algorithm>
#include <iostream>

#include "simplesquirrel/object.hpp"
#include "simplesquirrel/enum.hpp"
#include "simplesquirrel/vm.hpp"

static SQInteger squirrel_istream_read_char(SQUserPointer stream)
{
  std::istream* in = reinterpret_cast<std::istream*>(stream);

  const SQInteger c = in->get();
  if (in->eof())
    return 0;

  return c;
}

namespace ssq {
    VM* VM::get(HSQUIRRELVM vm) {
        SQUserPointer ptr = sq_getforeignptr(vm);
        if (!ptr) return nullptr;
        return static_cast<VM*>(ptr);
    }

    VM& VM::getMain(HSQUIRRELVM vm) {
        SQUserPointer ptr = sq_getsharedforeignptr(vm);
        assert(ptr);
        return *static_cast<VM*>(ptr);
    }

    VM::VM():Table(), foreignPtr(nullptr) {

    }

    VM::VM(size_t stackSize, uint32_t flags):Table(), foreignPtr(nullptr) {
        vm = sq_open(stackSize);
        sq_setforeignptr(vm, this);
        sq_setsharedforeignptr(vm, this);

        registerStdlib(flags);

        setPrintFunc(&VM::defaultPrintFunc, &VM::defaultErrorFunc);
        //setRuntimeErrorFunc(&VM::defaultRuntimeErrorFunc);
        //setCompileErrorFunc(&VM::defaultCompilerErrorFunc);
        setStdErrorFunc(); // Use error functions from the Squirrel standard library by default, because they're more verbose (ex. callstacks)

        sq_resetobject(&obj);
        sq_pushroottable(vm);
        sq_getstackobj(vm, -1, &obj);
        sq_addref(vm, &obj);
        sq_pop(vm, 1);
    }

    VM::VM(const HSQOBJECT& threadObj):Table(), foreignPtr(nullptr) {
        assert(threadObj._type == OT_THREAD);

        vm = threadObj._unVal.pThread;
        sq_setforeignptr(vm, this);

        sq_resetobject(&obj);
        sq_pushroottable(vm);
        sq_getstackobj(vm, -1, &obj);
        sq_addref(vm, &obj);
        sq_pop(vm, 1);
    }

    void VM::destroy() {
        classMap.clear();
        if (vm != nullptr) {
            sq_resetobject(&obj);

            VM& mainVM = VM::getMain(vm);
            if (&mainVM == this) { // This is the main VM
                // Destroy all threads
                for (HSQOBJECT& threadObj : threads) {
                    sq_resetobject(&threadObj);
                }
                threads.clear();

                sq_collectgarbage(vm);
                sq_close(vm);
            } else { // This is a thread VM, originating from simplesquirrel
                mainVM.destroyThread(*this);
            }
        }
        vm = nullptr;
    }

    VM::~VM() {
        destroy();
    }

    void VM::swap(VM& other) NOEXCEPT {
        if (vm)
        {
          sq_setforeignptr(vm, &other);
          if (sq_getsharedforeignptr(vm) == this)
            sq_setsharedforeignptr(vm, &other);
        }
        if (other.vm)
        {
          sq_setforeignptr(other.vm, this);
          if (sq_getsharedforeignptr(other.vm) == &other)
            sq_setsharedforeignptr(other.vm, this);
        }

        using std::swap;
        Object::swap(other);
        //swap(runtimeException, other.runtimeException);
        //swap(compileException, other.compileException);
        swap(classMap, other.classMap);
        swap(foreignPtr, other.foreignPtr);
    }
        
    VM::VM(VM&& other) NOEXCEPT :Table(), foreignPtr(nullptr) {
        swap(other);
    }

    void VM::registerStdlib(uint32_t flags) {
        if (flags == 0)return;
        sq_pushroottable(vm);
        if(flags & ssq::Libs::IO)
            sqstd_register_iolib(vm);
        if(flags & ssq::Libs::BLOB)
            sqstd_register_bloblib(vm);
        if(flags & ssq::Libs::MATH)
            sqstd_register_mathlib(vm);
        if(flags & ssq::Libs::SYSTEM)
            sqstd_register_systemlib(vm);
        if(flags & ssq::Libs::STRING)
            sqstd_register_stringlib(vm);
        sq_pop(vm, 1);
    }

    void VM::setRootTable(Table& table) {
        sq_resetobject(&obj);
        sq_pushobject(vm, table.getRaw());
        sq_getstackobj(vm, -1, &obj);
        sq_addref(vm, &obj);
        sq_setroottable(vm);
    }

    void VM::setPrintFunc(SqPrintFunc printFunc, SqErrorFunc errorFunc) {
        sq_setprintfunc(vm, printFunc, errorFunc);
    }

    void VM::setRuntimeErrorFunc(SqRuntimeErrorFunc runtimeErrorFunc) {
        sq_newclosure(vm, runtimeErrorFunc, 0);
        sq_seterrorhandler(vm);
    }

    void VM::setCompileErrorFunc(SqCompileErrorFunc compileErrorFunc) {
        sq_setcompilererrorhandler(vm, compileErrorFunc);
    }

    void VM::setStdErrorFunc() {
        sqstd_seterrorhandlers(vm);
    }

    void VM::setForeignPtr(void* ptr) {
        foreignPtr = ptr;
    }

    void* VM::getForeignPtr() const {
        return foreignPtr;
    }

    bool VM::isThread() const {
        return VM::getMain(vm).getHandle() != vm;
    }

    SQInteger VM::getState() const {
        return sq_getvmstate(vm);
    }

    SQInteger VM::getTop() const {
        return sq_gettop(vm);
    }

    Script VM::compileSource(const char* source, const char* name) {
        Script script(vm);
        if (SQ_FAILED(sq_compilebuffer(vm, source, strlen(source), name, true))) {
            //if (!compileException)
                throw CompileException(vm, "Source cannot be compiled!");
            //throw *compileException;
        }

        sq_getstackobj(vm,-1,&script.getRaw());
        sq_addref(vm, &script.getRaw());
        sq_pop(vm, 1);
        return script;
    }

    Script VM::compileSource(std::istream& source, const char* name) {
        Script script(vm);
        if (SQ_FAILED(sq_compile(vm, squirrel_istream_read_char, &source, name, SQTrue))) {
            //if (!compileException)
                throw CompileException(vm, "Source cannot be compiled!");
            //throw *compileException;
        }

        sq_getstackobj(vm,-1,&script.getRaw());
        sq_addref(vm, &script.getRaw());
        sq_pop(vm, 1);
        return script;
    }

    Script VM::compileFile(const char* path) {
        Script script(vm);
        if (SQ_FAILED(sqstd_loadfile(vm, path, true))) {
            //if (!compileException)
                throw CompileException(vm, "File not found or cannot be read!");
            //throw *compileException;
        }

        sq_getstackobj(vm, -1, &script.getRaw());
        sq_addref(vm, &script.getRaw());
        sq_pop(vm, 1);
        return script;
    }

    void VM::run(const Script& script, bool printCallstack) {
        if (script.isEmpty()) {
            throw RuntimeException(vm, "Empty script object.");
        }

        const SQInteger old_top = sq_gettop(vm);
        sq_pushobject(vm, script.getRaw());
        sq_pushroottable(vm);
        try {
            if (SQ_FAILED(sq_call(vm, 1, SQFalse, SQTrue))) {
                //if (!runtimeException)
                    throw RuntimeException(vm, "Error running script!");
                //throw *runtimeException;
            }

            // Root table may've changed
            sq_resetobject(&obj);
            sq_pushroottable(vm);
            sq_getstackobj(vm, -1, &obj);
            sq_addref(vm, &obj);
            sq_pop(vm, 1);

            if (sq_getvmstate(vm) != SQ_VMSTATE_SUSPENDED) {
                sq_settop(vm, old_top);
            }
        }
        catch (const RuntimeException&) {
            if (printCallstack) {
                sqstd_printcallstack(vm);
            }
            sq_settop(vm, old_top);
            throw;
        }
        catch (const std::exception& err) {
            sq_settop(vm, old_top);
            throw std::runtime_error(std::string("Running script failed: ") + err.what());
        }
    }

    Object VM::runAndReturn(const Script& script, bool printCallstack) {
        if (script.isEmpty()) {
            throw RuntimeException(vm, "Empty script object.");
        }

        const SQInteger old_top = sq_gettop(vm);
        sq_pushobject(vm, script.getRaw());
        sq_pushroottable(vm);
        try {
            if (SQ_FAILED(sq_call(vm, 1, SQTrue, SQTrue))) {
                //if (!runtimeException)
                    throw RuntimeException(vm, "Error running script!");
                //throw *runtimeException;
            }

            // Root table may've changed
            sq_resetobject(&obj);
            sq_pushroottable(vm);
            sq_getstackobj(vm, -1, &obj);
            sq_addref(vm, &obj);
            sq_pop(vm, 1);

            Object ret = detail::pop<Object>(vm, -1);
            sq_settop(vm, old_top);
            return ret;
        }
        catch (const RuntimeException&) {
            if (printCallstack) {
                sqstd_printcallstack(vm);
            }
            sq_settop(vm, old_top);
            throw;
        }
        catch (const std::exception& err) {
            sq_settop(vm, old_top);
            throw std::runtime_error(std::string("Running script failed: ") + err.what());
        }
    }

    VM VM::newThread(size_t stackSize) {
        assert(VM::getMain(vm).getHandle() == vm); // Assert this is the main VM

        HSQUIRRELVM thread = sq_newthread(vm, stackSize);
        if (!thread)
            throw RuntimeException(vm, "Failed to create thread!");

        HSQOBJECT threadObj;
        sq_resetobject(&threadObj);
        if (SQ_FAILED(sq_getstackobj(vm, -1, &threadObj)))
            throw RuntimeException(vm, "Failed to get Squirrel thread from stack!");
        sq_addref(vm, &threadObj);

        VM threadVM(threadObj);
        threads.push_back(std::move(threadObj));

        sq_pop(vm, 1); // Pop thread
        return threadVM;
    }

    void VM::destroyThread(VM& threadVM) {
        assert(VM::getMain(vm).getHandle() == vm); // Assert this is the main VM
        assert(threadVM.vm);

        auto it = std::find_if(threads.begin(), threads.end(),
            [&threadVM](const HSQOBJECT& threadObj) {
                return threadVM.vm == threadObj._unVal.pThread;
            });
        assert(it != threads.end());

        sq_resetobject(&*it);
        threads.erase(it);

        sq_collectgarbage(vm);
        threadVM.vm = nullptr;
    }

    Enum VM::addEnum(const char* name) {
        Enum enm(vm);
        sq_pushconsttable(vm);
        sq_pushstring(vm, name, strlen(name));
        detail::push<Object>(vm, enm);
        if(SQ_FAILED(sq_newslot(vm, -3, SQFalse))) {
            throw RuntimeException(vm, "Failed to add enumerator '" + std::string(name) + "'!");
        }
        sq_pop(vm,1); // pop table
        return std::move(enm);
    }

    VM& VM::operator = (VM&& other) NOEXCEPT {
        if(this != &other) {
            swap(other);
        }
        return *this;
    }

    Object VM::callAndReturn(SQUnsignedInteger nparams, SQInteger top) const {
        if(SQ_FAILED(sq_call(vm, 1 + nparams, SQTrue, SQTrue))) {
            sq_settop(vm, top);
            //if (!runtimeException)
                throw RuntimeException(vm, "Error running script!");
            //throw *runtimeException;
        }

        Object ret(vm);
        sq_getstackobj(vm, -1, &ret.getRaw());
        sq_addref(vm, &ret.getRaw());
        sq_settop(vm, top);
        return ret;
    }

    void VM::debugStack() const {
        auto top = getTop();
        while(top >= 0) {
            SQObjectType objectType = sq_gettype(vm, top);
            Type type = Type(objectType);
            std::cout << "stack index: " << top << " type: " << typeToStr(type) << std::endl;
            top--;
        }
    }

    void VM::defaultPrintFunc(HSQUIRRELVM vm, const SQChar *s, ...){
        va_list vl;
        va_start(vl, s);
        vprintf(s, vl);
        printf("\n");
        va_end(vl);
    }

    void VM::defaultErrorFunc(HSQUIRRELVM vm, const SQChar *s, ...){
        va_list vl;
        va_start(vl, s);
        fprintf(stderr, s, vl);
        fprintf(stderr, "\n");
        va_end(vl);
    }
/*
    SQInteger VM::defaultRuntimeErrorFunc(HSQUIRRELVM vm) {
        SQStackInfos si;
        sq_stackinfos(vm, 1, &si);

        auto source = (si.source != nullptr ? si.source : "null");
        auto funcname = (si.funcname != nullptr ? si.funcname : "unknown");

        const SQChar *sErr = 0;
        if(sq_gettop(vm) >= 1){
            if(SQ_FAILED(sq_getstring(vm, 2, &sErr))){
                sErr = "unknown error";
            }
        }

        VM* ssq_vm = VM::get(vm);
        if (!ssq_vm) return 0;
        ssq_vm->runtimeException.reset(new RuntimeException(
            vm,
            sErr,
            source,
            funcname,
            si.line
        ));
        return 0;
    }

    void VM::defaultCompilerErrorFunc(
        HSQUIRRELVM vm,
        const SQChar* desc,
        const SQChar* source,
        SQInteger line,
        SQInteger column) {
        VM* ssq_vm = VM::get(vm);
        if (!ssq_vm) return;
        ssq_vm->compileException.reset(new CompileException(
            vm,
            desc,
            source,
            line,
            column
        ));
    }
*/
    void VM::pushArgs() {

    }

    std::unordered_map<size_t, HSQOBJECT> VM::classMap = {};

    void VM::addClassObj(size_t hashCode, const HSQOBJECT& obj) {
        classMap[hashCode] = obj;
    }

    const HSQOBJECT& VM::getClassObj(size_t hashCode) {
        return classMap.at(hashCode);
    }

    namespace detail {
        void addClassObj(size_t hashCode, const HSQOBJECT& obj) {
            VM::addClassObj(hashCode, obj);
        }

        const HSQOBJECT& getClassObj(size_t hashCode) {
            return VM::getClassObj(hashCode);
        }
    }
}
