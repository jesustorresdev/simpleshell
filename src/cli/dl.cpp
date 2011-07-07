/*
 * dl.cpp - Programming interface to the dynamic linking loader
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

#include <string>

#include <cli/dl.hpp>
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

    namespace loader_error
    {

        std::error_code make_error_code(LoaderError e)
        {
            return std::error_code(static_cast<int>(e), loaderCategory());
        }

        std::error_condition make_error_condition(LoaderError e)
        {
            return std::error_condition(static_cast<int>(e),
                loaderCategory());
        }
    }

    //
    // Class LoaderCateogry
    //

    const char* LoaderCategory::name() const
    {
        return "dl";
    }

    std::string LoaderCategory::message(int ev) const
    {
        switch (ev) {
        case loader_error::LIBRARY_ALREADY_LOADED:
            return "A dynamic library is already loaded";
        case loader_error::LIBRARY_LOAD_FAILED:
            return "Failed to load dynamic library";
        case loader_error::LIBRARY_NOT_LOADED:
            return "No loaded dynamic library";
        case loader_error::SYMBOL_RESOLUTION_FAILED:
            return "Failed to resolve symbol";
        default:
            return "Unknown dynamic linking loader error";
        }
    }

    std::error_condition
    LoaderCategory::default_error_condition(int ev) const
    {
        switch (ev) {
        case loader_error::LIBRARY_ALREADY_LOADED:
            return std::errc::address_in_use;
        case loader_error::LIBRARY_LOAD_FAILED:
            return std::errc::permission_denied;
        case loader_error::LIBRARY_NOT_LOADED:
            return std::errc::address_not_available;
        case loader_error::SYMBOL_RESOLUTION_FAILED:
            return std::errc::address_not_available;
        default:
            return std::error_condition(ev, *this);
        }
    }

    std::error_category& loaderCategory()
    {
        static LoaderCategory instance;
        return instance;
    }

    //
    // Class DynamicLibrary
    //

    DynamicLibrary::DynamicLibrary() : libraryHandle_(NULL) {}

    DynamicLibrary::~DynamicLibrary()
    {
        if (isLoad()) {
            DynamicLibrary::dlclose(libraryHandle_);
        }
    }

    void DynamicLibrary::load(const char *fileName, OpenFlags flags)
    {
        if (isLoad()) {
            lastErrorMessage_.clear();
            errorCode_ = loader_error::LIBRARY_ALREADY_LOADED;
            return;
        }

        if ((flags & LAZY_BINDING) == 0) {
            flags |= ONLOAD_BINDING;
        }

        libraryHandle_ = DynamicLibrary::dlopen(fileName, flags);
        if (libraryHandle_ == NULL) {
            lastErrorMessage_ = DynamicLibrary::dlerror();
            errorCode_ = loader_error::LIBRARY_LOAD_FAILED;
        }
    }

    void DynamicLibrary::load(const std::string& fileName, OpenFlags flags)
    {
        if (! fileName.empty()) {
            load(fileName, flags);
        }
    }

    void DynamicLibrary::load(enum SpecialFileNames fileName, OpenFlags flags)
    {
        if (fileName == MAIN_PROGRAM) {
            load(NULL, flags);
        }
    }
}
