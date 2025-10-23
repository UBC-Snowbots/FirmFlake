#define CATCH_CONFIG_MAIN // Let catch2 handle the main function and boiler plate code.
#include <catch2/catch_all.hpp>
#include <low_pass_fir_filter.h>


TEST_CASE("test_constructor")
{

    SECTION("test_no_order")
    {
        LowPassFIRFilter filter_instance;
        REQUIRE(filter_instance.order == 1);
    }

    SECTION("valid_orders")
    {
        LowPassFIRFilter filter_instance_1(1);
        REQUIRE(filter_instance_1.order == 1);

        LowPassFIRFilter filter_instance_5(5);
        REQUIRE(filter_instance_5.order == 5);

        LowPassFIRFilter filter_instance_10(MAX_LP_FIR_ORDER);
        REQUIRE(filter_instance_10.order == MAX_LP_FIR_ORDER);
    }

    SECTION("test_order_too_low_or_too_high")
    {
        REQUIRE_THROWS_AS(LowPassFIRFilter(0), std::invalid_argument);
        REQUIRE_THROWS_AS(LowPassFIRFilter(MAX_LP_FIR_ORDER + 1), std::invalid_argument);
    }

}

TEST_CASE("test_update")
{
    SECTION("test_empty_buffer_same_input") {
        LowPassFIRFilter filter_instance(5);
        unsigned int val = 50;
        for (unsigned int i = 0; i < 7; i++) {
            REQUIRE(filter_instance.update(val) == 50);
        }
    }

    /*
        buffer = [50, 50, 50, 50, 50] -> output = 50
        buffer = [100, 50, 50, 50, 50] -> output = 60
        buffer = [100, 20, 50, 50, 50] -> output = 54
    */
    SECTION("test_empty_buffer_different_inputs") {
        LowPassFIRFilter filter_instance(4);
        unsigned int inputs[] = {50, 50, 50, 100, 20};
        unsigned int expected[] = {50, 50, 50, 60, 54};
        unsigned int n = 5;
        for (unsigned int i = 0; i < n; i++) {
            REQUIRE(filter_instance.update(inputs[i]) == expected[i]);
        }
    }


    SECTION("test_input_spikes") {
        LowPassFIRFilter filter_instance(1);
        unsigned int inputs[] = {50, 60, 70, 500, 100, 60, 50, 40, 50, 40, 50};
        unsigned int expected[] = {50, 55, 65, 285, 300, 80, 55, 45, 45, 45, 45};

        unsigned int n = 11;
        for (unsigned int i = 0; i < n; i++) {
            REQUIRE(filter_instance.update(inputs[i]) == expected[i]);
        }
    }
}