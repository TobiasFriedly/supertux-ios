#pragma once

namespace ssq {
    /**
    * @brief Any exposed classes must inherit this interface
    * @ingroup simplesquirrel
    */
    class ExposableClass {
    public:
        /**
        * @brief Constructs an instance
        */
        ExposableClass() {}
        /**
        * @brief Virtual destructor
        */
        virtual ~ExposableClass() {}
    };
}
