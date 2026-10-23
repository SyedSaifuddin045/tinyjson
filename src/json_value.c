#include "tinyjson.h"
#include "internal.h"

#include <stdlib.h>
#include <string.h>

char *json_strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);

    if (copy == NULL)
    {
        return NULL;
    }

    memcpy(copy, s, len);
    return copy;
}

void json_value_null(JSONValue *out)
{
    if (out == NULL)
    {
        return;
    }

    memset(out, 0, sizeof(*out));
    out->type = JSON_NULL;
}

void json_value_bool(JSONValue *out, bool value)
{
    if (out == NULL)
    {
        return;
    }

    memset(out, 0, sizeof(*out));
    out->type = JSON_BOOL;
    out->boolValue = value;
}

void json_value_int(JSONValue *out, int64_t value)
{
    if (out == NULL)
    {
        return;
    }

    memset(out, 0, sizeof(*out));
    out->type = JSON_INT;
    out->intValue = value;
}

void json_value_number(JSONValue *out, double value)
{
    if (out == NULL)
    {
        return;
    }

    memset(out, 0, sizeof(*out));
    out->type = JSON_NUMBER;
    out->numberValue = value;
}

bool json_value_string(JSONValue *out, const char *value)
{
    if (out == NULL || value == NULL)
    {
        return false;
    }

    char *copy = json_strdup(value);

    if (copy == NULL)
    {
        return false;
    }

    memset(out, 0, sizeof(*out));
    out->type = JSON_STRING;
    out->stringValue = copy;
    return true;
}

bool json_value_copy(JSONValue *dst, const JSONValue *src)
{
    if (dst == NULL || src == NULL)
    {
        return false;
    }

    memset(dst, 0, sizeof(*dst));
    dst->type = JSON_NULL;

    switch (src->type)
    {
    case JSON_NULL:
        return true;

    case JSON_BOOL:
        dst->type = JSON_BOOL;
        dst->boolValue = src->boolValue;
        return true;

    case JSON_INT:
        dst->type = JSON_INT;
        dst->intValue = src->intValue;
        return true;

    case JSON_NUMBER:
        dst->type = JSON_NUMBER;
        dst->numberValue = src->numberValue;
        return true;

    case JSON_STRING:
    {
        if (src->stringValue == NULL)
        {
            return true; /* inconsistent source: copy as JSON_NULL */
        }

        char *copy = json_strdup(src->stringValue);
        if (copy == NULL)
        {
            return false;
        }

        dst->type = JSON_STRING;
        dst->stringValue = copy;
        return true;
    }

    case JSON_ARRAY:
    {
        if (src->arrayValue == NULL)
        {
            return true;
        }

        JSONArray *array = json_array_init();
        if (array == NULL)
        {
            return false;
        }

        for (size_t i = 0; i < src->arrayValue->size; i++)
        {
            JSONValue item;

            if (!json_value_copy(&item, &src->arrayValue->values[i]) ||
                !json_array_push(array, item))
            {
                json_value_free(&item);
                json_array_free(array);
                return false;
            }
        }

        dst->type = JSON_ARRAY;
        dst->arrayValue = array;
        return true;
    }

    case JSON_OBJECT:
    {
        if (src->objectValue == NULL)
        {
            return true;
        }

        JSONObject *object = json_object_init();
        if (object == NULL)
        {
            return false;
        }

        for (size_t i = 0; i < src->objectValue->size; i++)
        {
            const JSONKeyValuePair *pair = &src->objectValue->pairs[i];
            JSONValue item;

            if (!json_value_copy(&item, &pair->value) ||
                !json_object_set(object, pair->key, item))
            {
                json_value_free(&item);
                json_object_free(object);
                return false;
            }
        }

        dst->type = JSON_OBJECT;
        dst->objectValue = object;
        return true;
    }
    }

    return false;
}

void json_value_free(JSONValue *value)
{
    if (value == NULL)
    {
        return;
    }

    switch (value->type)
    {
    case JSON_NULL:
    case JSON_BOOL:
    case JSON_INT:
    case JSON_NUMBER:
        /* These types own no separately allocated payload. */
        break;

    case JSON_STRING:
        /* TinyJSON owns dynamically allocated string storage. */
        if (value->stringValue != NULL)
        {
            free(value->stringValue);
            value->stringValue = NULL;
        }
        break;

    case JSON_ARRAY:
        if (value->arrayValue != NULL)
        {
            /* Recursively frees elements, buffer and the struct itself. */
            json_array_free(value->arrayValue);
            value->arrayValue = NULL;
        }
        break;

    case JSON_OBJECT:
        if (value->objectValue != NULL)
        {
            /* Recursively frees keys, values, pair buffer and the struct. */
            json_object_free(value->objectValue);
            value->objectValue = NULL;
        }
        break;
    }

    /* Leave the value in a predictable state. */
    value->type = JSON_NULL;
}
