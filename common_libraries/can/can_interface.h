#pragma once
#include <comms.h> //msg types


enum class CanStatus {
    OK,
    NOT_OK, // GEneric not ok
    RX_QUEUE_EMPTY, // Rx queue is emtpy. Nothing new to read

    ERR_TX_QUEUE_FULL, // tx queue is full. 
    ERR_TX_HW_BUSY,    // For interrupt based tx
    ERR_BUS_FAILURE   // Hardware failure (can peripheral is in error state). This can be caused by wiring issues or misconfigurations
};  

class CanInterface {
public:

    virtual CanStatus init() = 0;
    
    // Depending on the implementation, this may:
    // send blocking (blocks until message is sent)
    // fills up a queue thats handled by another thread
    // etc...
    virtual CanStatus send(const can_msg_t& msg) = 0;

    // Up to the user if this is a blocking read that touches hardware, or just checks a queue
    virtual CanStatus read(can_msg_t& msg) = 0;
    
};