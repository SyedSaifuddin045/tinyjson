#include "unity.h"
#include "tinyjson.h"

#include <stdio.h>
#include <string.h>

void setUp(void)
{
}

void tearDown(void)
{
}

static JSONValue number(double value)
{
    JSONValue v;
    memset(&v, 0, sizeof v);
    v.type = JSON_NUMBER;
    v.numberValue = value;
    return v;
}

void test_object_init_returns_empty_object(void)
{
    JSONObject *obj = json_object_init();

    TEST_ASSERT_NOT_NULL(obj);
    TEST_ASSERT_EQUAL_UINT(0, (unsigned)obj->size);
    TEST_ASSERT_EQUAL_UINT(0, (unsigned)obj->capacity);
    TEST_ASSERT_NULL(obj->pairs);

    json_object_free(obj);
}

void test_object_free_null_is_noop(void)
{
    json_object_free(NULL);
}

void test_object_set_then_get(void)
{
    JSONObject *obj = json_object_init();

    TEST_ASSERT_TRUE(json_object_set(obj, "answer", number(42.0)));
    TEST_ASSERT_EQUAL_UINT(1, (unsigned)obj->size);

    const JSONValue *v = json_object_get(obj, "answer");
    TEST_ASSERT_NOT_NULL(v);
    TEST_ASSERT_EQUAL_INT(JSON_NUMBER, v->type);
    TEST_ASSERT_EQUAL_DOUBLE(42.0, v->numberValue);

    json_object_free(obj);
}

void test_object_set_replaces_existing_key(void)
{
    JSONObject *obj = json_object_init();

    TEST_ASSERT_TRUE(json_object_set(obj, "a", number(1.0)));
    TEST_ASSERT_TRUE(json_object_set(obj, "a", number(2.0)));

    TEST_ASSERT_EQUAL_UINT(1, (unsigned)obj->size);
    TEST_ASSERT_EQUAL_DOUBLE(2.0, json_object_get(obj, "a")->numberValue);

    json_object_free(obj);
}

void test_object_get_missing_key_returns_null(void)
{
    JSONObject *obj = json_object_init();

    TEST_ASSERT_TRUE(json_object_set(obj, "present", number(1.0)));
    TEST_ASSERT_NULL(json_object_get(obj, "absent"));

    json_object_free(obj);
}

void test_object_get_rejects_null_arguments(void)
{
    JSONObject *obj = json_object_init();

    TEST_ASSERT_TRUE(json_object_set(obj, "a", number(1.0)));
    TEST_ASSERT_NULL(json_object_get(NULL, "a"));
    TEST_ASSERT_NULL(json_object_get(obj, NULL));

    json_object_free(obj);
}

void test_object_set_rejects_null_arguments(void)
{
    JSONObject *obj = json_object_init();
    JSONValue v = number(1.0);

    TEST_ASSERT_FALSE(json_object_set(NULL, "a", v));
    TEST_ASSERT_FALSE(json_object_set(obj, NULL, v));
    TEST_ASSERT_EQUAL_UINT(0, (unsigned)obj->size);

    json_object_free(obj);
}

void test_object_set_copies_key(void)
{
    JSONObject *obj = json_object_init();
    char key[8];

    strcpy(key, "abc");
    TEST_ASSERT_TRUE(json_object_set(obj, key, number(1.0)));

    /* Mutate the caller's buffer: the stored key must be unaffected. */
    key[0] = 'X';

    TEST_ASSERT_NULL(json_object_get(obj, "Xbc"));
    TEST_ASSERT_NOT_NULL(json_object_get(obj, "abc"));

    json_object_free(obj);
}

void test_object_grows_beyond_initial_capacity(void)
{
    JSONObject *obj = json_object_init();
    enum { COUNT = 32 };
    char key[16];

    for (int i = 0; i < COUNT; i++)
    {
        snprintf(key, sizeof key, "k%d", i);
        TEST_ASSERT_TRUE(json_object_set(obj, key, number((double)i)));
    }

    TEST_ASSERT_EQUAL_UINT(COUNT, (unsigned)obj->size);
    TEST_ASSERT_TRUE(obj->capacity >= (size_t)COUNT);

    for (int i = 0; i < COUNT; i++)
    {
        snprintf(key, sizeof key, "k%d", i);
        const JSONValue *v = json_object_get(obj, key);
        TEST_ASSERT_NOT_NULL(v);
        TEST_ASSERT_EQUAL_DOUBLE((double)i, v->numberValue);
    }

    json_object_free(obj);
}

void test_object_free_releases_string_payloads(void)
{
    JSONObject *obj = json_object_init();
    JSONValue s;

    memset(&s, 0, sizeof s);
    s.type = JSON_STRING;
    s.stringValue = malloc(6);
    TEST_ASSERT_NOT_NULL(s.stringValue);
    memcpy(s.stringValue, "hello", 6);

    TEST_ASSERT_TRUE(json_object_set(obj, "greeting", s));

    /* ASan fails the test run if this leaks. */
    json_object_free(obj);
}

void test_object_free_frees_nested_containers(void)
{
    JSONObject *inner = json_object_init();
    TEST_ASSERT_NOT_NULL(inner);
    TEST_ASSERT_TRUE(json_object_set(inner, "x", number(1.0)));

    JSONObject *outer = json_object_init();
    TEST_ASSERT_NOT_NULL(outer);

    JSONValue nested;
    memset(&nested, 0, sizeof nested);
    nested.type = JSON_OBJECT;
    nested.objectValue = inner;

    TEST_ASSERT_TRUE(json_object_set(outer, "inner", nested));

    /* ASan fails the run if `inner` or its key/value leaks. */
    json_object_free(outer);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_object_init_returns_empty_object);
    RUN_TEST(test_object_free_null_is_noop);
    RUN_TEST(test_object_set_then_get);
    RUN_TEST(test_object_set_replaces_existing_key);
    RUN_TEST(test_object_get_missing_key_returns_null);
    RUN_TEST(test_object_get_rejects_null_arguments);
    RUN_TEST(test_object_set_rejects_null_arguments);
    RUN_TEST(test_object_set_copies_key);
    RUN_TEST(test_object_grows_beyond_initial_capacity);
    RUN_TEST(test_object_free_releases_string_payloads);
    RUN_TEST(test_object_free_frees_nested_containers);
    return UNITY_END();
}
