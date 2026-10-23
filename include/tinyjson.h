/**
 * @file tinyjson.h
 * @brief Minimal JSON value model for C.
 *
 * Memory model
 * ------------
 * Scalars (null, bool, number) are stored inline inside a #JSONValue.
 * Strings, arrays and objects own heap-allocated payloads that are referenced
 * from the #JSONValue by pointer.
 *
 * Ownership is transferred on insertion: after json_object_set() or
 * json_array_push(), the container owns any dynamically allocated payload of
 * the value and will free it. The caller must not free that payload and must
 * not insert the same value into two containers.
 */

#ifndef TINYJSON_H
#define TINYJSON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Discriminator for the active member of #JSONValue's union.
 */
typedef enum JSONType
{
    JSON_NULL,   /**< No value; no payload owned.              */
    JSON_BOOL,   /**< Member: value.boolValue.                */
    JSON_INT,    /**< Member: value.intValue.                 */
    JSON_NUMBER, /**< Member: value.numberValue.              */
    JSON_STRING, /**< Member: value.stringValue (owned).      */
    JSON_ARRAY,  /**< Member: value.arrayValue (owned).       */
    JSON_OBJECT  /**< Member: value.objectValue (owned).      */
} JSONType;

typedef struct JSONValue JSONValue;
typedef struct JSONArray JSONArray;
typedef struct JSONObject JSONObject;
typedef struct JSONKeyValuePair JSONKeyValuePair;

/**
 * @brief A single JSON value.
 *
 * The active union member is selected by #type. Only one member is meaningful
 * at a time; reading any other member is undefined behaviour.
 */
struct JSONValue
{
    JSONType type; /**< Which member of the union is active. */

    union
    {
        bool boolValue;          /**< Valid when type == JSON_BOOL.   */
        int64_t intValue;        /**< Valid when type == JSON_INT.    */
        double numberValue;      /**< Valid when type == JSON_NUMBER. */
        char *stringValue;       /**< Valid when type == JSON_STRING. NUL-terminated, owned. */
        JSONArray *arrayValue;   /**< Valid when type == JSON_ARRAY.  Owned. */
        JSONObject *objectValue; /**< Valid when type == JSON_OBJECT. Owned. */
    };
};

/**
 * @brief Growable, contiguous list of values.
 *
 * Access elements as @c array->values[i] for @c i < @c array->size.
 */
struct JSONArray
{
    size_t size;       /**< Number of elements in use.             */
    size_t capacity;   /**< Number of slots allocated in @c values. */
    JSONValue *values; /**< Element block; NULL while capacity == 0. */
};

/**
 * @brief One key/value entry of a #JSONObject.
 */
struct JSONKeyValuePair
{
    char *key;        /**< NUL-terminated key, owned by the object. */
    JSONValue value;  /**< Associated value, stored inline.         */
};

/**
 * @brief Growable, contiguous list of key/value pairs.
 *
 * Keys are unique. Lookup is linear, O(size); appropriate for the small
 * objects found in typical JSON documents.
 */
struct JSONObject
{
    size_t size;              /**< Number of pairs in use.                   */
    size_t capacity;          /**< Number of slots allocated in @c pairs.    */
    JSONKeyValuePair *pairs;  /**< Pair block; NULL while capacity == 0.     */
};

/**
 * @brief Return the library version string.
 *
 * @return Pointer to a statically allocated NUL-terminated string such as
 *         "0.1.0". Never NULL; must not be freed.
 */
const char *tinyjson_version(void);

/**
 * @brief Release the payload owned by a value and reset it to JSON_NULL.
 *
 * Frees @c stringValue, or recursively destroys @c arrayValue / @c objectValue
 * (including their structs). Scalars own nothing.
 *
 * @param value Value to clear. May be NULL, in which case this is a no-op.
 *              The struct itself is inline storage owned by the caller and is
 *              never freed. After the call @c value->type is JSON_NULL, so
 *              calling this function twice on the same value is safe.
 */
void json_value_free(JSONValue *value);

/**
 * @brief Construct a JSON null value.
 *
 * @param out Destination. Must not hold an owned payload (constructors do not
 *            free existing contents). May be NULL, in which case this is a
 *            no-op. On return @c out->type is JSON_NULL.
 */
void json_value_null(JSONValue *out);

/**
 * @brief Construct a JSON bool value.
 *
 * @param out   Destination. Must not hold an owned payload. May be NULL.
 * @param value Boolean payload.
 */
void json_value_bool(JSONValue *out, bool value);

/**
 * @brief Construct a JSON integer value.
 *
 * @param out   Destination. Must not hold an owned payload. May be NULL.
 * @param value Signed 64-bit payload.
 */
void json_value_int(JSONValue *out, int64_t value);

/**
 * @brief Construct a JSON number (floating point) value.
 *
 * @param out   Destination. Must not hold an owned payload. May be NULL.
 * @param value IEEE-754 double payload.
 */
void json_value_number(JSONValue *out, double value);

/**
 * @brief Construct a JSON string value by copying @p value.
 *
 * The destination owns an independent copy; the caller keeps ownership of
 * @p value. Only this constructor can fail, because it allocates.
 *
 * @param out   Destination. Must not hold an owned payload. May be NULL.
 * @param value NUL-terminated source string. Must not be NULL.
 *
 * @return true on success; false if @p out or @p value is NULL, or allocation
 *         fails. On failure @p out is left untouched.
 */
bool json_value_string(JSONValue *out, const char *value);

