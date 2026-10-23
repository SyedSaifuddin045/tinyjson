#include "unity.h"
#include "tinyjson.h"

#include <stdint.h>
#include <string.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_value_null_constructor(void)
{
    JSONValue v;
    v.type = JSON_STRING; /* junk: constructor must overwrite everything */

    json_value_null(&v);

    TEST_ASSERT_EQUAL_INT(JSON_NULL, v.type);
}

void test_value_bool_constructor(void)
{
    JSONValue v;
    json_value_bool(&v, true);

    TEST_ASSERT_EQUAL_INT(JSON_BOOL, v.type);
    TEST_ASSERT_TRUE(v.boolValue);

    json_value_bool(&v, false);
    TEST_ASSERT_FALSE(v.boolValue);
}

void test_value_int_constructor(void)
{
    JSONValue v;
    json_value_int(&v, INT64_C(-9000000000));

    TEST_ASSERT_EQUAL_INT(JSON_INT, v.type);
    TEST_ASSERT_EQUAL_INT64(INT64_C(-9000000000), v.intValue);
    TEST_ASSERT_EQUAL_INT(JSON_INT, v.type);
}

void test_value_number_constructor(void)
{
    JSONValue v;
    json_value_number(&v, 2.5);

    TEST_ASSERT_EQUAL_INT(JSON_NUMBER, v.type);
    TEST_ASSERT_EQUAL_DOUBLE(2.5, v.numberValue);
}

void test_value_string_constructor_copies_input(void)
{
    JSONValue v;
    char buffer[8];

    strcpy(buffer, "hello");
    TEST_ASSERT_TRUE(json_value_string(&v, buffer));

    TEST_ASSERT_EQUAL_INT(JSON_STRING, v.type);
    TEST_ASSERT_EQUAL_STRING("hello", v.stringValue);

    buffer[0] = 'X';
    TEST_ASSERT_EQUAL_STRING("hello", v.stringValue);

    json_value_free(&v);
}

void test_value_string_constructor_rejects_null(void)
{
    JSONValue v;
    json_value_bool(&v, true);

    TEST_ASSERT_FALSE(json_value_string(&v, NULL));
    TEST_ASSERT_EQUAL_INT(JSON_BOOL, v.type); /* untouched on failure */
}

void test_value_free_is_idempotent(void)
{
    JSONValue v;

    TEST_ASSERT_TRUE(json_value_string(&v, "hello"));

    json_value_free(&v);
    TEST_ASSERT_EQUAL_INT(JSON_NULL, v.type);
    TEST_ASSERT_NULL(v.stringValue);

    json_value_free(&v); /* must not double free */
    TEST_ASSERT_EQUAL_INT(JSON_NULL, v.type);
}

void test_value_free_int_owns_nothing(void)
{
    JSONValue v;
    json_value_int(&v, 7);

    json_value_free(&v);
    TEST_ASSERT_EQUAL_INT(JSON_NULL, v.type);
}

void test_value_copy_scalar(void)
{
    JSONValue src;
    JSONValue dst;

    json_value_int(&src, 11);
    TEST_ASSERT_TRUE(json_value_copy(&dst, &src));

    TEST_ASSERT_EQUAL_INT(JSON_INT, dst.type);
    TEST_ASSERT_EQUAL_INT64(11, dst.intValue);
}

void test_value_copy_string_is_deep(void)
{
    JSONValue src;
    JSONValue dst;

    TEST_ASSERT_TRUE(json_value_string(&src, "hello"));
    TEST_ASSERT_TRUE(json_value_copy(&dst, &src));

    /* Mutating the source must not affect the copy. */
    src.stringValue[0] = 'X';

    TEST_ASSERT_EQUAL_STRING("hello", dst.stringValue);

    json_value_free(&src);
    TEST_ASSERT_EQUAL_STRING("hello", dst.stringValue); /* copy survives */

    json_value_free(&dst);
}

