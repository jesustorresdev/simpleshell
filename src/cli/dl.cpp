/*
 * dl.cpp - Programming interface to the dynamic linking loader
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

#include <string>

#define gettext(str) str    // TODO: Use Boost.Locale when available

#include <cli/detail/utility.hpp>
#include <cli/dl.hpp>

namespace dl
{
    namespace posix {
        #include <dlfcn.h>
    }

    //
    // Error handling support
    //

    std::error_code make_error_code(LoaderError e) noexcept
    {
        return std::error_code(static_cast<int>(e), loaderCategory());
    }

    std::error_condition make_error_condition(LoaderError e) noexcept
    {
        return std::error_condition(static_cast<int>(e), loaderCategory());
    }

    //
    // Class LoaderCateogry
    //

    const char* LoaderCategory::name() const noexcept
    {
        return "dl";
    }

    std::string LoaderCategory::message(int ev) const
    {
        if (LoaderError(ev) == LoaderError::LIBRARY_ALREADY_LOADED) {
            return gettext("A dynamic library is already loaded");
        }
        if (LoaderError(ev) == LoaderError::LIBRARY_LOAD_FAILED) {
            return gettext("Failed to load dynamic library");
        }
        if (LoaderError(ev) == LoaderError::LIBRARY_NOT_LOADED) {
            return gettext("No loaded dynamic library");
        }
        if (LoaderError(ev) == LoaderError::SYMBOL_RESOLUTION_FAILED) {
            return gettext("Failed to resolve symbol");
        }
        return gettext("Unknown dynamic linking loader error");
    }

    std::error_condition
    LoaderCategory::default_error_condition(int ev) const noexcept
    {
        if (LoaderError(ev) == LoaderError::LIBRARY_LOAD_FAILED) {
            return std::errc::permission_denied;
        }
        if (LoaderError(ev) == LoaderError::SYMBOL_RESOLUTION_FAILED) {
            return std::errc::address_not_available;
        }
        return std::error_condition(ev, *this);
    }

    std::error_category& loaderCategory()
    {
        static LoaderCategory instance;
        return instance;
    }

    //
    // Class DynamicLibrary
    //

    DynamicLibrary::DynamicLibrary() : libraryHandle_(nullptr, &dlclose) {}

    void DynamicLibrary::load(const char *fileName, OpenFlags flags)
    {
        if (isLoad()) {
            lastErrorMessage_.clear();
            errorCode_ = LoaderError::LIBRARY_ALREADY_LOADED;
            return;
        }

        if (! static_cast<bool>(flags & OpenFlags::LAZY_BINDING)) {
            flags |= OpenFlags::ONLOAD_BINDING;
        }

        void* handler = DynamicLibrary::dlopen(fileName, flags);
        if (handler != nullptr) {
            libraryHandle_ = LibraryHandle(handler, &dlclose);
        }
        else {
            lastErrorMessage_ = DynamicLibrary::dlerror();
            errorCode_ = LoaderError::LIBRARY_LOAD_FAILED;
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
        if (fileName == SpecialFileNames::MAIN_PROGRAM) {
            load(NULL, flags);
        }
    }
}
