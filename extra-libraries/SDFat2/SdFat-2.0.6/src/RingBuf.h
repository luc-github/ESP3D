/**
 * Copyright (c) 2011-2020 Bill Greiman
 * This file is part of the SdFat library for SD memory cards.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef RingBuf_h
#define RingBuf_h
/**
 * \file
 * \brief Ring buffer for data loggers.
 */
#include "Arduino.h"
#include "common/FmtNumber.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS
//  Teensy 3.5/3.6 has hard fault at 0x20000000 for unaligned memcpy.
#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
inline bool is_aligned(const void* ptr, uintptr_t alignment) {
    auto iptr = reinterpret_cast<uintptr_t>(ptr);
    return !(iptr % alignment);
}
inline void memcpyBuf(void* dst, const void* src, size_t len) {
  const uint8_t* b = reinterpret_cast<const uint8_t*>(0X20000000UL);
  uint8_t* d = reinterpret_cast<uint8_t*>(dst);
  const uint8_t *s = reinterpret_cast<const uint8_t*>(src);
  if ((is_aligned(d, 4) && is_aligned(s, 4) && (len & 3) == 0) ||
    !((d < b && b <= (d + len)) || (s < b && b <= (s + len)))) {
    memcpy(dst, src, len);
  } else {
    while (len--) {
      *d++ = *s++;
    }
  }
}
#else  // defined(__MK64FX512__) || defined(__MK66FX1M0__)
inline void memcpyBuf(void* dst, const void* src, size_t len) {
  memcpy(dst, src, len);
}
#endif  // defined(__MK64FX512__) || defined(__MK66FX1M0__)
#endif  // DOXYGEN_SHOULD_SKIP_THIS
/**
 * \class RingBuf
 * \brief Ring buffer for data loggers.
 *
 * This ring buffer may be used in ISRs.  bytesFreeIsr(), bytesUsedIsr(),
 * memcopyIn(), and memcopyOut() are ISR callable.  For ISR use call
 * memcopyIn() in the ISR and use writeOut() in non-interrupt code
 * to write data to a file. readIn() and memcopyOut can be use in a
 * similar way to provide file data to an ISR.
 *
 * Print into a RingBuf in an ISR should also work but has not been verified.
 */