void test_value_copy_array_is_deep(void)
{
    JSONArray *source_array = json_array_init();
    TEST_ASSERT_NOT_NULL(source_array);

    JSONValue item;
    TEST_ASSERT_TRUE(json_value_string(&item, "first"));
    TEST_ASSERT_TRUE(json_array_push(source_array, item));
    json_value_int(&item, 2);
    TEST_ASSERT_TRUE(json_array_push(source_array, item));

    JSONValue src;
    memset(&src, 0, sizeof src);
    src.type = JSON_ARRAY;
    src.arrayValue = source_array;

    JSONValue dst;
    TEST_ASSERT_TRUE(json_value_copy(&dst, &src));

    TEST_ASSERT_EQUAL_INT(JSON_ARRAY, dst.type);
    TEST_ASSERT_NOT_NULL(dst.arrayValue);
    TEST_ASSERT_EQUAL_UINT(2, (unsigned)dst.arrayValue->size);
    TEST_ASSERT_EQUAL_STRING("first", dst.arrayValue->values[0].stringValue);
    TEST_ASSERT_EQUAL_INT64(2, dst.arrayValue->values[1].intValue);

    json_value_free(&src); /* frees source array */
    TEST_ASSERT_EQUAL_STRING("first", dst.arrayValue->values[0].stringValue);

    json_value_free(&dst);
}

void test_value_copy_object_is_deep(void)
{
    JSONObject *source_obj = json_object_init();
    TEST_ASSERT_NOT_NULL(source_obj);

    JSONValue item;
    json_value_int(&item, 42);
    TEST_ASSERT_TRUE(json_object_set(source_obj, "answer", item));

    TEST_ASSERT_TRUE(json_value_string(&item, "text"));
    TEST_ASSERT_TRUE(json_object_set(source_obj, "label", item));

    JSONValue src;
    memset(&src, 0, sizeof src);
    src.type = JSON_OBJECT;
    src.objectValue = source_obj;

    JSONValue dst;
    TEST_ASSERT_TRUE(json_value_copy(&dst, &src));

    TEST_ASSERT_EQUAL_INT(JSON_OBJECT, dst.type);
    TEST_ASSERT_NOT_NULL(dst.objectValue);
    TEST_ASSERT_EQUAL_UINT(2, (unsigned)dst.objectValue->size);

    const JSONValue *answer = json_object_get(dst.objectValue, "answer");
    const JSONValue *label = json_object_get(dst.objectValue, "label");
    TEST_ASSERT_NOT_NULL(answer);
    TEST_ASSERT_NOT_NULL(label);
    TEST_ASSERT_EQUAL_INT64(42, answer->intValue);
    TEST_ASSERT_EQUAL_STRING("text", label->stringValue);

    json_value_free(&src); /* frees source object + keys */
    TEST_ASSERT_EQUAL_STRING("text", json_object_get(dst.objectValue, "label")->stringValue);

    json_value_free(&dst);
}

void test_value_copy_rejects_null(void)
{
    JSONValue v;
    json_value_int(&v, 1);

    TEST_ASSERT_FALSE(json_value_copy(NULL, &v));
    TEST_ASSERT_FALSE(json_value_copy(&v, NULL));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_value_null_constructor);
    RUN_TEST(test_value_bool_constructor);
    RUN_TEST(test_value_int_constructor);
    RUN_TEST(test_value_number_constructor);
    RUN_TEST(test_value_string_constructor_copies_input);
    RUN_TEST(test_value_string_constructor_rejects_null);
    RUN_TEST(test_value_free_is_idempotent);
    RUN_TEST(test_value_free_int_owns_nothing);
    RUN_TEST(test_value_copy_scalar);
    RUN_TEST(test_value_copy_string_is_deep);
    RUN_TEST(test_value_copy_array_is_deep);
    RUN_TEST(test_value_copy_object_is_deep);
    RUN_TEST(test_value_copy_rejects_null);
    return UNITY_END();
}
