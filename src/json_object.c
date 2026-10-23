#include "tinyjson.h"
#include <stddef.h>
#include <string.h>
#include <stdint.h>

/**
 * @brief Portable replacement for POSIX strdup().
 *
 * Allocates a NUL-terminated copy of @p s. Named with an internal_ prefix
 * because "strdup" itself is reserved by POSIX and may already be declared by
 * <string.h>.
 *
 * @param s NUL-terminated string to copy. Must not be NULL.
 * @return Heap-allocated copy owned by the caller, or NULL on allocation
 *         failure. Free with free().
 */
static char *internal_strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);

    if (copy == NULL)
        return NULL;

    memcpy(copy, s, len);
    return copy;
}

JSONObject* json_object_init(void)
{
    JSONObject* obj = malloc(sizeof(JSONObject));

    if(obj == NULL)
        return NULL;
    
    obj->capacity = 0;
    obj->size = 0;
    obj->pairs = NULL;

    return obj;
}

bool json_object_find(const JSONObject *obj, const char *key, size_t *out_index)
{
    if (obj == NULL || key == NULL)
        return false;

    for (size_t i = 0; i < obj->size; i++)
    {
        if (obj->pairs[i].key != NULL && strcmp(obj->pairs[i].key, key) == 0)
        {
            if (out_index != NULL)
                *out_index = i;
            return true;
        }
    }

    return false;
}

bool json_object_set(JSONObject* obj,const char* key,JSONValue value)
{
    if(obj == NULL || key == NULL)
        return false;

    size_t index;

    if (json_object_find(obj, key, &index))
    {
        json_value_free(&obj->pairs[index].value);
        obj->pairs[index].value = value;
        return true;
    }

    if (obj->size == obj->capacity)
    {
        size_t new_capacity = (obj->capacity == 0) ? 4 : obj->capacity * 2;

        if (obj->capacity > SIZE_MAX / 2)
            return false;

        if (new_capacity > SIZE_MAX / sizeof(*obj->pairs))
            return false;

        JSONKeyValuePair *new_pairs = realloc(
            obj->pairs, new_capacity * sizeof(*obj->pairs));

        if (new_pairs == NULL)
            return false;

        obj->pairs    = new_pairs;
        obj->capacity = new_capacity;
    }

    char *key_copy = internal_strdup(key);
    if (key_copy == NULL)
        return false;

    obj->pairs[obj->size].key = key_copy;
    obj->pairs[obj->size].value = value;
    obj->size++;

    return true;
}

const JSONValue* json_object_get(const JSONObject* obj, const char* key)
{
    size_t index;

    if (!json_object_find(obj, key, &index))
        return NULL;

    return &obj->pairs[index].value;
}

void json_object_free(JSONObject* obj)
{
    if(obj == NULL)
        return;

    for(size_t i = 0;i<obj->size;i++)
    {
        free(obj->pairs[i].key);
        json_value_free(&obj->pairs[i].value);
    }

    free(obj->pairs);
    free(obj);
}