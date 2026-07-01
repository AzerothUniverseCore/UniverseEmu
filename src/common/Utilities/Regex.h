/*
 * This file is part of the SyphrenaCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SyphrenaCore_Regex_h__
#define SyphrenaCore_Regex_h__

// std::wregex doesn't work with patterns provided in db2 files
// so we have to use boost
#include <boost/regex.hpp>
#define SC_REGEX_NAMESPACE boost

namespace Syphrena
{
    using regex = SC_REGEX_NAMESPACE :: regex;
    using wregex = SC_REGEX_NAMESPACE :: wregex;

    using :: SC_REGEX_NAMESPACE :: regex_match;
    using :: SC_REGEX_NAMESPACE :: regex_search;
}

#endif // SyphrenaCore_Regex_h__
