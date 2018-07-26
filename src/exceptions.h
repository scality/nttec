/*
 * Copyright 2017-2018 Scality
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __QUAD_EXCEPTIONS_H__
#define __QUAD_EXCEPTIONS_H__

#include <stdexcept>

namespace quad {

/** Base class for all the exceptions generated by QuadIron. */
class Exception : public std::runtime_error {
  public:
    explicit Exception(const std::string& reason) : std::runtime_error(reason)
    {
    }
    explicit Exception(const char*& reason) : std::runtime_error(reason) {}
};

/** A faulty logic within the program (a programmer's mistake). */
class LogicError : public Exception {
  public:
    explicit LogicError(const std::string& reason) : Exception(reason) {}
    explicit LogicError(const char* reason) : Exception(reason) {}
};

/** Report an invalid argument. */
class InvalidArgument : public LogicError {
  public:
    explicit InvalidArgument(const std::string& reason) : LogicError(reason) {}
    explicit InvalidArgument(const char* reason) : LogicError(reason) {}
};

/** A domain error. */
class DomainError : public Exception {
  public:
    explicit DomainError(const std::string& reason) : Exception(reason) {}
    explicit DomainError(const char* reason) : Exception(reason) {}
};

/** Raised when no solution exists. */
class NoSolution : public Exception {
  public:
    explicit NoSolution(const std::string& reason) : Exception(reason) {}
    explicit NoSolution(const char* reason) : Exception(reason) {}
};

} // namespace quad

#endif
