/*
 Print.cpp - Base class that provides print() and println()
 Copyright (c) 2008 David A. Mellis.  All right reserved.
 many modifications, by Paul Stoffregen <paul@pjrc.com>
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 
 Modified 23 November 2006 by David A. Mellis
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <avr/pgmspace.h>
#include "Arduino.h"      // (was wiring.h)

#include "Print.h"


#if ARDUINO >= 100
#else
void Print::write(const char *str)
{
	write((const uint8_t *)str, strlen(str));
}
#endif


#if ARDUINO >= 100
size_t Print::write(const uint8_t *buffer, size_t size)
{
	size_t count = 0;
	while (size--) count += write(*buffer++);
	return count;
}
#else
void Print::write(const uint8_t *buffer, size_t size)
{
	while (size--) write(*buffer++);
}
#endif


#if ARDUINO >= 100
size_t Print::print(const String &s)
{
	uint8_t buffer[33];
	size_t count = 0;
	unsigned int index = 0;
	unsigned int len = s.length();
	while (len > 0) {
		s.getBytes(buffer, sizeof(buffer), index);
		unsigned int nbytes = len;
		if (nbytes > sizeof(buffer)-1) nbytes = sizeof(buffer)-1;
		index += nbytes;
		len -= nbytes;
		count += write(buffer, nbytes);
	}
	return count;
}
#else
void Print::print(const String &s)
{
	unsigned int len = s.length();
	for (unsigned int i=0; i < len; i++) {
		write(s[i]);
	}
}
#endif


#if ARDUINO >= 100
size_t Print::print(const __FlashStringHelper *ifsh)
{
	uint8_t buffer[32];
	size_t count = 0;
	const char PROGMEM *p = (const char PROGMEM *)ifsh;
	unsigned int len = strlen_P(p);
	while (len > 0) {
		unsigned int nbytes = len;
		if (nbytes > sizeof(buffer)) nbytes = sizeof(buffer);
		memcpy_P(buffer, p, nbytes);
		p += nbytes;
		len -= nbytes;
		count += write(buffer, nbytes);
	}
	return count;
}
#else
void Print::print(const __FlashStringHelper *ifsh)
{
	const char PROGMEM *p = (const char PROGMEM *)ifsh;
	while (1) {
		unsigned char c = pgm_read_byte(p++);
		if (c == 0) return;
		write(c);
	}
}
#endif


#if ARDUINO >= 100
size_t Print::print(long n)
{
	uint8_t sign=0;

	if (n < 0) {
		sign = 1;
		n = -n;
	}
	return printNumber(n, sign, 10);
}
#else
void Print::print(long n)
{
	uint8_t sign=0;

	if (n < 0) {
		sign = 1;
		n = -n;
	}
	printNumber(n, sign, 10);
}
#endif


#if ARDUINO >= 100
size_t Print::println(void)
{
	uint8_t buf[2]={'\r', '\n'};
	return write(buf, 2);
}
#else
void Print::println(void)
{
	uint8_t buf[2]={'\r', '\n'};
	write(buf, 2);
}
#endif


//#define USE_HACKER_DELIGHT_OPTIMIZATION
#define USE_STIMMER_OPTIMIZATION
#define USE_BENCHMARK_CODE


#ifdef USE_HACKER_DELIGHT_OPTIMIZATION
// Adapted from Hacker's Delight (Henry Warren, ISBN 0321842685) www.hackersdelight.org
// by Rob Tillaart, Tom Carpenter, "genom2" with input from others...
// http://forum.arduino.cc/index.php?topic=167414.0
//
#define divmod10_asm(in32, tmp32, mod8)				\
asm volatile (							\
	"mov	%2, %A0		\n\t" /* mod = in */		\
	"ori	%A0, 1		\n\t" /* q = in | 1 */		\
	"movw	%A1, %A0	\n\t" /* x = q */		\
	"movw	%C1, %C0	\n\t"				\
	"lsr	%D1		\n\t" /* x = x >> 2 */		\
	"ror	%C1		\n\t"				\
	"ror	%B1		\n\t"				\
	"ror	%A1		\n\t"				\
	"lsr	%D1		\n\t"  				\
	"ror	%C1		\n\t"				\
	"ror	%B1		\n\t"				\
	"ror	%A1		\n\t"				\
	"sub	%A0, %A1	\n\t" /* q = q - x  */		\
	"sbc	%B0, %B1	\n\t"				\
	"sbc	%C0, %C1	\n\t"				\
	"sbc	%D0, %D1	\n\t"				\
	"movw	%A1, %A0	\n\t" /* x = q  */		\
	"movw	%C1, %C0	\n\t"				\
	"lsr	%D1		\n\t" /* x = x >> 4  */		\
	"ror	%C1		\n\t"				\
	"ror	%B1		\n\t"				\
	"ror	%A1		\n\t"				\
	"lsr	%D1		\n\t"				\
	"ror	%C1		\n\t"				\
	"ror	%B1		\n\t"				\
	"ror	%A1		\n\t"				\
	"lsr	%D1		\n\t"				\
	"ror	%C1		\n\t"				\
	"ror	%B1		\n\t"				\
	"ror	%A1		\n\t"				\
	"lsr	%D1		\n\t"				\
	"ror	%C1		\n\t"				\
	"ror	%B1		\n\t"				\
	"ror	%A1		\n\t"				\
	"add	%A1, %A0	\n\t" /* x = x + q */		\
	"adc	%B1, %B0	\n\t"				\
	"adc	%C1, %C0	\n\t"				\
	"adc	%D1, %D0	\n\t"				\
	"movw	%A0, %A1	\n\t" /* q = x */		\
	"movw	%C0, %C1	\n\t"				\
	"add	%A0, %B1	\n\t" /* q = q + (x >> 8) */	\
	"adc	%B0, %C1	\n\t"				\
	"adc	%C0, %D1	\n\t"				\
	"adc	%D0, r1		\n\t"				\
	"mov	%A0, %B0	\n\t" /* q = q >> 8 */		\
	"mov	%B0, %C0	\n\t"				\
	"mov	%C0, %D0	\n\t"				\
	"eor	%D0, %D0	\n\t"				\
	"add	%A0, %A1	\n\t" /* q = q + x */		\
	"adc	%B0, %B1	\n\t"				\
	"adc	%C0, %C1	\n\t"				\
	"adc	%D0, %D1	\n\t"				\
	"mov	%A0, %B0	\n\t" /* q = q >> 8 */		\
	"mov	%B0, %C0	\n\t"				\
	"mov	%C0, %D0	\n\t"				\
	"eor	%D0, %D0	\n\t"				\
	"add	%A0, %A1	\n\t" /* q = q + x */		\
	"adc	%B0, %B1	\n\t"				\
	"adc	%C0, %C1	\n\t"				\
	"adc	%D0, %D1	\n\t"				\
	"mov	%A0, %B0	\n\t" /* q = q >> 8 */		\
	"mov	%B0, %C0	\n\t"				\
	"mov	%C0, %D0	\n\t"				\
	"eor	%D0, %D0	\n\t"				\
	"add	%A0, %A1	\n\t" /* q = q + x */		\
	"adc	%B0, %B1	\n\t"				\
	"adc	%C0, %C1	\n\t"				\
	"adc	%D0, %D1	\n\t"				\
	"andi	%A0, 0xF8	\n\t" /* q = q & ~0x7 */	\
	"sub	%2, %A0		\n\t" /* mod = mod - q */	\
	"lsr	%D0		\n\t" /* q = q >> 2  */		\
	"ror	%C0		\n\t"				\
	"ror	%B0		\n\t"				\
	"ror	%A0		\n\t"				\
	"lsr	%D0		\n\t"				\
	"ror	%C0		\n\t"				\
	"ror	%B0		\n\t"				\
	"ror	%A0		\n\t"				\
	"sub	%2, %A0		\n\t" /* mod = mod - q */	\
	"lsr	%D0		\n\t" /* q = q >> 1 */		\
	"ror	%C0		\n\t"				\
	"ror	%B0		\n\t"				\
	"ror	%A0		\n\t"				\
	:  "+d" (in32), "=r" (tmp32), "=r" (mod8) : : "r0"	\
)
#endif // USE_HACKER_DELIGHT_OPTIMIZATION

