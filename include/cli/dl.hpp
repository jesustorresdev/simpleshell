/*
 * dl.hpp - Programming interface to the dynamic linking loader
 *
 *   Copyright 2010-2016 Jesús Torres <jmtorres@ull.es>
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
#include <functional>
#include <memory>
#include <string>
#include <system_error>
#include <type_traits>

namespace dl
{
    namespace posix {
        #include <dlfcn.h>
    }

    //
    // Error handling support
    //

    enum class LoaderError
    {
        LIBRARY_ALREADY_LOADED = 1,
        LIBRARY_LOAD_FAILED,
        LIBRARY_NOT_LOADED,
        SYMBOL_RESOLUTION_FAILED,
    };

    class LoaderCategory : public std::error_category
    {
        virtual const char* name() const noexcept;
        virtual std::string message(int ev) const;
        virtual std::error_condition default_error_condition(int ev) const noexcept;
    };

    std::error_category& loaderCategory();

    std::error_code make_error_code(LoaderError e) noexcept;
    std::error_condition make_error_condition(LoaderError e) noexcept;

    //
    // Loader options and flags
    //

    enum class OpenFlags
    {
        LAZY_BINDING        = RTLD_LAZY,
        ONLOAD_BINDING      = RTLD_NOW,
        GLOBAL_BINDING      = RTLD_GLOBAL,
        LOCAL_BINDING       = RTLD_LOCAL,
#if defined(__GLIBC__)
# if __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2)
        NOUNLOAD_ONCLOSE    = RTLD_NODELETE,
        NOLOAD_ONOPEN       = RTLD_NOLOAD,
# endif
# if __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ > 3)
        DEEP_BINDING        = RTLD_DEEPBIND,
# endif
#endif /* __GLIBC__ */
    };

    enum class SpecialFileNames
    {
        MAIN_PROGRAM
    };

    //
    // Class DynamicLibrary
    //

    class DynamicLibrary
    {
        public:

            DynamicLibrary();

            template <typename T>
            DynamicLibrary(T const& fileName,
                OpenFlags flags = OpenFlags::ONLOAD_BINDING);

            void load(const char* fileName,
                OpenFlags flags = OpenFlags::ONLOAD_BINDING);
            void load(const std::string& fileName,
                OpenFlags flags = OpenFlags::ONLOAD_BINDING);
            void load(SpecialFileNames fileName,
                OpenFlags flags = OpenFlags::ONLOAD_BINDING);

            //
            // Error handling
            //

            bool isLoad() const
                { return static_cast<bool>(libraryHandle_); }

            std::error_code lastError() const
                { return errorCode_; }

            const std::string lastErrorMessage() const
                { return lastErrorMessage_; }

            //
            // Members for symbol resolution
            //

            template <typename Type>
            Type*& resolve(Type*& pointer, const std::string& symbol);

            template <typename Type>
            Type* resolve(const std::string& symbol);

            template <typename Signature>
            std::function<Signature>&
            resolveFunction(std::function<Signature>& function,
                const std::string& symbol);

            template <typename Signature>
            std::function<Signature>
            resolveFunction(const std::string& symbol);

#if defined(_GNU_SOURCE)
            template <typename Type>
            Type*& resolveDefault(const std::string& symbol, Type*& pointer)
            {
                return resolve<Type>(pointer, RTLD_DEFAULT, symbol);
            }

            template <typename Type>
            Type* resolveDefault(const std::string& symbol)
            {
                Type* pointer;
                return resolve<Type>(pointer, RTLD_DEFAULT, symbol);
            }

            template <typename Type>
            Type*& resolveNext(const std::string& symbol, Type*& pointer)
            {
                return resolve<Type>(pointer, RTLD_NEXT, symbol);
            }

            template <typename Type>
            Type* resolveNext(const std::string& symbol)
            {
                Type* pointer;
                return resolve<Type>(pointer, RTLD_NEXT, symbol);
            }

            template <typename Signature>
            std::function<Signature>&
            resolveDefaultFunction(std::function<Signature>& function,
                const std::string& symbol)
            {
                return resolveFunction<Signature>(function, RTLD_DEFAULT,
                    symbol);
            }

            template <typename Signature>
            std::function<Signature>
            resolveDefaultFunction(const std::string& symbol)
            {
                std::function<Signature> function;
                return resolveFunction<Signature>(function, RTLD_DEFAULT,
                    symbol);
            }

            template <typename Signature>
            std::function<Signature>&
            resolveNextFunction(std::function<Signature>& function,
                const std::string& symbol)
            {
                return resolveFunction<Signature>(function, RTLD_NEXT, symbol);
            }

            template <typename Signature>
            std::function<Signature>
            resolveNextFunction(const std::string& symbol)
            {
                std::function<Signature> function;
                return resolveFunction<Signature>(function, RTLD_NEXT, symbol);
            }
#endif /* _GNU_SOURCE */

            //
            // C library function wrappers
            //

            static void* dlopen(const std::string& fileName, OpenFlags flags)
            {
                return posix::dlopen(fileName.c_str(), static_cast<int>(flags));
            }

            static void* dlopen(const char* fileName, OpenFlags flags)
            {
                return posix::dlopen(fileName, static_cast<int>(flags));
            }

            static int dlclose(void* handle)
            {
                return posix::dlclose(handle);
            }

            static void* dlsym(void* handle, const std::string &symbol)
            {
                return posix::dlsym(handle, symbol.c_str());
            }

            static void* dlsym(void* handle, const char* symbol)
            {
                return posix::dlsym(handle, symbol);
            }

            static std::string dlerror();

        protected:
            std::error_code errorCode_;

        private:
            typedef std::unique_ptr<void, decltype(&dlclose)> LibraryHandle;

            LibraryHandle libraryHandle_;
            std::string lastErrorMessage_;

            //
            // Private members for symbol resolution
            //

            template <typename Type>
            Type*& resolve(Type*& pointer, const LibraryHandle& handle,
                const std::string& symbol);

            template <typename Signature>
            std::function<Signature>&
            resolveFunction(std::function<Signature>& function,
                const LibraryHandle& handle, const std::string& symbol);
    };

    template <typename T>
    DynamicLibrary::DynamicLibrary(T const& fileName, OpenFlags flags)
    {
        load(fileName, flags);
    }

    inline std::string DynamicLibrary::dlerror()
    {
        const char *message = posix::dlerror();
        return (message == nullptr) ? std::string() : std::string(message);
    }

    template <typename Type>
    Type*& DynamicLibrary::resolve(Type*& pointer, const std::string& symbol)
    {
        if (isLoad()) {
            resolve<Type>(pointer, libraryHandle_, symbol);
        }
        else {
            lastErrorMessage_.clear();
            errorCode_ = LoaderError::LIBRARY_NOT_LOADED;
        }

        return pointer;
    }


    template <typename Type>
    Type* DynamicLibrary::resolve(const std::string& symbol)
    {
        Type* pointer;
        return resolve<Type>(pointer, symbol);
    }

    template <typename Signature>
    std::function<Signature>& DynamicLibrary::resolveFunction(
        std::function<Signature>& function, const std::string& symbol)
    {
        if (isLoad()) {
            resolveFunction<Signature>(function, libraryHandle_, symbol);
        }
        else {
            lastErrorMessage_.clear();
            errorCode_ = LoaderError::LIBRARY_NOT_LOADED;
        }

        return function;
    }

    template <typename Signature>
    std::function<Signature> DynamicLibrary::resolveFunction(
        const std::string& symbol)
    {
        std::function<Signature> function;
        return resolveFunction<Signature>(function, symbol);
    }

    //
    // Private members for symbol resolution
    //

    template <typename Type>
    Type*& DynamicLibrary::resolve(Type*& pointer, const LibraryHandle& handle,
        const std::string& symbol)
    {
        DynamicLibrary::dlerror();
        void* address = DynamicLibrary::dlsym(handle.get(), symbol.c_str());
        std::string errorMessage = DynamicLibrary::dlerror();
        if ((address == nullptr) && errorMessage.empty()) {
            lastErrorMessage_ = errorMessage;
            errorCode_ = LoaderError::SYMBOL_RESOLUTION_FAILED;
        }
        else {
            pointer = reinterpret_cast<Type*>(address);
            errorCode_.clear();
        }

        return pointer;
    }

    template <typename Signature>
    std::function<Signature>&
    DynamicLibrary::resolveFunction(std::function<Signature>& function,
        const LibraryHandle& handle, const std::string& symbol)
    {
        typename std::add_pointer<Signature>::type address;
        resolve<Signature>(address, handle, symbol);
        if (! errorCode_) {
            function = address;
        }

        return function;
    }

    //
    // Boolean operators overload for OpenFlags
    //

    inline OpenFlags operator&(OpenFlags a, OpenFlags b)
    {
        return OpenFlags(static_cast<int>(a) & static_cast<int>(b));
    }

    inline OpenFlags operator|(OpenFlags a, OpenFlags b)
    {
        return OpenFlags(static_cast<int>(a) | static_cast<int>(b));
    }

    inline OpenFlags operator^(OpenFlags a, OpenFlags b)
    {
        return OpenFlags(static_cast<int>(a) ^ static_cast<int>(b));
    }

    inline OpenFlags operator&=(OpenFlags &a, OpenFlags b)
    {
        return a = a & b;
    }

    inline OpenFlags operator|=(OpenFlags &a, OpenFlags b)
    {
        return a = a | b;
    }

    inline OpenFlags operator^=(OpenFlags a, OpenFlags b)
    {
        return a = a ^ b;
    }

    inline OpenFlags operator~(OpenFlags a)
    {
        return OpenFlags(~static_cast<int>(a));
    }
}

//
// Enable automatic conversion from LoaderError::LoaderErrorType enum to
// error_code constants.
//

namespace std
{
    template <>
    struct is_error_code_enum<dl::LoaderError> : public true_type {};
}

#endif /* DL_HPP_ */
