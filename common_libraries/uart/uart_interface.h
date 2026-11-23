#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <comms.h>

enum class UartStatus {
    OK, // operation succeeded 
    NOT_OK, // UART generally failed 
    RX_EMPTY, // no received byte available, buffer empty 
    RX_QUEUE_FULL, // no room for another received byte
    TX_BUSY, // transmitter is currently occupied
    TX_QUEUE_FULL, // no room for another transmission
    FRAME_ERROR // received frame is invalid. drop frame? 
};

// Interrupt-to-main-loop receive buffer.
// Push from the receive interrupt only; pop from the main loop only.
template <std::size_t Capacity>
class UartRxBuffer {
    static_assert(Capacity >= 2, "Capacity must be at least 2");
    static_assert((Capacity & (Capacity - 1)) == 0,
                  "Capacity must be a power of two");

public:
    UartStatus push(uint8_t byte) noexcept {
        const uint32_t head = head_.load(std::memory_order_relaxed);
        const uint32_t next_head = (head + 1u) & kMask;

        if (next_head == tail_.load(std::memory_order_acquire)) {
            overflow_.store(true, std::memory_order_relaxed);
            return UartStatus::RX_QUEUE_FULL;
        }

        buffer_[head] = byte;
        head_.store(next_head, std::memory_order_release);
        return UartStatus::OK;
    }

    UartStatus pop(uint8_t& byte) noexcept {
        const uint32_t tail = tail_.load(std::memory_order_relaxed);

        if (tail == head_.load(std::memory_order_acquire)) {
            return UartStatus::RX_EMPTY;
        }

        byte = buffer_[tail];
        tail_.store((tail + 1u) & kMask, std::memory_order_release);
        return UartStatus::OK;
    }

    bool consume_overflow_flag() noexcept {
        return overflow_.exchange(false, std::memory_order_acq_rel);
    }

private:
    static constexpr uint32_t kMask = static_cast<uint32_t>(Capacity - 1);

    std::array<uint8_t, Capacity> buffer_{};
    std::atomic<uint32_t> head_{0};
    std::atomic<uint32_t> tail_{0};
    std::atomic<bool> overflow_{false};
};

class UartInterface {
    public:
        virtual UartStatus init() = 0; // sets it to like truly virtual or smt 
        
        // Transmits an already-framed byte sequence. 
        virtual UartStatus send(const uint8_t* data, size_t length) = 0;

        // reads one received byte for polling impelmentation? consider changing this when dma/interrupting 
        virtual UartStatus read(uint8_t& byte) = 0; 

        virtual ~UartInterface() = default; 
};