/**
 * @brief Deep-copy a value.
 *
 * Copies scalars by value and duplicates every owned payload: strings are
 * copied, arrays and objects are copied recursively, including their keys.
 * The source is not modified and the copy is fully independent, so freeing
 * either value never affects the other.
 *
 * @param dst Destination. Must not hold an owned payload. On success it holds
 *            a deep copy; on failure it is reset to JSON_NULL. Must not be
 *            NULL.
 * @param src Source value. Must not be NULL.
 *
 * @return true on success; false if either argument is NULL or allocation
 *         fails. A NUL-container or NULL-string source is copied as JSON_NULL.
 */
bool json_value_copy(JSONValue *dst, const JSONValue *src);

/* ------------------------------------------------------------------------- */
/* JSON Array                                                                */
/* ------------------------------------------------------------------------- */

/**
 * @brief Allocate a new, empty array.
 *
 * @return New array with @c size == 0, @c capacity == 0 and @c values == NULL,
 *         or NULL if allocation fails. Release with json_array_free().
 */
JSONArray *json_array_init(void);

/**
 * @brief Append a value to the end of an array, growing it if needed.
 *
 * Takes ownership of any dynamically allocated payload referenced by
 * @p value (see the file-level memory model). On success the caller must not
 * free that payload, and must not insert the same value into another
 * container.
 *
 * @param array Array to append to. Must not be NULL.
 * @param value Value to copy into the array. Copied by value; only its
 *              owned payload (if any) is transferred.
 *
 * @return true on success; false if @p array is NULL, the capacity would
 *         overflow, or memory allocation fails. On failure @p array is
 *         unchanged and the caller still owns @p value's payload.
 */
bool json_array_push(JSONArray *array, JSONValue value);

/**
 * @brief Read an element without copying it.
 *
 * @param array Array to read from. May be NULL.
 * @param index Zero-based element index.
 *
 * @return Pointer to the element, or NULL if @p array is NULL or
 *         @p index >= @c array->size. The pointer refers to internal storage
 *         and is invalidated by the next json_array_push() or
 *         json_array_free() on @p array. Treat the result as read-only.
 */
const JSONValue *json_array_get(const JSONArray *array, size_t index);

/**
 * @brief Destroy an array and everything it owns.
 *
 * Frees every element's owned payload, the @c values block and the #JSONArray
 * structure itself.
 *
 * @param array Array to destroy. May be NULL, in which case this is a no-op.
 *              Must not be used after the call.
 */
void json_array_free(JSONArray *array);

/* ------------------------------------------------------------------------- */
/* JSON Object                                                               */
/* ------------------------------------------------------------------------- */

/**
 * @brief Allocate a new, empty object.
 *
 * @return New object with @c size == 0, @c capacity == 0 and @c pairs == NULL,
 *         or NULL if allocation fails. Release with json_object_free().
 */
JSONObject *json_object_init(void);

/**
 * @brief Locate an existing key.
 *
 * Performs a linear scan, comparing keys with strcmp().
 *
 * @param obj  Object to search. May be NULL.
 * @param key  NUL-terminated key to look for. May be NULL.
 * @param out_index Optional output. When the return value is true it receives
 *                  the index of the matching pair. Ignored when not found.
 *                  May be NULL if the caller only needs membership.
 *
 * @return true if @p key exists in @p obj, false otherwise (including when
 *         @p obj or @p key is NULL).
 */
bool json_object_find(const JSONObject *obj, const char *key, size_t *out_index);

/**
 * @brief Insert a key/value pair, or replace the value of an existing key.
 *
 * The key is copied; the caller keeps ownership of @p key. Any dynamically
 * allocated payload referenced by @p value is transferred to @p obj, and the
 * previous payload for that key (if replacing) is freed.
 *
 * @param obj   Object to modify. Must not be NULL.
 * @param key   NUL-terminated key. Copied, so it need not outlive the call.
 *              Must not be NULL.
 * @param value Value to store. Copied by value; ownership of its dynamic
 *              payload is transferred on success.
 *
 * @return true on success; false if @p obj or @p key is NULL, the capacity
 *         would overflow, or memory allocation fails. On failure @p obj is
 *         unchanged and the caller still owns @p value's payload.
 *
 * @note Do not pass a value that is already owned by @p obj; freeing the old
 *       payload would invalidate the value being inserted.
 */
bool json_object_set(JSONObject *obj, const char *key, JSONValue value);

/**
 * @brief Look up a value by key.
 *
 * @param obj Object to search. May be NULL.
 * @param key NUL-terminated key. May be NULL.
 *
 * @return Pointer to the stored value, or NULL if @p obj / @p key is NULL or
 *         the key is absent. The pointer refers to internal storage and is
 *         invalidated by the next json_object_set() or json_object_free() on
 *         @p obj. Treat the result as read-only.
 */
const JSONValue *json_object_get(const JSONObject *obj, const char *key);

/**
 * @brief Remove a key and everything it owns.
 *
 * Frees the stored key, frees the value's owned payload, and shifts later
 * pairs down to keep the remaining insertion order.
 *
 * @param obj Object to modify. May be NULL.
 * @param key NUL-terminated key to remove. May be NULL.
 *
 * @return true if the key existed and was removed; false if @p obj or
 *         @p key is NULL, or the key is absent (object left unchanged).
 */
bool json_object_delete(JSONObject *obj, const char *key);

/**
 * @brief Destroy an object and everything it owns.
 *
 * Frees every key, every value payload, the @c pairs block and the #JSONObject
 * structure itself.
 *
 * @param obj Object to destroy. May be NULL, in which case this is a no-op.
 *            Must not be used after the call.
 */
void json_object_free(JSONObject *obj);

#ifdef __cplusplus
}
#endif

#endif /* TINYJSON_H */
