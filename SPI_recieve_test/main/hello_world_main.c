/* SPI Slave example, receiver (uses SPI Slave driver to communicate with sender)

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "driver/spi_slave.h"
#include "driver/gpio.h"

/*
SPI receiver (slave) example.

This example is supposed to work together with the SPI sender. It uses the standard SPI pins (MISO, MOSI, SCLK, CS) to
transmit data over in a full-duplex fashion, that is, while the master puts data on the MOSI pin, the slave puts its own
data on the MISO pin.

This example uses one extra pin: GPIO_HANDSHAKE is used as a handshake pin. After a transmission has been set up and we're
ready to send/receive data, this code uses a callback to set the handshake pin high. The sender will detect this and start
sending a transaction. As soon as the transaction is done, the line gets set low again.
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////// Please update the following configuration according to your HardWare spec /////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#define RCV_HOST    SPI2_HOST

#define GPIO_HANDSHAKE      2
#define GPIO_MOSI           13
#define GPIO_MISO           12
#define GPIO_SCLK           14
#define GPIO_CS             15

//Called after a transaction is queued and ready for pickup by master. We use this to set the handshake line high.
void my_post_setup_cb(spi_slave_transaction_t *trans)
{
    gpio_set_level(GPIO_HANDSHAKE, 1);
}

//Called after transaction is sent/received. We use this to set the handshake line low.
void my_post_trans_cb(spi_slave_transaction_t *trans)
{
    gpio_set_level(GPIO_HANDSHAKE, 0);
}

//Main application
void app_main(void)
{
    int n = 0;
    esp_err_t ret;

    //Configuration for the SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    //Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg = {
        .mode = 0,
        .spics_io_num = GPIO_CS,
        .queue_size = 3,
        .flags = 0,
        .post_setup_cb = my_post_setup_cb,
        .post_trans_cb = my_post_trans_cb
    };

    //Configuration for the handshake line
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = BIT64(GPIO_HANDSHAKE),
    };

    //Configure handshake line as output
    gpio_config(&io_conf);
    //Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
    gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLDOWN_ONLY);
    gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLDOWN_ONLY);
    gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);

    //Initialize SPI slave interface
    ret = spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, SPI_DMA_DISABLED);
    assert(ret == ESP_OK);

    
    char *recvbuf = spi_bus_dma_memory_alloc(RCV_HOST, 4, 0);
    assert(recvbuf);
    
    spi_slave_transaction_t t = {0};

    while (1) {
        memset(recvbuf, 0, 4);

        // 2. Configure for Receive-Only
        t.length = 4 * 8;
        t.rx_buffer = recvbuf;
        t.tx_buffer = NULL; // <-- CRITICAL: Tells the ESP32 not to transmit anything

        // 3. The post_setup_cb still fires automatically here, raising the handshake pin
        ret = spi_slave_transmit(RCV_HOST, &t, portMAX_DELAY);

        // Print the first 4 bytes to verify your payload arrived
        printf("Received %d bits: %02X %02X %02X %02X\n", (int)t.trans_len,
            recvbuf[0], recvbuf[1], recvbuf[2], recvbuf[3]); 

        // (Optional: You can leave your sleep/pause logic here if you want)
    }
}
