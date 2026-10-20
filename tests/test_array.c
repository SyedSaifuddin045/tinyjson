#include "unity.h"
#include "tinyjson.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_json_array_init_and_free(void)
{
    JSONArray arr;
    TEST_ASSERT_TRUE(json_array_init(&arr));
    TEST_ASSERT_EQUAL_UINT(0, (unsigned)arr.size);
    TEST_ASSERT_EQUAL_UINT(0, (unsigned)arr.capacity);
    TEST_ASSERT_NULL(arr.values);

    json_array_free(&arr);
    TEST_ASSERT_EQUAL_UINT(0, (unsigned)arr.size);
    TEST_ASSERT_EQUAL_UINT(0, (unsigned)arr.capacity);
    TEST_ASSERT_NULL(arr.values);
}

void test_json_array_push_null_value(void)
{
    JSONArray arr;
    json_array_init(&arr);

    JSONValue v;
    v.type = JSON_NULL;

    TEST_ASSERT_TRUE(json_array_push(&arr, v));
    TEST_ASSERT_EQUAL_UINT(1, (unsigned)arr.size);
    TEST_ASSERT_TRUE(arr.capacity >= 1);

    json_array_free(&arr);
}

void test_json_array_push_grows_capacity(void)
{
    JSONArray arr;
    json_array_init(&arr);

    for (int i = 0; i < 10; i++)
    {
        JSONValue v;
        v.type = JSON_NULL;
        TEST_ASSERT_TRUE(json_array_push(&arr, v));
    }

    TEST_ASSERT_EQUAL_UINT(10, (unsigned)arr.size);
    TEST_ASSERT_TRUE(arr.capacity >= 10);

    json_array_free(&arr);
}

void test_json_array_init_null(void)
{
    TEST_ASSERT_FALSE(json_array_init(NULL));
}

void test_json_array_push_null_array(void)
{
    JSONValue v;
    v.type = JSON_NULL;
    TEST_ASSERT_FALSE(json_array_push(NULL, v));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_json_array_init_and_free);
    RUN_TEST(test_json_array_push_null_value);
    RUN_TEST(test_json_array_push_grows_capacity);
    RUN_TEST(test_json_array_init_null);
    RUN_TEST(test_json_array_push_null_array);
    return UNITY_END();
}
