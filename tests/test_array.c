#include "unity.h"
#include "tinyjson.h"

#include <string.h>

void setUp(void)
{
}

void tearDown(void)
{
}

static JSONValue null_value(void)
{
    JSONValue v;
    memset(&v, 0, sizeof v);
    v.type = JSON_NULL;
    return v;
}

void test_array_init_returns_empty_array(void)
{
    JSONArray *arr = json_array_init();

    TEST_ASSERT_NOT_NULL(arr);
    TEST_ASSERT_EQUAL_UINT(0, (unsigned)arr->size);
    TEST_ASSERT_EQUAL_UINT(0, (unsigned)arr->capacity);
    TEST_ASSERT_NULL(arr->values);

    json_array_free(arr);
}

void test_array_free_null_is_noop(void)
{
    json_array_free(NULL);
}

void test_array_push_null_value(void)
{
    JSONArray *arr = json_array_init();

    TEST_ASSERT_TRUE(json_array_push(arr, null_value()));
    TEST_ASSERT_EQUAL_UINT(1, (unsigned)arr->size);
    TEST_ASSERT_TRUE(arr->capacity >= 1);

    json_array_free(arr);
}

void test_array_push_grows_capacity(void)
{
    JSONArray *arr = json_array_init();

    for (int i = 0; i < 10; i++)
    {
        TEST_ASSERT_TRUE(json_array_push(arr, null_value()));
    }

    TEST_ASSERT_EQUAL_UINT(10, (unsigned)arr->size);
    TEST_ASSERT_TRUE(arr->capacity >= 10);

    json_array_free(arr);
}

void test_array_push_null_array(void)
{
    TEST_ASSERT_FALSE(json_array_push(NULL, null_value()));
}

void test_array_get_returns_element(void)
{
    JSONArray *arr = json_array_init();

    JSONValue v;
    json_value_int(&v, 7);
    TEST_ASSERT_TRUE(json_array_push(arr, v));

    TEST_ASSERT_TRUE(json_array_push(arr, null_value()));

    const JSONValue *first = json_array_get(arr, 0);
    TEST_ASSERT_NOT_NULL(first);
    TEST_ASSERT_EQUAL_INT(JSON_INT, first->type);
    TEST_ASSERT_EQUAL_INT64(7, first->intValue);

    const JSONValue *second = json_array_get(arr, 1);
    TEST_ASSERT_NOT_NULL(second);
    TEST_ASSERT_EQUAL_INT(JSON_NULL, second->type);

    json_array_free(arr);
}

void test_array_get_out_of_range_returns_null(void)
{
    JSONArray *arr = json_array_init();

    TEST_ASSERT_TRUE(json_array_push(arr, null_value()));
    TEST_ASSERT_NULL(json_array_get(arr, 1));
    TEST_ASSERT_NULL(json_array_get(arr, 1000));

    json_array_free(arr);
}

void test_array_get_rejects_null_array(void)
{
    TEST_ASSERT_NULL(json_array_get(NULL, 0));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_array_init_returns_empty_array);
    RUN_TEST(test_array_free_null_is_noop);
    RUN_TEST(test_array_push_null_value);
    RUN_TEST(test_array_push_grows_capacity);
    RUN_TEST(test_array_push_null_array);
    RUN_TEST(test_array_get_returns_element);
    RUN_TEST(test_array_get_out_of_range_returns_null);
    RUN_TEST(test_array_get_rejects_null_array);
    return UNITY_END();
}
