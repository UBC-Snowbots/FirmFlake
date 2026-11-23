# UART Implementation Tasks

## Complete

- [x] CubeMX owns UART4/UART5 initialization and `HAL_UART_MspInit()`.
- [x] CubeMX configures UART clocks, GPIO alternate functions, 115200 8N1,
      and UART4 NVIC.
- [x] `UART4_IRQHandler()` calls `HAL_UART_IRQHandler(&huart4)`.
- [x] `UartInterface` provides byte-oriented `init()`, `send()`, `read()`,
      and virtual teardown support.
- [x] `UartSTM32G4XX::init()` starts one-byte HAL interrupt receive.
- [x] `UartRxBuffer` provides interrupt receive storage and overflow state.
- [x] `UartSTM32G4XX::process_rxbuf()` drains the receive ring and invokes
      comms conversion, decoding, and dispatch.
- [x] `dispatch()` accepts `const msg_t&`.
- [x] UART transmit uses blocking `HAL_UART_Transmit()`.
- [x] COBS/CRC encoding and decoding unit tests exist.
- [x] PDB constructs
      `UartSTM32G4XX uart4(huart4, uart_msg_handler)`.
- [x] PDB implements `HAL_UART_RxCpltCallback()` and routes UART4 completion
      to `uart4.rx_interrupt_callback()`.
- [x] PDB declares a `uart_msg_handler(const msg_t&)` application handler.
- [x] PDB calls `uart4.init()` after CubeMX UART initialization.
- [x] PDB calls `uart4.process_rxbuf()` in the application superloop.
- [x] PDB UART callback and receive processing compile into `PDB_24V.elf`.
- [x] PDB executable source list includes `common_libraries/crc/crc.c`.

## Platform-specific tasks

### Receive interrupt wiring

- [x] PDB defines `HAL_UART_RxCpltCallback(UART_HandleTypeDef*)` with
      `extern "C"` linkage.
- [ ] Confirm the callback is linked as the HAL receive callback and is
      defined exactly once.
- [x] PDB calls `uart4.init()` during startup, after `MX_UART4_Init()`.
- [ ] Verify `rx_interrupt_callback()` receives a populated `rx_byte_` and
      successfully re-arms `HAL_UART_Receive_IT()`.
- [ ] Add receive-error handling for overrun, noise, parity, and framing.
- [ ] Add UART5 callback routing only if UART5 receive is required.

### Receive frame processing

- [x] Verified: `convert_from_uart()` receives the complete COBS frame,
      including the trailing `0x00` delimiter. The unit tests pass the output
      of `convert_to_uart()` directly to `convert_from_uart()`.
- [x] The copy return value is passed as the exact COBS frame length to
      `convert_from_uart()`.
- [x] The parser assigns the `copy_to()` return value directly to
      `frame_length`.
- [x] `push_overwrite()` is acceptable for MVP frame assembly. It keeps the
      newest bytes as the frame grows, rather than losing all receive bytes.
- [x] `FrameBuf.clear()` runs after every delimiter, including invalid frames.
- [x] Empty and frame-capacity checks run before conversion.
- [x] The raw frame output is initialized: `uint8_t raw_frame[...]{}`.
- [ ] Surface conversion/decode/unknown-message failures through status or
      counters.
- [ ] Verify incomplete frames cannot trigger dispatch later.

### Transmit

- [x] Blocking transmit using `HAL_UART_Transmit()` is sufficient for MVP and
      HIL testing, with the known tradeoff that it blocks the application.
- [ ] Map meaningful HAL transmit failures; do not always return `NOT_OK`.
- [ ] Interrupt/DMA transmit is not implemented.
- [ ] Before interrupt/DMA transmit, define concurrent-send behavior.
- [ ] Keep interrupt/DMA transmit buffers alive until completion.
- [ ] Add `TX_BUSY` and `TX_QUEUE_FULL` only if queued or interrupt transmit
      is implemented.

## Application-specific tasks

### HAL callback ownership

- [x] PDB owns and defines the UART4 receive callback.
- [x] PDB maps `huart4.Instance == UART4` to `uart4`.
- [x] PDB callback has C linkage.
- [x] UART receive initialization occurs before the application superloop.
- [ ] Keep the callback definition in one place.

### UART object setup

- [x] PDB constructs `UartSTM32G4XX` with `huart4` and
      `uart_msg_handler`.
- [x] PDB calls `uart4.init()` during startup.
- [x] PDB calls `uart4.process_rxbuf()` periodically in its superloop.
- [x] UART startup occurs after `MX_UART4_Init()` and before application work.

### Message dispatch

- [x] PDB defines a basic `uart_msg_handler(const msg_t&)`.
- [ ] Replace the heartbeat placeholder and implement application behavior
      by `message.type`.
- [ ] Handle required payload fields for each supported message type.
- [ ] Explicitly handle unknown message types.

### HIL verification

- [x] PDB firmware builds with callback C linkage and receive processing.
- [ ] Verify one raw UART4 byte reaches the ring.
- [ ] Verify callback re-arming after one-byte receive.
- [ ] Send one complete encoded `msg_t` and verify PDB dispatch.
- [ ] Test blocking transmit from PDB to a host.
- [ ] Test UART loopback.
- [ ] Test empty, repeated, oversized, and corrupted frames.
- [ ] Test receive-ring overflow and recovery.
- [ ] Test blocking transmit timing and concurrent sends.
