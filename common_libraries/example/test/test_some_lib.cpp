#define CATCH_CONFIG_MAIN // Let catch2 handle the main function and boiler plate code.
#include <catch2/catch_all.hpp>

// Code under test
#include <some_lib.h>

#define EXTERNAL_OBJECT_INIT_DEFAULT 420
#define VALID_INT32_TO_DOUBLE 100

TEST_CASE("test_some_lib")
{
    uint32_t some_external_object = EXTERNAL_OBJECT_INIT_DEFAULT;
    SomeLib lib_instance(some_external_object);

    SECTION("test_valid_double") // "Happy path" test -> Make sure the code works with valid inputs
    {
        // Setup objects
        uint32_t my_int = VALID_INT32_TO_DOUBLE;
        uint32_t my_doubled_int = 0;

        // "inject" data / call functions
        bool ret = lib_instance.double_int32(my_int, my_doubled_int);
        internal_status_t status = lib_instance.get_status();

        // Assert outputs
        REQUIRE(ret == true);
        REQUIRE(my_int == VALID_INT32_TO_DOUBLE); // my_int should not of changed
        REQUIRE(status.num_failures == 0);
        REQUIRE(status.num_operations == 1);
        REQUIRE(status.num_successes == 1);
        REQUIRE(some_external_object == EXTERNAL_OBJECT_INIT_DEFAULT);
        REQUIRE(my_doubled_int == 2*my_int);
    }

    SECTION("test_invalid_double") // "unhappy path" test -> Make sure the code raises errors with invalid inputs / Make sure the code fails successfully
    {
        uint32_t my_int = -1; // Max out uint32
        uint32_t my_doubled_int = 99; 

        bool ret = lib_instance.double_int32(my_int, my_doubled_int);
        internal_status_t status = lib_instance.get_status();

        REQUIRE(ret == false);
        REQUIRE(status.num_failures == 1);
        REQUIRE(status.num_operations == 1);
        REQUIRE(status.num_successes == 0);
        REQUIRE(some_external_object == EXTERNAL_OBJECT_FAILURE_VALUE); // on error, external object should be changed. Idk why
        REQUIRE(my_doubled_int == 99); // dst object should not change
    }

}