#ifdef USE_STIMMER_OPTIMIZATION
// http://forum.arduino.cc/index.php?topic=167414.msg1293679#msg1293679
#define divmod10_asm32(in32, mod8, tmp8)		\
asm volatile (					\
      " ldi %2,51     \n\t"			\
      " mul %A0,%2    \n\t"			\
      " clr %A0       \n\t"			\
      " add r0,%2     \n\t"			\
      " adc %A0,r1    \n\t"			\
      " mov %1,r0     \n\t"			\
      " mul %B0,%2    \n\t"			\
      " clr %B0       \n\t"			\
      " add %A0,r0    \n\t"			\
      " adc %B0,r1    \n\t"			\
      " mul %C0,%2    \n\t"			\
      " clr %C0       \n\t"			\
      " add %B0,r0    \n\t"			\
      " adc %C0,r1    \n\t"			\
      " mul %D0,%2    \n\t"			\
      " clr %D0       \n\t"			\
      " add %C0,r0    \n\t"			\
      " adc %D0,r1    \n\t"			\
      " clr r1        \n\t"  			\
      " add %1,%A0    \n\t"			\
      " adc %A0,%B0   \n\t"			\
      " adc %B0,%C0   \n\t"			\
      " adc %C0,%D0   \n\t"			\
      " adc %D0,r1    \n\t"			\
      " add %1,%B0    \n\t"			\
      " adc %A0,%C0   \n\t"			\
      " adc %B0,%D0   \n\t"			\
      " adc %C0,r1    \n\t"			\
      " adc %D0,r1    \n\t"			\
      " add %1,%D0    \n\t"			\
      " adc %A0,r1    \n\t"			\
      " adc %B0,r1    \n\t"			\
      " adc %C0,r1    \n\t"			\
      " adc %D0,r1    \n\t"			\
      " lsr %D0       \n\t"			\
      " ror %C0       \n\t"			\
      " ror %B0       \n\t"			\
      " ror %A0       \n\t"			\
      " ror %1        \n\t"   			\
      " ldi %2,10     \n\t"			\
      " mul %1,%2     \n\t"			\
      " mov %1,r1     \n\t"			\
      " clr r1        \n\t"			\
      :"+r"(in32),"=d"(mod8),"=d"(tmp8) : : "r0")

