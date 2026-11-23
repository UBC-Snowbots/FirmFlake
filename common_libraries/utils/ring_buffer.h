#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

// generic ring buffer, originally written for FrameBuf logic for UART
// ! more testing is required, wakinda AI-assisted
template <std::size_t Capacity>
class RingBuffer {
    static_assert(Capacity >= 2, "Capacity must be at least 2");
    static_assert((Capacity & (Capacity - 1)) == 0,
                  "Capacity must be a power of two");

public:
    // standard push, drops byte if full
    bool push(uint8_t byte) {
        if (full()) {
            overflow_ = true;
            return false;
        }

        buffer_[head_] = byte;
        head_ = (head_ + 1u) & kMask; // this pattern does the same thing as (head_ + 1) % capacity 
        size_++;
        return true;
    }

    // doesnt use ATomic shit, replaces old bytes, kinda like a python fixed-length deque
    void push_overwrite(uint8_t byte) {
        buffer_[head_] = byte;
        head_ = (head_ + 1u) & kMask;

        if (size_ == Capacity) {
            tail_ = (tail_ + 1u) & kMask;
        } else {
            size_++;
        }
    }

    // doesnt use ATomic shit
    bool pop(uint8_t& byte) {
        if (empty()) {
            return false;
        }

        byte = buffer_[tail_];
        tail_ = (tail_ + 1u) & kMask;
        size_--;
        return true;
    }

    // Drain multiple bytes at once
    std::size_t read(uint8_t* dest, std::size_t max_len) {
        if (dest == nullptr) {
            return 0;
        }

        std::size_t count = 0;
        while (count < max_len && pop(dest[count])) {
            count++;
        }
        return count;
    }

    // Copies the oldest bytes in FIFO order into dest.
    std::size_t copy_to(uint8_t* dest, std::size_t dest_capacity) const {
        if (dest == nullptr) {
            return 0;
        }

        const std::size_t count = (size_ < dest_capacity) ? size_ : dest_capacity;
        if (count == 0) { 
            return 0; 
        }

        const std::size_t first = ((tail_ + count) <= Capacity) ? count : Capacity - tail_; // length of first segment

        std::memcpy(dest, buffer_.data() + tail_, first * sizeof(uint8_t));

        // copy wrap-around bytes 
        if (first < count) { 
            std::memcpy(dest + first, buffer_.data(), (count - first) * sizeof(uint8_t));
        }

        return count;   
    }

    bool empty() { return size_ == 0; }

    bool full() { return size_ == Capacity; }

    std::size_t size() { return size_; }

    void clear() {
        head_ = 0;
        tail_ = 0;
        size_ = 0;
    }

    bool consume_overflow_flag() {
        const bool overflow = overflow_;
        overflow_ = false;
        return overflow;
    }

private:
    static constexpr uint32_t kMask = static_cast<uint32_t>(Capacity - 1);

    std::array<uint8_t, Capacity> buffer_{};
    uint32_t head_{0};
    uint32_t tail_{0};
    std::size_t size_{0};
    bool overflow_{false};
};
