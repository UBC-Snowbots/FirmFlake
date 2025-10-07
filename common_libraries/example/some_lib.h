#pragma once    // ensures this file is not included multiple times (similar to #IFNDEF SOME_LIB_H) 
#include <stdint.h> // Whatever includes this library needs

#define EXTERNAL_OBJECT_FAILURE_VALUE 69


typedef struct internal_status { 
    uint32_t num_successes  = 0;
    uint32_t num_failures   = 0;
    uint32_t num_operations = 0;
} internal_status_t;

class SomeLib {
public:
    SomeLib(uint32_t& external_object) : external_object(external_object){}; // Constructor, with external refrence input
    ~SomeLib() = default; // Deconstructor


    bool double_int32(const uint32_t& input, uint32_t& output_dst);
    
    const internal_status_t& get_status() const; // Read only getter that does not copy AND does not change internal variables, while still needing this->.
    

private:

    internal_status_t internal_status = {}; // Private instantiation of internal status

    uint32_t &external_object; 

    void prv_update_status(bool operation_was_successfull); // Private helper function to update internal status. Cannot be static as it needs `this` pointer
};