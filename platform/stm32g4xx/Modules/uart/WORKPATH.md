# UART Implementation Tasks

## Complete

- [x] CubeMX owns UART4/UART5 initialization and `HAL_UART_MspInit()`.
- [x] CubeMX configures UART clocks, GPIO alternate functions, 115200 8N1,
      and UART4 NVIC.
- [x] `UART4_IRQHandler()` calls `HAL_UART_IRQHandler(&huart4)`.
- [x] `UartInterface` provides byte-oriented `init()`, `send()`, and `read()`.
- [x] `UartSTM32G4XX::init()` starts a one-byte HAL receive interrupt.
- [x] `UartRxBuffer` provides a receive ring and overflow flag.
- [x] `UartSTM32G4XX::process_rxbuf()` reads the ring and decodes complete
      frames into `msg_t`.
- [x] `dispatch()` is implemented and accepts `const msg_t&`.
- [x] Transmit currently uses blocking `HAL_UART_Transmit()` rather than an
      interrupt or queued transmit.
- [x] COBS/CRC encoding and decoding unit tests exist and pass.

## Platform-specific tasks

### Receive interrupt wiring

- [ ] The `HAL_UART_RxCpltCallback(UART_HandleTypeDef*)` callback is currently
      deferred to the application.
- [ ] Confirm whether receive callback ownership should remain application-
      specific or move into a platform registration API.
- [ ] Implement the callback so it:
      1. identifies the UART handle;
      2. pushes `rx_byte_` into `UartRxBuffer`;
      3. re-arms `HAL_UART_Receive_IT()`.
- [ ] Change `rx_interrupt_callback()` to use the class receive member when
      callback code is connected.
- [ ] Confirm the callback is defined exactly once with C linkage.
- [ ] Add receive-error handling for overrun, noise, parity, and framing.
- [ ] Confirm UART5 IRQ and callback routing if UART5 is used.

### Receive frame processing

- [ ] Review whether `FrameBuf` should exclude the final `0x00` delimiter
      before calling `convert_from_uart()`.
- [ ] Pass the exact COBS frame length rather than assuming the fixed frame
      capacity.
- [ ] Replace `push_overwrite()` frame assembly with explicit overflow/discard
      state.
- [ ] Reset frame state after every delimiter, valid or invalid.
- [ ] Reject empty and oversized frames before conversion.
- [ ] Use an initialized output frame:
      `uint8_t raw_frame[COMMS_UART_MSG_SIZE]{}`.
- [ ] Surface conversion/decode failures through status or counters.
- [ ] Verify that an incomplete frame cannot trigger dispatch later.

### Transmit

- [ ] Blocking transmit is complete for a first HIL implementation, but it
      uses `HAL_TIMEOUT` and can block the application.
- [ ] Map meaningful HAL transmit failures rather than always returning
      `NOT_OK`.
- [ ] Before interrupt transmit, define the expected concurrent-send behavior.
- [ ] Interrupt/DMA transmit is intentionally not implemented yet.
- [ ] Keep interrupt/DMA transmit data alive until HAL reports completion.
- [ ] Add `TX_BUSY` and `TX_QUEUE_FULL` only if a queued or interrupt transmit
      is implemented.

## Application-specific tasks

### HAL callback ownership

- [ ] Starred/deferred: install `HAL_UART_RxCpltCallback()` in application
      code.
- [ ] Map CubeMX `huart4`/`huart5` handles to application-owned
      `UartSTM32G4XX` objects.
- [ ] Initialize the mapping before calling `UartSTM32G4XX::init()`.
- [ ] Do not define `HAL_UART_RxCpltCallback()` twice.

### UART object setup

- [ ] Construct `UartSTM32G4XX` objects with CubeMX handles and application
      message handlers.
- [ ] Call `uart.init()` during application startup.
- [ ] Call `uart.process_rxbuf()` periodically in the application superloop.
- [ ] Add UART initialization and processing to the actual application module.
- [ ] Instantiate the UART only after `MX_UARTx_Init()` in CubeMX startup.

### Message dispatch

- [ ] Implement the application message handler.
- [ ] Dispatch decoded messages by `message.type`.
- [ ] Handle each message payload in application code.
- [ ] Explicitly handle unknown and invalid message types.

### HIL verification

- [ ] Compile the PDB firmware after the latest UART changes.
- [ ] Implement the application callback and verify one raw byte.
- [ ] Verify receive callback re-arming.
- [ ] Send one complete encoded `msg_t` and verify dispatch.
- [ ] Test blocking transmit from the STM32 to a host.
- [ ] Test UART loopback.
- [ ] Test empty, repeated, oversized, and corrupted frames.
- [ ] Test receive-ring overflow and recovery.
- [ ] Test blocking transmit timing and concurrent sends.
