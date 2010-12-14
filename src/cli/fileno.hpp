/*
 * fileno.cpp - Similar to fileno(3), but taking a C++ stream
 *
 *  http://www.ginac.de/~kreckel/fileno/
 *      Author: Richard B. Kreckel
 *
 *  This code is in the public domain.
 */

#ifndef FILENO_H_
#define FILENO_H_

#include <iosfwd>

template <typename charT, typename traits>
int fileno(const std::basic_ios<charT, traits>& stream);

#endif /* FILENO_H_ */
