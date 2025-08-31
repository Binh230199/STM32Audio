#ifndef CORE_UTIL_SHORT_CODE_H_
#define CORE_UTIL_SHORT_CODE_H_

#define C_UNUSED_VAR(x) ((void)(x))

// Early return nếu con trỏ null
#define C_RETURN_IF_NULL(ptr)     do { if ((ptr) == NULL) { return; } } while(0)

// Early return nếu điều kiện đúng
#define C_RETURN_IF(cond)         do { if (cond) { return; } } while(0)

// Early return với giá trị nếu con trỏ null
#define C_RETURN_VAL_IF_NULL(ptr, val)   do { if ((ptr) == NULL) { return (val); } } while(0)

// Early return với giá trị nếu điều kiện đúng
#define C_RETURN_VAL_IF(cond, val)       do { if (cond) { return (val); } } while(0)

#endif /* CORE_UTIL_SHORT_CODE_H_ */
