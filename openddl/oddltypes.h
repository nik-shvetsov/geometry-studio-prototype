/*
  OpenDDL Library Software License
  ==================================

  OpenDDL Library, version 1.1
  Copyright 2014-2015, Eric Lengyel
  All rights reserved.

  The OpenDDL Library is free software published on the following website:

    http://openddl.org/

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the entire text of this license,
  comprising the above copyright notice, this list of conditions, and the following
  disclaimer.

  2. Redistributions of any modified source code files must contain a prominent
  notice immediately following this license stating that the contents have been
  modified from their original form.

  3. Redistributions in binary form must include attribution to the author in any
  listing of credits provided with the distribution. If there is no listing of
  credits, then attribution must be included in the documentation and/or other
  materials provided with the distribution. The attribution must be exactly the
  statement "This software contains the OpenDDL Library by Eric Lengyel" (without
  quotes) in the case that the distribution contains the original, unmodified
  OpenDDL Library, or it must be exactly the statement "This software contains a
  modified version of the OpenDDL Library by Eric Lengyel" (without quotes) in the
  case that the distribution contains a modified version of the OpenDDL Library.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/



/* MODIFIED */

#ifndef ODDLTypes_h
#define ODDLTypes_h


#include <math.h>
#include <new>


namespace ODDL
{
  #define restrict __restrict


  using int8            = signed char;
  using unsigned_int8  = unsigned char;

  using int16           = short;
  using unsigned_int16  = unsigned short;

  using int32           = int;
  using unsigned_int32  = unsigned int;

  #if defined(_WIN64)


    using int64                 = __int64;
    using unsigned_int64        = unsigned __int64;

    using machine               = __int64;
    using unsigned_machine      = unsigned __int64;

    using machine_int           = __int64;
    using unsigned_machine_int  = unsigned __int64;

  #else

    using int64                 = long;
    using unsigned_int64        = unsigned long;

    using machine               = long;
    using unsigned_machine      = unsigned long;

    using machine_int           = long;
    using unsigned_machine_int  = unsigned long;

  #endif


  inline int32 Abs(int32 x)
  {
    int32 a = x >> 31;
    return ((x ^ a) - a);
  }

  inline int32 Min(int32 x, int32 y)
  {
    int32 a = x - y;
    return (x - (a & ~(a >> 31)));
  }

  inline int32 Max(int32 x, int32 y)
  {
    int32 a = x - y;
    return (x - (a & (a >> 31)));
  }

  inline int32 MinZero(int32 x)
  {
    return (x & (x >> 31));
  }

  inline int32 MaxZero(int32 x)
  {
    return (x & ~(x >> 31));
  }


  template <class type> class AutoDelete
  {
    private:

      type  *reference;

      AutoDelete(const AutoDelete&) = delete;

    public:

      explicit AutoDelete(type *ptr)
      {
        reference = ptr;
      }

      ~AutoDelete()
      {
        delete reference;
      }

      operator type *(void) const
      {
        return (reference);
      }

      type *const *operator &(void) const
      {
        return (&reference);
      }

      type *operator ->(void) const
      {
        return (reference);
      }

      AutoDelete& operator =(type *ptr)
      {
        reference = ptr;
        return (*this);
      }
  };
}


#endif
