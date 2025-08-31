/*
 * core_util_ptr.h
 *
 *  Created on: Aug 28, 2025
 *      Author: binhhv.23.1.99@gmail.com
 */

#ifndef CORE_UTIL_INCLUDE_CORE_UTIL_PTR_H_
#define CORE_UTIL_INCLUDE_CORE_UTIL_PTR_H_

#include <stddef.h> // Định nghĩa NULL cho các macro/hàm con trỏ

// Kiểm tra con trỏ null
#define C_PTR_IS_NULL(ptr)        ((ptr) == NULL)
#define C_PTR_IS_NOT_NULL(ptr)    ((ptr) != NULL)

// Kiểm tra con trỏ hợp lệ (có thể mở rộng thêm logic nếu cần)
#define C_PTR_IS_VALID(ptr)       ((ptr) != NULL)

// Gán giá trị cho con trỏ nếu chưa null
#define C_PTR_SET_IF_NULL(ptr, val) do { if ((ptr) == NULL) { (ptr) = (val); } } while(0)

// Giải phóng con trỏ và set về NULL (dùng cho malloc/free)
#define C_PTR_SAFE_FREE(ptr)      do { if ((ptr) != NULL) { free(ptr); (ptr) = NULL; } } while(0)

// Hàm inline kiểm tra và gọi callback nếu con trỏ hợp lệ
static inline void core_ptr_call_if_valid(void (*callback)(void*), void* arg) {
  if (callback != NULL) {
    callback(arg);
  }
}

#endif /* CORE_UTIL_INCLUDE_CORE_UTIL_PTR_H_ */
