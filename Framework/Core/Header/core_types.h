/*
 * core_types.h
 *
 *  Created on: Jul 12, 2024
 *      Author: binhhv.23.1.99@gmail.com
 */

#ifndef CORE_HEADER_CORE_TYPES_H_
#define CORE_HEADER_CORE_TYPES_H_

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Define boolean type
 */
typedef enum
{
  false = 0,
  true = 1
} bool_t;

// Typedef for data types
typedef char char_t;
typedef float float32_t;
typedef double float64_t;

#endif /* CORE_HEADER_CORE_TYPES_H_ */