#define divmod10_asm24(in32, mod8, tmp8)		\
asm volatile (					\
      " ldi %2,51     \n\t"			\
      " mul %A0,%2    \n\t"			\
      " clr %A0       \n\t"			\
      " add r0,%2     \n\t"			\
      " adc %A0,r1    \n\t"			\
      " mov %1,r0     \n\t"			\
      " mul %B0,%2    \n\t"			\
      " clr %B0       \n\t"			\
      " add %A0,r0    \n\t"			\
      " adc %B0,r1    \n\t"			\
      " mul %C0,%2    \n\t"			\
      " clr %C0       \n\t"			\
      " add %B0,r0    \n\t"			\
      " adc %C0,r1    \n\t"			\
      " clr r1        \n\t"  			\
      " add %1,%A0    \n\t"			\
      " adc %A0,%B0   \n\t"			\
      " adc %B0,%C0   \n\t"			\
      " adc %C0,r1    \n\t"			\
      " add %1,%B0    \n\t"			\
      " adc %A0,%C0   \n\t"			\
      " adc %B0,r1    \n\t"			\
      " adc %C0,r1    \n\t"			\
      " lsr %C0       \n\t"			\
      " ror %B0       \n\t"			\
      " ror %A0       \n\t"			\
      " ror %1        \n\t"   			\
      " ldi %2,10     \n\t"			\
      " mul %1,%2     \n\t"			\
      " mov %1,r1     \n\t"			\
      " clr r1        \n\t"			\
      :"+r"(in32),"=d"(mod8),"=d"(tmp8) : : "r0")
      
