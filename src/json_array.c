#include "tinyjson.h"

#include <stdlib.h>
#include <stdint.h>

JSONArray* json_array_init(void)
{
    JSONArray* array = malloc(sizeof(JSONArray));

    if (array == NULL)
    {
        return NULL;
    }

    array->size = 0;
    array->capacity = 0;
    array->values = NULL;

    return array;
}

bool json_array_push(JSONArray *array, JSONValue value)
{
    if (array == NULL)
    {
        return false;
    }

    /* Grow the buffer if it is full. */
    if (array->size == array->capacity)
    {
        size_t new_capacity;

        if (array->capacity == 0)
        {
            new_capacity = 1;
        }
        else
        {
            /* Prevent overflow when doubling capacity. */
            if (array->capacity > SIZE_MAX / 2)
            {
                return false;
            }

            new_capacity = array->capacity * 2;
        }

        /* Prevent overflow when calculating allocation size. */
        if (new_capacity > SIZE_MAX / sizeof(*array->values))
        {
            return false;
        }

        JSONValue *new_values = realloc(
            array->values,
            new_capacity * sizeof(*array->values));

        if (new_values == NULL)
        {
            return false;
        }

        array->values = new_values;
        array->capacity = new_capacity;
    }

    /* Insert at the first unused slot. */
    array->values[array->size] = value;

    /* The insertion is now complete. */
    array->size++;

    return true;
}

const JSONValue *json_array_get(const JSONArray *array, size_t index)
{
    if (array == NULL || index >= array->size)
    {
        return NULL;
    }

    return &array->values[index];
}

void json_array_free(JSONArray *array)
{
    if (array == NULL)
    {
        return;
    }

    for (size_t i = 0; i < array->size; i++)
    {
        json_value_free(&array->values[i]);
    }

    free(array->values);
    free(array);
}
