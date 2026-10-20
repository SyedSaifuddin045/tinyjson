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
            JSONArray *array = value->arrayValue;

            /* Recursively free every element's payload. */
            for (size_t i = 0; i < array->size; i++)
            {
                json_value_free(&array->values[i]);
            }

            /* Free the array's element buffer. */
            free(array->values);

            /* Free the JSONArray structure itself. */
            free(array);

            value->arrayValue = NULL;
        }
        break;

    case JSON_OBJECT:
        if (value->objectValue != NULL)
        {
            JSONObject *object = value->objectValue;

            /* Recursively free every key-value entry. */
            for (size_t i = 0; i < object->size; i++)
            {
                JSONKeyValuePair *pair = &object->pairs[i];

                /* Free the owned key string. */
                if (pair->key != NULL)
                {
                    free(pair->key);
                    pair->key = NULL;
                }

                /* Free the value's owned payload. */
                json_value_free(&pair->value);
            }

            /* Free the entries buffer. */
            free(object->pairs);

            /* Free the JSONObject structure itself. */
            free(object);

            value->objectValue = NULL;
        }
        break;
    }

    /* Leave the value in a predictable state. */
    value->type = JSON_NULL;
}