#define divmod10_asm16(in32, mod8, tmp8)		\
asm volatile (					\
      " ldi %2,51     \n\t"			\
      " mul %A0,%2    \n\t"			\
      " clr %A0       \n\t"			\
      " add r0,%2     \n\t"			\
      " adc %A0,r1    \n\t"			\
      " mov %1,r0     \n\t"			\
      " mul %B0,%2    \n\t"			\
      " clr %B0       \n\t"			\
      " add %A0,r0    \n\t"			\
      " adc %B0,r1    \n\t"			\
      " clr r1        \n\t"  			\
      " add %1,%A0    \n\t"			\
      " adc %A0,%B0   \n\t"			\
      " adc %B0,r1   \n\t"			\
      " add %1,%B0    \n\t"			\
      " adc %A0,r1   \n\t"			\
      " adc %B0,r1    \n\t"			\
      " lsr %B0       \n\t"			\
      " ror %A0       \n\t"			\
      " ror %1        \n\t"   			\
      " ldi %2,10     \n\t"			\
      " mul %1,%2     \n\t"			\
      " mov %1,r1     \n\t"			\
      " clr r1        \n\t"			\
      :"+r"(in32),"=d"(mod8),"=d"(tmp8) : : "r0")
 
#define divmod10_asm8(in32, mod8, tmp8)		\
asm volatile (					\
      " ldi %2,51     \n\t"			\
      " mul %A0,%2    \n\t"			\
      " clr %A0       \n\t"			\
      " add r0,%2     \n\t"			\
      " adc %A0,r1    \n\t"			\
      " mov %1,r0     \n\t"			\
      " clr r1        \n\t"  			\
      " add %1,%A0    \n\t"			\
      " adc %A0,r1    \n\t"			\
      " lsr %A0       \n\t"			\
      " ror %1        \n\t"   			\
      " ldi %2,10     \n\t"			\
      " mul %1,%2     \n\t"			\
      " mov %1,r1     \n\t"			\
      " clr r1        \n\t"			\
      :"+r"(in32),"=d"(mod8),"=d"(tmp8) : : "r0")
#endif // USE_STIMMER_OPTIMIZATION



#ifdef USE_BENCHMARK_CODE
uint32_t usec_print = 0;
#endif


#if ARDUINO >= 100
size_t Print::printNumberDec(unsigned long n, uint8_t sign)
#else
void Print::printNumberDec(unsigned long n, uint8_t sign)
#endif
{
	uint8_t digit, buf[11], *p;
	uint32_t tmp32;
	uint8_t tmp8;

#ifdef USE_BENCHMARK_CODE
	uint32_t usec = micros();
#endif
	p = buf + (sizeof(buf)-1);
	
	#if defined(USE_STIMMER_OPTIMIZATION)
	
	  while(n & 0xff000000){divmod10_asm32(n, digit, tmp8);*--p = digit + '0';}
	  while(n & 0xff0000){divmod10_asm24(n, digit, tmp8);*--p = digit + '0';}
	  while(n & 0xff00){divmod10_asm16(n, digit, tmp8);*--p = digit + '0';}
	  while((n & 0xff)>9){divmod10_asm8(n, digit, tmp8);*--p = digit + '0';}
	  *--p = n + '0';
	
	#else
	do {
		#if defined(USE_HACKER_DELIGHT_OPTIMIZATION)
		divmod10_asm(n, tmp32, digit);
		#else
		tmp32 = n;
		n = n / 10;
		digit = tmp32 - n * 10;
		#endif
		*--p = digit + '0';
	} while (n);
        #endif
	if (sign) *--p = '-';
#ifdef USE_BENCHMARK_CODE
	usec_print += micros() - usec;
#endif
#if ARDUINO >= 100
	return write(p, sizeof(buf)-1 - (p - buf));
#else
	write(p, sizeof(buf)-1 - (p - buf));
#endif
}

