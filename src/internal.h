/**
 * @file internal.h
 * @brief Private helpers shared between TinyJSON translation units.
 *
 * Not installed with the public headers; include only from the library's own
 * translation units.
 */

#ifndef TINYJSON_INTERNAL_H
#define TINYJSON_INTERNAL_H

#include <stddef.h>

/**
 * @brief Portable replacement for POSIX strdup().
 *
 * Named json_strdup() rather than strdup() because "strdup" is reserved by
 * POSIX and may already be declared by <string.h>.
 *
 * @param s NUL-terminated string to copy. Must not be NULL.
 * @return Heap-allocated copy owned by the caller, or NULL on allocation
 *         failure. Free with free().
 */
char *json_strdup(const char *s);

#endif /* TINYJSON_INTERNAL_H */
