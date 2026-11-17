/** 
* @brief Unit tests for crc modules 
**/

#include "crc.h"
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <string_view>

/**
* @brief overload function for convenience
* @note able to pass string directly as arg, for example "Shalom". This is to make testing easier, idk if this is a good practice tho
*/
uint16_t crc16(std::string_view str) {
    return crc16(reinterpret_cast<const uint8_t*>(str.data()),str.size());
}


/** 
* @note this test suite was not developed with TDD in mind
*/
const char msg[] = "Shalom World!\r\n";

TEST_CASE("CRC16 standard cases", "[crc16][unit][correctness]") {
    SECTION("hej") {
        REQUIRE( crc16("Shalom World!\r\n") == 0x9E12);
    }

    SECTION("parsing \\ + r instead of \\r") {
        REQUIRE_FALSE( crc16("Shalom World!\r\n") == 0xBD0B);
    }
    
    SECTION("Standard test vector") {
        REQUIRE( crc16("123456789") == 0x29B1);
    }

    SECTION("CRC-16: one-bit error") {
        uint8_t data[] = {0x12, 0x34, 0x56};
        const uint16_t original = crc16(data, 3);
        
        data[2] ^= 0x01; // flip one bit, emulate transmission failure 
        const uint16_t corrupted = crc16(data, 3);

        REQUIRE_FALSE(original == corrupted);
    }
}

TEST_CASE("CRC16 edges cases", "[crc16][edge]") {
    SECTION("CRC16 empty data") {
        REQUIRE( crc16(nullptr, 0) == 0xFFFF);
    }

    SECTION("CRC 16 single byte") {
        const uint8_t data[] = {0x00};
        REQUIRE( crc16(data, 1) == 0xE1F0);
    }
}