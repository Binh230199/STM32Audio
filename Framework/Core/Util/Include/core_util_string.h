/*
 * core_util_string.h
 *
 *  Created on: Aug 26, 2025
 *      Author: binhhv.23.1.99@gmail.com
 */

#ifndef CORE_UTIL_INCLUDE_CORE_UTIL_STRING_H_
#define CORE_UTIL_INCLUDE_CORE_UTIL_STRING_H_

#include <string.h>
#include <stdio.h>


#/**
 * @brief Check if a string is empty or NULL.
 * @param s String to check.
 * @return true if string is NULL or empty, false if it has data.
 *
 * Example:
 *   char *str1 = NULL;
 *   char *str2 = "";
 *   char *str3 = "abc";
 *   if (C_STR_IS_EMPTY(str1)) {...} // true
 *   if (C_STR_IS_EMPTY(str2)) {...} // true
 *   if (C_STR_IS_EMPTY(str3)) {...} // false
 */
#define C_STR_IS_EMPTY(s)            (((s) == NULL) || ((s)[0] == '\0'))


/**
 * @brief Safely copy a string, always null-terminated.
 * @param d Destination buffer.
 * @param s Source string.
 * @param n Size of destination buffer.
 *
 * Example:
 *   char dest[10];
 *   C_STRNCPY(dest, "hello", sizeof(dest));
 *   // dest = "hello"
 */
#define C_STRNCPY(d, s, n)              \
do {                                  \
  if ((d) != NULL && (s) != NULL) {   \
    (void)strncpy((d), (s), (n) - 1); \
    (d)[(n) - 1] = '\0';              \
  }                                   \
} while (0)


/**
 * @brief Safely format a string into buffer, always null-terminated.
 * @param d Destination buffer.
 * @param n Size of destination buffer.
 * @param fmt printf format string.
 * @param ... Format arguments.
 *
 * Example:
 *   char buf[32];
 *   int val = 123;
 *   C_STRFMT(buf, sizeof(buf), "Value: %d", val);
 *   // buf = "Value: 123"
 */
#define C_STRFMT(d, n, fmt, ...)                           \
do {                                                       \
  if ((d) != NULL && (n) > 0) {                            \
    int _len = snprintf((d), (n), (fmt), ##__VA_ARGS__);   \
    if (_len < 0 || _len >= (int)(n)) {                    \
      (d)[(n) - 1] = '\0'; /* Always null-terminate */    \
    }                                                      \
  }                                                        \
} while (0)


#endif /* CORE_UTIL_INCLUDE_CORE_UTIL_STRING_H_ */
