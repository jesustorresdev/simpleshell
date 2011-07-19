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

#include <climits>
#include <string>

#include <boost/function.hpp>
#include <boost/type_traits/add_pointer.hpp>

#include <cli/internals.hpp>

namespace dl
{
    namespace posix {
        #include <dlfcn.h>
    }

    using namespace boost;

    //
    // Error handling support
    //

    namespace LoaderError
    {
        enum LoaderErrorType
        {
            LIBRARY_ALREADY_LOADED = 1,
            LIBRARY_LOAD_FAILED,
            LIBRARY_NOT_LOADED,
            SYMBOL_RESOLUTION_FAILED
        };

        std::error_code make_error_code(LoaderErrorType e);
        std::error_condition make_error_condition(LoaderErrorType e);
    }

    class LoaderCategory : public std::error_category
    {
        virtual const char* name() const;
        virtual std::string message(int ev) const;
        virtual std::error_condition default_error_condition(int ev) const;
    };

    std::error_category& loaderCategory();

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
                LOCAL_BINDING       = RTLD_LOCAL
#if defined(__GLIBC__)
# if __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2)
               ,NOUNLOAD_ONCLOSE    = RTLD_NODELETE,
                NOLOAD_ONOPEN       = RTLD_NOLOAD
# endif
# if __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ > 3)
               ,DEEP_BINDING        = RTLD_DEEPBIND
# endif
#endif /* __GLIBC__ */
            };

            enum SpecialFileNames
            {
                MAIN_PROGRAM
            };

            DynamicLibrary();

            template <typename T>
            DynamicLibrary(T const& fileName,
                OpenFlags flags = ONLOAD_BINDING);

            ~DynamicLibrary();

            void load(const char* fileName,
                OpenFlags flags = ONLOAD_BINDING);
            void load(const std::string& fileName,
                OpenFlags flags = ONLOAD_BINDING);
            void load(enum SpecialFileNames fileName,
                OpenFlags flags = ONLOAD_BINDING);

            //
            // Methods for symbol resolution
            //

            template<typename Signature>
            boost::function<Signature>
            getFunction(const std::string& symbol)
                { return getFunction<Signature>(libraryHandle_, symbol); }
            
            template<typename Type>
            Type* getVariable(const std::string& symbol)
                { return getVariable<Type>(libraryHandle_, symbol); }

#if defined(_GNU_SOURCE)
            template<typename Signature>
            boost::function<Signature>
            getDefaultFunction(const std::string& symbol)
                { return getFunction<Signature>(RTLD_DEFAULT, symbol); }

            template<typename Signature>
            boost::function<Signature>
            getNextFunction(const std::string& symbol)
                { return getFunction<Signature>(RTLD_NEXT, symbol); }

            template<typename Type>
            Type* getDefaultVariable(const std::string& symbol)
                { return getVariable<Type>(RTLD_DEFAULT, symbol); }

            template<typename Type>
            Type* getNextVariable(const std::string& symbol)
                { return getVariable<Type>(RTLD_NEXT, symbol); }
#endif /* _GNU_SOURCE */

            //
            // Error handling
            //

            bool isLoad() const { return libraryHandle_ != NULL; }

            std::error_code getLastError() const
                { return errorCode_; }

            const std::string getLastErrorMessage() const
                { return lastErrorMessage_; }

            //
            // C library function wrappers
            //

            static void* dlopen(const std::string& fileName, OpenFlags flags)
                { return posix::dlopen(fileName.c_str(), flags); }
            static void* dlopen(const char* fileName, OpenFlags flags)
                { return posix::dlopen(fileName, flags); }

            static int dlclose(void* handle)
                { return posix::dlclose(handle); }

            static void* dlsym(void* handle, const std::string &symbol)
                { return posix::dlsym(handle, symbol.c_str()); }
            static void* dlsym(void* handle, const char* symbol)
                { return posix::dlsym(handle, symbol); }

            static std::string dlerror();

        protected:
            std::error_code errorCode_;

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

    template <typename T>
    DynamicLibrary::DynamicLibrary(T const& fileName, OpenFlags flags)
        : libraryHandle_(NULL)
    {
        load(fileName, flags);
    }

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

        if (isLoad()) {
            DynamicLibrary::dlerror();
            void* address = DynamicLibrary::dlsym(handle, symbol.c_str());
            std::string errorMessage = DynamicLibrary::dlerror();
            if ((address == NULL) && errorMessage.empty()) {
                lastErrorMessage_ = errorMessage;
                errorCode_ = LoaderError::SYMBOL_RESOLUTION_FAILED;
            }
            else {
                using namespace boost;
                typedef typename add_pointer<Signature>::type function_pointer;
                function = reinterpret_cast<function_pointer>(address);
                errorCode_.clear();
            }
        }
        else {
            lastErrorMessage_.clear();
            errorCode_ = LoaderError::LIBRARY_NOT_LOADED;
        }

        return function;
    }

    template<typename Type>
    Type* DynamicLibrary::getVariable(void* handle, const std::string &symbol)
    {
        if (isLoad()) {
            DynamicLibrary::dlerror();
            void* address = DynamicLibrary::dlsym(handle, symbol.c_str());
            std::string errorMessage = DynamicLibrary::dlerror();
            if ((address == NULL) && errorMessage.empty()) {
                lastErrorMessage_ = errorMessage;
                errorCode_ = LoaderError::SYMBOL_RESOLUTION_FAILED;
            }
            else {
                errorCode_.clear();
            }

            return reinterpret_cast<Type*>(address);
        }
        else {
            lastErrorMessage_.clear();
            errorCode_ = LoaderError::LIBRARY_NOT_LOADED;
            return NULL;
        }
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
// Enable automatic conversion from LoaderError::LoaderErrorType enum to
// error_code constants.
//

namespace boost { namespace system
{
    template <>
    struct is_error_code_enum<dl::LoaderError::LoaderErrorType>
        : public true_type {};
}}

#endif /* DL_HPP_ */
