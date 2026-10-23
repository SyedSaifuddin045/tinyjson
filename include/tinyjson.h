
#ifndef TINYJSON_H
#define TINYJSON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum JSONType
{
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JSONType;

typedef struct JSONValue JSONValue;
typedef struct JSONArray JSONArray;
typedef struct JSONObject JSONObject;
typedef struct JSONKeyValuePair JSONKeyValuePair;

struct JSONValue
{
    JSONType type;
    union
    {
        bool boolValue;
        double numberValue;
        char *stringValue;
        JSONArray *arrayValue;
        JSONObject *objectValue;
    };
};

struct JSONArray
{
    size_t size;
    size_t capacity;
    JSONValue *values;
};

struct JSONKeyValuePair
{
    char *key;
    JSONValue value;
};

struct JSONObject
{
    size_t size;
    size_t capacity;
    JSONKeyValuePair *pairs;
};

const char *tinyjson_version(void);

/* Releases the payload owned by `value`.
 *
 * `value` itself is inline storage owned by the caller (it may live on the
 * stack or inside a pair/array slot) and is NOT freed. After the call the
 * value is reset to JSON_NULL, so calling this twice is safe.
 */
void json_value_free(JSONValue *value);

/* JSON Array
 *
 * Ownership: json_array_push takes ownership of any dynamically allocated
 * payload referenced by `value` (e.g. value.stringValue). The caller must not
 * free that payload afterwards.
 *
 * json_array_free releases every element, the element buffer and the
 * JSONArray structure itself. `array` must not be used after the call.
 */
JSONArray *json_array_init(void);
bool json_array_push(JSONArray *array, JSONValue value);
void json_array_free(JSONArray *array);

/* JSON Object
 *
 * Ownership: json_object_set takes ownership of any dynamically allocated
 * payload referenced by `value`. The caller must not free it afterwards. The
 * key is copied; the caller keeps ownership of their own key buffer.
 *
 * Lifetime: pointers returned by json_object_get point into the object's
 * internal storage and are invalidated by the next set or free call.
 *
 * json_object_free releases every key, every value payload, the pair buffer
 * and the JSONObject structure itself. `obj` must not be used after the call.
 */
JSONObject *json_object_init(void);
bool json_object_find(const JSONObject *obj, const char *key, size_t *out_index);
bool json_object_set(JSONObject *obj, const char *key, JSONValue value);
const JSONValue *json_object_get(const JSONObject *obj, const char *key);
void json_object_free(JSONObject *obj);

#ifdef __cplusplus
}
#endif

#endif /* TINYJSON_H */