#if ARDUINO >= 100
size_t Print::printNumberHex(unsigned long n)
#else
void Print::printNumberHex(unsigned long n)
#endif
{
	uint8_t digit, buf[8], *p;

	p = buf + (sizeof(buf)-1);
	do {
		digit = n & 15;
		*--p = (digit < 10) ? '0' + digit : 'A' + digit - 10;
		n >>= 4;
	} while (n);
#if ARDUINO >= 100
	return write(p, sizeof(buf)-1 - (p - buf));
#else
	write(p, sizeof(buf)-1 - (p - buf));
#endif
}

#if ARDUINO >= 100
size_t Print::printNumberBin(unsigned long n)
#else
void Print::printNumberBin(unsigned long n)
#endif
{
	uint8_t buf[32], *p;

	p = buf + (sizeof(buf)-1);
	do {
		*--p = '0' + ((uint8_t)n & 1);
		n >>= 1;
	} while (n);
#if ARDUINO >= 100
	return write(p, sizeof(buf)-1 - (p - buf));
#else
	write(p, sizeof(buf)-1 - (p - buf));
#endif
}

#if ARDUINO >= 100
size_t Print::printNumberAny(unsigned long n, uint8_t base)
#else
void Print::printNumberAny(unsigned long n, uint8_t base)
#endif
{
	uint8_t digit, buf[21], *p;
	uint32_t tmp;
	//uint32_t usec;

	//usec = micros();
	p = buf + (sizeof(buf)-1);
	do {
		tmp = n;
		n = n / base;
		digit = tmp - n * base;
		*--p = (digit < 10) ? '0' + digit : 'A' + digit - 10;
	} while (n);
	//usec_print += micros() - usec;
#if ARDUINO >= 100
	return write(p, sizeof(buf)-1 - (p - buf));
#else
	write(p, sizeof(buf)-1 - (p - buf));
#endif
}




#if ARDUINO >= 100
size_t Print::printFloat(double number, uint8_t digits) 
#else
void Print::printFloat(double number, uint8_t digits) 
#endif
{
	uint8_t sign=0;
#if ARDUINO >= 100
	size_t count=0;
#endif

	// Handle negative numbers
	if (number < 0.0) {
		sign = 1;
		number = -number;
	}

	// Round correctly so that print(1.999, 2) prints as "2.00"
	double rounding = 0.5;
	for (uint8_t i=0; i<digits; ++i) {
		rounding *= 0.1;
	}
	number += rounding;

	// Extract the integer part of the number and print it
	unsigned long int_part = (unsigned long)number;
	double remainder = number - (double)int_part;
#if ARDUINO >= 100
	count += printNumber(int_part, sign, 10);
#else
	printNumber(int_part, sign, 10);
#endif

	// Print the decimal point, but only if there are digits beyond
	if (digits > 0) {
		uint8_t n, buf[8], count=1;
		buf[0] = '.';

		// Extract digits from the remainder one at a time
		if (digits > sizeof(buf) - 1) digits = sizeof(buf) - 1;

		while (digits-- > 0) {
			remainder *= 10.0;
			n = (uint8_t)(remainder);
			buf[count++] = '0' + n;
			remainder -= n; 
		}
#if ARDUINO >= 100
		count += write(buf, count);
#else
		write(buf, count);
#endif
	}
#if ARDUINO >= 100
	return count;
#endif
}


