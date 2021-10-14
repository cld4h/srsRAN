/**
 * Copyright 2013-2021 Software Radio Systems Limited
 *
 * This file is part of srsLTE.
 *
 * srsLTE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsLTE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#ifndef SRSLTE_ASSERT_H
#define SRSLTE_ASSERT_H

#include "srslte/srslog/srslog.h"
#include <cstdio>

#define srslte_unlikely(expr) __builtin_expect(!!(expr), 0)

#define srslte_terminate(fmt, ...)                                                                                     \
  std::fprintf(stderr, "%s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);                                         \
  srslog::flush();                                                                                                     \
  std::abort()

#ifdef ASSERTS_ENABLED

/**
 * Macro that asserts condition is true. If false, it logs the remaining parameters, prints the backtrace and closes
 * the application
 */
#define srslte_assert(condition, fmt, ...)                                                                             \
  do {                                                                                                                 \
    if (srslte_unlikely(not(condition))) {                                                                             \
      srslte_terminate(fmt, ##__VA_ARGS__);                                                                            \
    }                                                                                                                  \
  } while (0)

#ifdef STOP_ON_WARNING

/**
 * Macro that verifies if condition is true. If false, and STOP_ON_WARNING is true, it behaves like srslte_assert.
 * If STOP_ON_WARNING is false, it logs a warning.
 */
#define srslte_expect(condition, fmt, ...) srslte_assert(condition, fmt, ##__VA_ARGS__)

#else // STOP_ON_WARNING

#define srslte_expect(condition, fmt, ...)                                                                             \
  do {                                                                                                                 \
    if (srslte_unlikely(not(condition))) {                                                                             \
      srslog::fetch_basic_logger("ALL").warning("%s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__);                     \
    }                                                                                                                  \
  } while (0)

#endif // STOP_ON_WARNING

#else // ASSERTS_ENABLED

#define srslte_assert(condition, fmt, ...)                                                                             \
  do {                                                                                                                 \
  } while (0)

#define srslte_expect(condition, fmt, ...) srslte_assert(condition, fmt, ##__VA_ARGS__)

#endif

#endif // SRSLTE_ASSERT_H