template<class F, size_t Size>
class RingBuf : public Print {
 public:
  /**
   * RingBuf Constructor.
   */
  RingBuf() {}
  /**
   * Initialize RingBuf.
   * \param[in] file Underlying file.
   */
  void begin(F* file) {
    m_file = file;
    m_count = 0;
    m_head = 0;
    m_tail = 0;
    clearWriteError();
  }
  /**
   *
   * \return the RingBuf free space in bytes. Not ISR callable.
   */
  size_t bytesFree() const {
    size_t count;
    noInterrupts();
    count = m_count;
    interrupts();
    return Size - count;
  }
  /**
   * \return the RingBuf free space in bytes. ISR callable.
   */
  size_t bytesFreeIsr() const {
    return Size - m_count;
  }
  /**
   * \return the RingBuf used space in bytes. Not ISR callable.
   */
  size_t bytesUsed() const {
    size_t count;
    noInterrupts();
    count = m_count;
    interrupts();
    return count;
  }
  /**
   * \return the RingBuf used space in bytes.  ISR callable.
   */
  size_t bytesUsedIsr() const {
    return m_count;
  }
  /**
   * Copy data to the RingBuf from buf.
   * The number of bytes copied may be less than count if
   * count is greater than bytesFree.
   *
   * This function may be used in an ISR with writeOut()
   * in non-interrupt code.
   *
   * \param[in] buf Location of data to be copied.
   * \param[in] count number of bytes to be copied.
   * \return Number of bytes actually copied.
   */
  size_t memcpyIn(const void* buf, size_t count) {
    const uint8_t* src = (const uint8_t*)buf;
    size_t n = Size - m_count;
    if (count > n) {
      count = n;
    }
    size_t nread = 0;
    while (nread != count) {
        n = minSize(Size - m_head, count - nread);
        memcpyBuf(m_buf + m_head, src + nread, n);
        m_head = advance(m_head, n);
        nread += n;
    }
    m_count += nread;
    return nread;
  }
  /**
   * Copy date from the RingBuf to buf.
   * The number of bytes copied may be less than count if
   * bytesUsed is less than count.
   *
   * This function may be used in an ISR with readIn() in
   * non-interrupt code.
   *
   * \param[out] buf Location to receive the data.
   * \param[in] count number of bytes to be copied.
   * \return Number of bytes actually copied.
   */
  size_t memcpyOut(void* buf, size_t count) {
    uint8_t* dst = reinterpret_cast<uint8_t*>(buf);
    size_t nwrite = 0;
    size_t n = m_count;
    if (count > n) {
      count = n;
    }
    while (nwrite != count) {
      n = minSize(Size - m_tail, count - nwrite);
      memcpyBuf(dst + nwrite, m_buf + m_tail, n);
      m_tail = advance(m_tail, n);
      nwrite += n;
    }
    m_count -= nwrite;
    return nwrite;
  }
  /** Print a number followed by a field terminator.
   * \param[in] value The number to be printed.
   * \param[in] term The field terminator.  Use '\\n' for CR LF.
   * \param[in] prec Number of digits after decimal point.
   * \return The number of bytes written.
   */
  size_t printField(double value, char term, uint8_t prec = 2) {
    char buf[24];
    char* str = buf + sizeof(buf);
    if (term) {
      *--str = term;
      if (term == '\n') {
        *--str = '\r';
      }
    }
    str = fmtDouble(str, value, prec, false);
    return write(str, buf + sizeof(buf) - str);
  }
  /** Print a number followed by a field terminator.
   * \param[in] value The number to be printed.
   * \param[in] term The field terminator.  Use '\\n' for CR LF.
   * \param[in] prec Number of digits after decimal point.
   * \return The number of bytes written or -1 if an error occurs.
   */
  size_t printField(float value, char term, uint8_t prec = 2) {
    return printField(static_cast<double>(value), term, prec);
  }
  /** Print a number followed by a field terminator.
   * \param[in] value The number to be printed.
   * \param[in] term The field terminator.  Use '\\n' for CR LF.
   * \return The number of bytes written or -1 if an error occurs.
   */
  template <typename Type>
  size_t printField(Type value, char term) {
    char sign = 0;
    char buf[3*sizeof(Type) + 3];
    char* str = buf + sizeof(buf);

    if (term) {
      *--str = term;
      if (term == '\n') {
        *--str = '\r';
      }
    }
    if (value < 0) {
      value = -value;
      sign = '-';
    }
    if (sizeof(Type) < 4) {
      str = fmtBase10(str, (uint16_t)value);
    } else {
      str = fmtBase10(str, (uint32_t)value);
    }
    if (sign) {
      *--str = sign;
    }
    return write((const uint8_t*)str, &buf[sizeof(buf)] - str);
  }
  /**
   * Read data into the RingBuf from the underlying file.
   * the number of bytes read may be less than count if
   * bytesFree is less than count.
   *
   * This function may be used in non-interrupt code with
   * memcopyOut() in an ISR.
   *
   * \param[in] count number of bytes to be read.
   * \return Number of bytes actually read.
   */
  size_t readIn(size_t count) {
    size_t nread = 0;
    size_t n = bytesFree();  // Protected from interrupts.
    if (count > n) {
      count = n;
    }
    while (nread != count) {
        n = minSize(Size - m_head, count - nread);
        if ((size_t)m_file->read(m_buf + m_head, n) != n) {
          return nread;
        }
        m_head = advance(m_head, n);
        nread += n;
    }
    noInterrupts();
    m_count += nread;
    interrupts();
    return nread;
  }
  /**
   * Write all data in the RingBuf to the underlying file.
   * \param[in] data Byte to be written.
   * \return Number of bytes actually written.
   */
  bool sync() {
    size_t n = bytesUsed();
    return writeOut(n) == n;
  }
  /**
   * Copy data to the RingBuf from buf.
   *
   * The number of bytes copied may be less than count if
   * count is greater than bytesFree.
   * Use getWriteError() to check for print errors and
   * clearWriteError() to clear error.
   *
   * \param[in] buf Location of data to be written.
   * \param[in] count number of bytes to be written.
   * \return Number of bytes actually written.
   */
  size_t write(const void* buf, size_t count) {
    if (count > bytesFree()) {
      setWriteError();
    }
    return memcpyIn(buf, count);
  }
  /**
   * Override virtual function in Print for efficiency.
   *
   * \param[in] buf Location of data to be written.
   * \param[in] count number of bytes to be written.
   * \return Number of bytes actually written.
   */
  size_t write(const uint8_t* buf, size_t count) override {
    return write((const void*)buf, count);
  }
  /**
   * Required function for Print.
   * \param[in] data Byte to be written.
   * \return Number of bytes actually written.
   */
  size_t write(uint8_t data) override {
    return write(&data, 1);
  }
  /**
   * Write data to file from RingBuf buffer.
   * \param[in] count number of bytes to be written.
   *
   * The number of bytes written may be less than count if
   * bytesUsed is less than count or if an error occurs.
   *
   * This function may be used in non-interrupt code with
   * memcopyIn() in an ISR.
   *
   * \return Number of bytes actually written.
   */
  size_t writeOut(size_t count) {
    size_t n = bytesUsed();  // Protected from interrupts;
     if (count > n) {
      count = n;
    }
    size_t nwrite = 0;
    while (nwrite != count) {
      n = minSize(Size - m_tail, count - nwrite);
      if (m_file->write(m_buf + m_tail, n) != n) {
        break;
      }
      m_tail = advance(m_tail, n);
      nwrite += n;
    }
    noInterrupts();
    m_count -= nwrite;
    interrupts();
    return nwrite;
  }

 private:
  uint8_t __attribute__((aligned(4))) m_buf[Size];
  F* m_file = nullptr;
  volatile size_t m_count;
  size_t m_head;
  size_t m_tail;

  size_t advance(size_t index, size_t n) {
    index += n;
    return index < Size ? index : index - Size;
  }
  // avoid macro MIN
  size_t minSize(size_t a, size_t b) {return a < b ? a : b;}
};
#endif  // RingBuf_h
