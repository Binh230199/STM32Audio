/*
 * sink_interface.c
 *
 *  Created on: Aug 23, 2025
 *    Author: binhhv.23.1.99@gmail.com
 */

#include "core_logger_sink_interface.h"
#include <string.h>

void log_sink_init(core_logger_sink_t* sink, const log_sink_vtable_t* vtable,
           const char* name, log_level_t min_level)
{
  if (sink == NULL || vtable == NULL) {
    return;
  }

  sink->vtable = vtable;
  sink->min_level = min_level;

  /* Copy name with safety check */
  if (name != NULL) {
    (void)strncpy(sink->name, name, sizeof(sink->name) - 1);
    sink->name[sizeof(sink->name) - 1] = '\0';
  } else {
    (void)strncpy(sink->name, "Unknown", sizeof(sink->name) - 1);
    sink->name[sizeof(sink->name) - 1] = '\0';
  }
}

bool_t log_sink_should_log(const core_logger_sink_t* sink, log_level_t level)
{
  if (sink == NULL) {
    return false;
  }

  /* Check if level is appropriate */
  return (level >= sink->min_level);
}


