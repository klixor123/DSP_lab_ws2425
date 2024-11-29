#pragma once

// C++
#include <stdio.h>


// PDL
extern "C"
{
#include <platform.h>
#include <utils.h>
#include <pdl_header.h>
#include <kiss_fft.h>
#include <kiss_fftr.h>
}


// CMSIS-DSP
#include "arm_math.h"


// custom
#include "debug_utils.h"


extern "C"
{
    extern volatile uint8_t rx_proc_buffer;
    extern volatile uint8_t tx_proc_buffer;
    extern volatile boolean_t tx_buffer_empty;
    extern volatile boolean_t rx_buffer_full;
}
