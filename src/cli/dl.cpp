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

#include <boost/system/error_code.hpp>

#include <cli/dl.hpp>

namespace dl
{
    using namespace boost;

    //
    // Error handling support
    //

    namespace loader_error
    {

        system::error_code make_error_code(LoaderError e)
        {
            return system::error_code(static_cast<int>(e), loaderCategory());
        }

        system::error_condition make_error_condition(LoaderError e)
        {
            return system::error_condition(static_cast<int>(e),
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
        case loader_error::LIBRARY_LOAD_FAILED:
            return "Failed to load dynamic library";
        case loader_error::SYMBOL_RESOLUTION_FAILED:
            return "Failed to resolve symbol";
        default:
            return "Unknown dynamic linking loader error";
        }
    }

    system::error_condition
    LoaderCategory::default_error_condition(int ev) const
    {
        switch (ev) {
        case loader_error::LIBRARY_LOAD_FAILED:
            return system::errc::permission_denied;
        case loader_error::SYMBOL_RESOLUTION_FAILED:
            return system::errc::address_not_available;
        default:
            return system::error_condition(ev, *this);
        }
    }

    system::error_category& loaderCategory()
    {
        static LoaderCategory instance;
        return instance;
    }

    //
    // Class DynamicLibrary
    //

    DynamicLibrary::DynamicLibrary(const std::string& fileName,
        OpenFlags flags)
    {
        if ((flags & LAZY_BINDING) == 0) {
            flags |= ONLOAD_BINDING;
        }

        const char* c_fileName = fileName.empty() ? NULL : fileName.c_str();
        libraryHandle_ = DynamicLibrary::dlopen(c_fileName, flags);
        if (libraryHandle_ == NULL) {
            lastErrorMessage_ = DynamicLibrary::dlerror();
            errorCode_ = loader_error::LIBRARY_LOAD_FAILED;
        }
    }

    DynamicLibrary::~DynamicLibrary()
    {
        if (isOpen()) {
            DynamicLibrary::dlclose(libraryHandle_);
        }
    }
}
