#include "tinyjson.h"

#include <stdlib.h>

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
    case JSON_NUMBER:
        /* These types own no separately allocated payload. */
        break;

    case JSON_STRING:
        /* TinyJSON owns dynamically allocated string storage. */
        if (value->stringValue != NULL)
        {
            free((void *)value->stringValue);
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
