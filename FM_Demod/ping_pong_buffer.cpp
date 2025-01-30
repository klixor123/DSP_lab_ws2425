#include "global.h"



void ping_pong_buffer_init(ping_pong_buffer_instance* pp_buffer, buffer_mode mode)
{
    // clear buffer ping
    memset(pp_buffer->ping, 0, sizeof(pp_buffer->ping));
    // clear buffer pong
    memset(pp_buffer->pong, 0, sizeof(pp_buffer->pong));

    // clear access semaphores
    pp_buffer->ping_accessible = true;
    pp_buffer->pong_accessible = true;

    // reset mode to given value
    pp_buffer->ping_mode = mode;
    pp_buffer->pong_mode = mode;
}


int32_t* ping_pong_buffer_get_read_ptr(ping_pong_buffer_instance* pp_buffer)
{
    // lock out ISR
    NVIC_DisableIRQ(DSTC_HW_IRQn);

    // check ping buffer
    if(pp_buffer->ping_accessible)
    {
        // release old and take new semaphore
        pp_buffer->pong_accessible = true;
        pp_buffer->ping_accessible = false;

        if(pp_buffer->ping_mode == read)
        {
            pp_buffer->ping_mode = write;

            // unlock ISR
            NVIC_EnableIRQ(DSTC_HW_IRQn);

            return pp_buffer->ping;
        }
        else
        {
            // ping is read from, pong not yet ready to read
            return NULL;
        }
    }

    // check pong buffer
    if(pp_buffer->pong_accessible)
    {
        // release old and take new semaphore
        pp_buffer->ping_accessible = true;
        pp_buffer->pong_accessible = false;

        if(pp_buffer->pong_mode == read)
        {
            pp_buffer->pong_mode = write;

            // unlock ISR
            NVIC_EnableIRQ(DSTC_HW_IRQn);

            return pp_buffer->pong;
        }
        else
        {
            // pong is read from, ping not yet ready to read
            return NULL;
        }
    }

    // error: no buffer readable
    fatal_error();
    return NULL;
}


int32_t* ping_pong_buffer_get_write_ptr(ping_pong_buffer_instance* pp_buffer)
{
    // lock out ISR
    NVIC_DisableIRQ(DSTC_HW_IRQn);

    // check ping buffer
    if(pp_buffer->ping_accessible)
    {
        // release old and take new semaphore
        pp_buffer->pong_accessible = true;
        pp_buffer->ping_accessible = false;

        if(pp_buffer->ping_mode == write)
        {
            pp_buffer->ping_mode = read;

            // unlock ISR
            NVIC_EnableIRQ(DSTC_HW_IRQn);

            return pp_buffer->ping;
        }
        else
        {
            // ping is written to, pong not yet ready to write
            return NULL;
        }
    }

    // check pong buffer
    if(pp_buffer->pong_accessible)
    {
        // release old and take new semaphore
        pp_buffer->ping_accessible = true;
        pp_buffer->pong_accessible = false;

        if(pp_buffer->pong_mode == write)
        {
            pp_buffer->pong_mode = read;

            // unlock ISR
            NVIC_EnableIRQ(DSTC_HW_IRQn);

            return pp_buffer->pong;
        }
        else
        {
            // pong is written to, ping not yet ready to write
            return NULL;
        }
    }

    // error: no buffer writeable
    fatal_error();
    return NULL;
}


bool ping_pong_buffer_can_read(ping_pong_buffer_instance* pp_buffer)
{
    if(pp_buffer->ping_mode == read)
    {
        return true;
    }

    if(pp_buffer->pong_mode == read)
    {
        return true;
    }

    return false;
}


bool ping_pong_buffer_can_write(ping_pong_buffer_instance* pp_buffer)
{
    if(pp_buffer->ping_mode == write)
    {
        return true;
    }

    if(pp_buffer->pong_mode == write)
    {
        return true;
    }

    return false;
}
