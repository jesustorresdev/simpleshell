/*
 * glob.cpp - Find pathnames matching a pattern
 *
 *   Copyright 2010-2012 Jesús Torres <jmtorres@ull.es>
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
#include <utility>
#include <vector>

#include <boost/thread/tss.hpp>

#include <cli/internals.hpp>

namespace glob {
    // onGlobError() must be declared before declaring Glob class
    extern "C" int onGlobError(const char *epath, int eerrno);
}

#include <cli/glob.hpp>

namespace glob
{
    namespace posix {
        #include <glob.h>
    }

    using namespace boost;

    //
    // POSIX function glob() will call onGlobError() on error. This uses
    // the TSS variable currentGlobObject to remember the Glob object from
    // which the glob() function was invoked.
    //

    thread_specific_ptr<Glob> currentGlobObject;

    extern "C" int onGlobError(const char *epath, int eerrno)
    {
        return currentGlobObject->onError(epath,
            std::error_code(eerrno, std::system_category()));
    }

    //
    // Class Glob
    //

    Glob::Glob(const std::string& pattern, GlobFlags flags)
    {
        currentGlobObject.reset(this);

        posix::glob_t glob;
        posix::glob(pattern.c_str(), flags, &onGlobError, &glob);

        currentGlobObject.release();

        if (glob.gl_pathc) {
            for (char** p = glob.gl_pathv; *p != NULL; ++p) {
                pathNames_.push_back(*p);
            }
        }

        posix::globfree(&glob);
    }

    bool Glob::onError(const std::string& pathName,
        std::error_code errorCode)
    {
        errors_.push_back(std::make_pair(pathName, errorCode));
        return false;
    }

    std::string Glob::escape(const std::string& pattern)
    {
        std::string escaped;

        for (std::string::const_iterator i = pattern.begin();
            i < pattern.end(); ++i)
        {
            switch (*i) {
            case '~':   // EXPAND_TILDE
            case '*':
            case '?':
            case '[':
            case '\\':
                escaped.push_back('\\');
            default:
                escaped.push_back(*i);
            }
        }

        return escaped;
    }
}
