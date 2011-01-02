/*
 * dl.hpp - Programming interface to the dynamic linking loader
 *
 *   Copyright 2010 Jesús Torres <jmtorres@ull.es>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DL_HPP_
#define DL_HPP_

#include <string>

#include <boost/function.hpp>
#include <boost/system/error_code.hpp>
#include <boost/type_traits/add_pointer.hpp>

namespace dl
{
    namespace posix {
        #include <dlfcn.h>
    }

    using namespace boost;

    //
    // Error handling support
    //

    namespace loader_error
    {
        enum LoaderError
        {
            LIBRARY_LOAD_FAILED,
            SYMBOL_RESOLUTION_FAILED
        };

        system::error_code make_error_code(LoaderError e);
        system::error_condition make_error_condition(LoaderError e);
    }

    class LoaderCategory : public system::error_category
    {
        virtual const char* name() const;
        virtual std::string message(int ev) const;
        virtual system::error_condition default_error_condition(int ev) const;
    };

    system::error_category& loaderCategory();

    //
    // Class DynamicLibrary
    //

    class DynamicLibrary
    {
        public:

            enum OpenFlags
            {
                LAZY_BINDING        = RTLD_LAZY,
                ONLOAD_BINDING      = RTLD_NOW,
                GLOBAL_BINDING      = RTLD_GLOBAL,
                LOCAL_BINDING       = RTLD_LOCAL,
                NOUNLOAD_ONCLOSE    = RTLD_NODELETE,
                NOLOAD_ONOPEN       = RTLD_NOLOAD,
                DEEP_BINDING        = RTLD_DEEPBIND
            };

            DynamicLibrary(const std::string& fileName = std::string(),
                OpenFlags flags = ONLOAD_BINDING);
            ~DynamicLibrary();

            //
            // Methods for symbol resolution
            //

            template<typename Signature>
            boost::function<Signature>
            getFunction(const std::string& symbol)
                { return getFunction<Signature>(libraryHandle_, symbol); }

            template<typename Signature>
            boost::function<Signature>
            getDefaultFunction(const std::string& symbol)
                { return getFunction<Signature>(RTLD_DEFAULT, symbol); }

            template<typename Signature>
            boost::function<Signature>
            getNextFunction(const std::string& symbol)
                { return getFunction<Signature>(RTLD_NEXT, symbol); }

            template<typename Type>
            Type* getVariable(const std::string& symbol)
                { return getVariable<Type>(libraryHandle_, symbol); }

            template<typename Type>
            Type* getDefaultVariable(const std::string& symbol)
                { return getVariable<Type>(RTLD_DEFAULT, symbol); }

            template<typename Type>
            Type* getNextVariable(const std::string& symbol)
                { return getVariable<Type>(RTLD_NEXT, symbol); }

            //
            // Error handling
            //

            bool isOpen() const { return libraryHandle_ != NULL; }

            system::error_code getLastError() const
                { return errorCode_; }

            const std::string getLastErrorMessage() const
                { return lastErrorMessage_; }

            //
            // C library function wrappers
            //

            static void* dlopen(const std::string& fileName, OpenFlags flags)
                { return posix::dlopen(fileName.c_str(), flags); }

            static int dlclose(void* handle)
                { return posix::dlclose(handle); }

            static void* dlsym(void* handle, const std::string &symbol)
                { return posix::dlsym(handle, symbol.c_str()); }

            static std::string dlerror();

        protected:
            system::error_code errorCode_;

        private:
            void* libraryHandle_;
            std::string lastErrorMessage_;

            //
            // Private methods for symbol resolution
            //

            template<typename Signature>
            boost::function<Signature>
            getFunction(void* handle, const std::string& symbol);

            template<typename Type>
            Type* getVariable(void* handle, const std::string& symbol);
    };

    inline std::string DynamicLibrary::dlerror()
    {
        const char *message = posix::dlerror();
        return (message == NULL) ? std::string() : std::string(message);
    }

    template<typename Signature>
    boost::function<Signature> DynamicLibrary::getFunction(void* handle,
        const std::string& symbol)
    {
        boost::function<Signature> function;

        DynamicLibrary::dlerror();
        void* address = DynamicLibrary::dlsym(handle, symbol.c_str());
        std::string errorMessage = DynamicLibrary::dlerror();
        if ((address == NULL) && errorMessage.empty()) {
            lastErrorMessage_ = errorMessage;
            errorCode_ = loader_error::SYMBOL_RESOLUTION_FAILED;
        }
        else {
            using namespace boost;
            typedef typename add_pointer<Signature>::type function_pointer;
            function = reinterpret_cast<function_pointer>(address);
            errorCode_.clear();
        }

        return function;
    }

    template<typename Type>
    Type* DynamicLibrary::getVariable(void* handle, const std::string &symbol)
    {
        DynamicLibrary::dlerror();
        void* address = DynamicLibrary::dlsym(handle, symbol.c_str());
        std::string errorMessage = DynamicLibrary::dlerror();
        if ((address == NULL) && errorMessage.empty()) {
            lastErrorMessage_ = errorMessage;
            errorCode_ = loader_error::SYMBOL_RESOLUTION_FAILED;
        }
        else {
            errorCode_.clear();
        }

        return reinterpret_cast<Type*>(address);
    }

    //
    // Boolean operators overload for DynamicLibrary::OpenFlags
    //

    inline DynamicLibrary::OpenFlags
    operator&(DynamicLibrary::OpenFlags a, DynamicLibrary::OpenFlags b)
    {
        return DynamicLibrary::OpenFlags(static_cast<int>(a)
            & static_cast<int>(b));
    }

    inline DynamicLibrary::OpenFlags
    operator|(DynamicLibrary::OpenFlags a, DynamicLibrary::OpenFlags b)
    {
        return DynamicLibrary::OpenFlags(static_cast<int>(a)
            | static_cast<int>(b));
    }

    inline DynamicLibrary::OpenFlags
    operator^(DynamicLibrary::OpenFlags a, DynamicLibrary::OpenFlags b)
    {
        return DynamicLibrary::OpenFlags(static_cast<int>(a)
            ^ static_cast<int>(b));
    }

    inline DynamicLibrary::OpenFlags
    operator&=(DynamicLibrary::OpenFlags &a, DynamicLibrary::OpenFlags b)
    {
        return a = a & b;
    }

    inline DynamicLibrary::OpenFlags
    operator|=(DynamicLibrary::OpenFlags &a, DynamicLibrary::OpenFlags b)
    {
        return a = a | b;
    }

    inline DynamicLibrary::OpenFlags
    operator^=(DynamicLibrary::OpenFlags a, DynamicLibrary::OpenFlags b)
    {
        return a = a ^ b;
    }

    inline DynamicLibrary::OpenFlags
    operator~(DynamicLibrary::OpenFlags a)
    {
        return DynamicLibrary::OpenFlags(~static_cast<int>(a));
    }
}

//
// Enable automatic conversion from LoaderError enum to error_code constants
//

namespace boost { namespace system
{
    template <>
    struct is_error_code_enum<dl::loader_error::LoaderError>
        : public true_type {};
}}

#endif /* DL_HPP_ */
