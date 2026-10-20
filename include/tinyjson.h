
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

void json_value_free(JSONValue *value);

/* JSON Array */
bool json_array_init(JSONArray *array);
bool json_array_push(JSONArray *array, JSONValue value);
void json_array_free(JSONArray *array);

#ifdef __cplusplus
}
#endif

#endif /* TINYJSON_H */
