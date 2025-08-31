#ifndef CORE_UTIL_MATH_H_
#define CORE_UTIL_MATH_H_

#include <stdint.h>
#include <float.h>

// Max, Min cho số nguyên
static inline int int_max(int a, int b) { return (a > b) ? a : b; }
static inline int int_min(int a, int b) { return (a < b) ? a : b; }
static inline int int_clamp(int x, int min, int max) { return (x < min) ? min : ((x > max) ? max : x); }
static inline int int_abs(int x) { return (x < 0) ? -x : x; }

// Max, Min cho số thực
static inline float float_max(float a, float b) { return (a > b) ? a : b; }
static inline float float_min(float a, float b) { return (a < b) ? a : b; }
static inline float float_clamp(float x, float min, float max) { return (x < min) ? min : ((x > max) ? max : x); }
static inline float float_abs(float x) { return (x < 0.0f) ? -x : x; }

// So sánh số thực với epsilon
static inline int float_equal(float a, float b, float epsilon) { return (float_abs(a - b) < epsilon); }
static inline int float_is_zero(float a, float epsilon) { return (float_abs(a) < epsilon); }

// Max, Min cho double
static inline double double_max(double a, double b) { return (a > b) ? a : b; }
static inline double double_min(double a, double b) { return (a < b) ? a : b; }
static inline double double_clamp(double x, double min, double max) { return (x < min) ? min : ((x > max) ? max : x); }
static inline double double_abs(double x) { return (x < 0.0) ? -x : x; }
static inline int double_equal(double a, double b, double epsilon) { return (double_abs(a - b) < epsilon); }
static inline int double_is_zero(double a, double epsilon) { return (double_abs(a) < epsilon); }

#endif /* CORE_UTIL_MATH_H_ */
