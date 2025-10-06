#include "some_lib.h" // If path is direct/relative to this file
// #include <some_lib.h> // If the path is linked only through cmake

// Don't worry about why anyone would use this library. its just gibberish to serve as an example


bool SomeLib::double_int32(const uint32_t& input, uint32_t& output_dst)
{
    bool ret = false; // ret is a common variable name for the return value

    if(input & 0b1000'0000'0000'0000 != 0x0000)
    {
        // Input is too big to fit in an uint32
        ret = false;
        this->prv_update_status(ret);
        return ret; // Early return
    } else 
    {
        ret = true;
        this->prv_update_status(ret);
    }

    output_dst = input << 1;
    return ret;
}

const internal_status_t& SomeLib::get_status() const
{
    return this->internal_status;
}

void SomeLib::prv_update_status(bool operation_was_successfull)
{
    this->internal_status.num_operations++;

    if (operation_was_successfull)
    {
        this->internal_status.num_successes++;
    } else
    {
        this->external_object = EXTERNAL_OBJECT_FAILURE_VALUE;
        this->internal_status.num_failures++;
    }
}