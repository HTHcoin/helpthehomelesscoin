 /*
  inttypes.h

  Contributors:
    Created by Marek Michalkiewicz <marekm@linux.org.pl>

  THIS SOFTWARE IS NOT COPYRIGHTED

  This source code is offered for use in the public domain.  You may
 use, modify or distribute it freely.

 This code is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY.  ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 DISCLAIMED.  This includes but is not limited to warranties of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

 #ifndef __INTTYPES_H_
 #define __INTTYPES_H_

	#include <stdint.h>

 /* Use [u]intN_t if you need exactly N bits.
  XXX - doesn't handle the -mint8 option.  */

 typedef int8_t swift_int8_t;
 typedef uint8_t swift_uint8_t;

 typedef int16_t swift_int16_t;
 typedef uint16_t swift_uint16_t;

 typedef int32_t swift_int32_t;
 typedef uint32_t swift_uint32_t;

 typedef int64_t swift_int64_t;
 typedef uint64_t swift_uint64_t;

 //typedef swift_int16_t intptr_t;
 //typedef swift_uint16_t uintptr_t;

 #endif
