
#ifndef TINYJSON_H
#define TINYJSON_H

#include <stdbool.h>
#include <stddef.h>

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
        const char *stringValue;
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
#endif