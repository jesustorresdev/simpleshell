/*
 * glob.hpp - Find pathnames matching a pattern
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

#ifndef GLOB_HPP_
#define GLOB_HPP_

#include <string>
#include <utility>
#include <vector>

#include <boost/system/error_code.hpp>
#include <boost/filesystem.hpp>

namespace glob
{
    namespace posix {
        #include <glob.h>
    }

    using namespace boost;

    //
    // Class Glob
    //

    class Glob
    {
        public:

            typedef std::vector<std::pair<
                std::string, system::error_code> > ErrorsType;

            enum GlobFlags
            {
                NONE                                = 0,
                STOP_ON_ERRORS                      = GLOB_ERR,
                END_DIRECTORIES_WITH_PATH_SEPARATOR = GLOB_MARK,
                NO_PATH_NAMES_SORT                  = GLOB_NOSORT,
                NO_PATH_NAMES_CHECK                 = GLOB_NOCHECK,
                NO_ESCAPE_CHARACTER                 = GLOB_NOESCAPE,
                NO_MAGIC                            = GLOB_NOMAGIC,
                MATCH_LEADING_PERIOD                = GLOB_PERIOD,
                FIND_DIRECTORIES_ONLY               = GLOB_ONLYDIR,
                EXPAND_BRACE_EXPRESSIONS            = GLOB_BRACE,
                EXPAND_TILDE                        = GLOB_TILDE,
                EXPAND_TILDE_WITH_CHECK             = GLOB_TILDE_CHECK
            };

            Glob(const std::string& pattern, GlobFlags flags = NONE);

            //
            // Overloaded operators to assign the list of path names found
            //

            operator const std::vector<std::string>&() const;
            operator std::vector<std::string>() const;
            operator std::vector<filesystem::path>() const;

            //
            // Error handling
            //

            const ErrorsType& getErrors() const
                { return errors_; }

            //
            // Meta characters escaping
            //

            static std::string escape(const std::string& pattern);

        protected:
             ErrorsType errors_;

             bool onError(const std::string& pathName,
                 system::error_code errorCode);

        private:
             std::vector<std::string> pathNames_;

             friend int onGlobError(const char *epath, int eerrno);
    };

    inline Glob::operator const std::vector<std::string>&() const
    {
        return pathNames_;
    }

    inline Glob::operator std::vector<std::string>() const
    {
        return pathNames_;
    }

    inline Glob::operator std::vector<filesystem::path>() const
    {
        std::vector<filesystem::path> pathNames;
        std::vector<std::string>::const_iterator iter;
        for (iter = pathNames_.begin(); iter < pathNames_.end(); ++iter) {
            pathNames.push_back(filesystem::path(*iter));
        }
        return pathNames;
    }

    //
    // Boolean operators overload for Glob::GlobFlags
    //

    inline Glob::GlobFlags
    operator&(Glob::GlobFlags a, Glob::GlobFlags b)
    {
        return Glob::GlobFlags(static_cast<int>(a)
            & static_cast<int>(b));
    }

    inline Glob::GlobFlags
    operator|(Glob::GlobFlags a, Glob::GlobFlags b)
    {
        return Glob::GlobFlags(static_cast<int>(a)
            | static_cast<int>(b));
    }

    inline Glob::GlobFlags
    operator^(Glob::GlobFlags a, Glob::GlobFlags b)
    {
        return Glob::GlobFlags(static_cast<int>(a)
            ^ static_cast<int>(b));
    }

    inline Glob::GlobFlags
    operator&=(Glob::GlobFlags &a, Glob::GlobFlags b)
    {
        return a = a & b;
    }

    inline Glob::GlobFlags
    operator|=(Glob::GlobFlags &a, Glob::GlobFlags b)
    {
        return a = a | b;
    }

    inline Glob::GlobFlags
    operator^=(Glob::GlobFlags a, Glob::GlobFlags b)
    {
        return a = a ^ b;
    }

    inline Glob::GlobFlags
    operator~(Glob::GlobFlags a)
    {
        return Glob::GlobFlags(~static_cast<int>(a));
    }
}

#endif /* GLOB_HPP_ */
