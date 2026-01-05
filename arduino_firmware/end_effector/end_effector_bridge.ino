#include <CAN_FD_bridge.h>
#include <SPI.h>
#include <end_effector_base.h>

// Defines pin that peripheral ESP is connected to
#define SS_PIN 6

// Sending data buffer
uint8_t spi_controller_tx_buf[BUF_SIZE] = {0};

// Receiving data buffer
uint8_t spi_controller_rx_buf[BUF_SIZE] = {0};

void setup() {

  Serial.begin(115200);

  // Initialize SPI bus
  SPI.begin();

  // Set peripheral SS pin as inactive output (active-low)
  pinMode(SS_PIN, OUTPUT); 
  digitalWrite(SS_PIN, HIGH); 


  Serial.println("Bridge ESP: SPI Controller Initialized");

}

void loop() {

  // Translate CAN FD frame to SPI frame to send
  handle_CAN_FD_message(spi_controller_tx_buf);

  // Activate peripheral ESP connection for writing
  digitalWrite(SS_PIN, LOW);

  // Settings: 1MHz, Most significant bit first, data mode
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

  // Send all 32 bytes from buffer to the peripheral
  for (int i = 0; i < BUF_SIZE; i++) {
    // This sends a byte and receives a byte at the same time
    spi_controller_rx_buf[i] = SPI.transfer(spi_controller_tx_buf[i]); 
  }
  
  // Stop transmission
  SPI.endTransaction();

  // Deactivate peripheral connection
  digitalWrite(SS_PIN, HIGH);

  // Forward response message from peripheral onto CAN FD connection to Jetson
  handle_SPI_msg(spi_controller_rx_buf);

  // Print data received from peripheral
  Serial.print("Bridge ESP: Received bytes: ");
  for (int i = 0; i < BUF_SIZE; i++) {
    Serial.print(spi_controller_rx_buf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Delay before writing to peripheral again
  delay(1000);

}