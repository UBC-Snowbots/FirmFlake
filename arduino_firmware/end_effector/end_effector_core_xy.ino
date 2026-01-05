#include <end_effector_base.h>
#include <ESP32SPISlave.h>

// This ESP is the peripheral, the bridge ESP is the controller 
ESP32SPISlave peripheral;

// Sending data buffer
uint8_t spi_peripheral_tx_buf[BUF_SIZE] = {0};

// Receiving data buffer
uint8_t spi_peripheral_rx_buf[BUF_SIZE] = {0};

void setup() {

  Serial.begin(115200);

  // Pin assignments can be customized
  // Default pins are: VSPI (MISO=19, MOSI=23, SCLK=18, SS=5)
  peripheral.begin(); // Uses default VSPI pins
  
  // Set up buffers
  peripheral.set_buffers(spi_peripheral_tx_buf, spi_peripheral_rx_buf, BUF_SIZE);

  Serial.println("Core X-Y: SPI Peripheral Initialized");

}

void loop() {

  // While no message has been recieved from the controller
  while (!peripheral.wait(portMAX_DELAY)) {
    ;
  }

  // Once a message has been received from the controller
  // Get the message size
  size_t msg_size = peripheral.get_received_trans_len();

  // Print the number of bytes received
  Serial.print("Core X-Y: Received ");
  Serial.print(msg_size);
  Serial.print(" bytes: ");

  // Print the msg
  for (uint16_t i = 0; i < msg_size; i++) {
      Serial.print(spi_peripheral_rx_buf[i], HEX);
      // Clear the buffer
      spi_peripheral_rx_buf[i] = 0; 
  }
  Serial.println();

  // Interpret the message and perform appropriate action
  handle_SPI_msg(spi_peripheral_rx_buf);

  // Get updated sensor values and send to the bridge module
  // The controller will only pick this message up on the next message that it delivers
  send_data_update_SPI_msg(spi_peripheral_rx_buf, spi_peripheral_tx_buf);

  // Clear message status
  peripheral.pop(); 

}