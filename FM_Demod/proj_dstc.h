#pragma once





/* DstcInit: DSTC module initialization functions
 * Configures DSTC channel 0 to transfer data from memory to I2S TX, and channel 1 from I2S RX to memory.
 *
 * rx_buf_ptr and tx_buf_ptr are initial buffer pointers
 */
void 
init_DSTC(
    void (*rx_handler)(void), 
    int32_t* rx_buf_ptr,
    void (*tx_handler)(void),
    int32_t* tx_buf_ptr
    );
