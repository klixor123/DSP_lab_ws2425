#pragma once



typedef enum ping_pong_buffer_mode
{
    read,
    write
}buffer_mode;


typedef struct
{
    // data buffers
    int32_t ping[BLOCK_SIZE];
    int32_t pong[BLOCK_SIZE];

    // access semaphore
    bool ping_accessible;
    bool pong_accessible;

    // buffer mode: read or write
    buffer_mode ping_mode;
    buffer_mode pong_mode;
} ping_pong_buffer_instance;


// Initializes a ping_pong_buffer
//
// pp_buffer        points to an instance of a ping_pong_buffer
// mode             initial mode (read or write) of the buffer, is applied to all internal buffers
void 
ping_pong_buffer_init(
    ping_pong_buffer_instance* pp_buffer,
    buffer_mode mode
    );


// returns a pointer to one of the internal buffers
// automatically manages access rights and mode
// will go to fatal_error if no buffer is accessible
//
// pp_buffer        points to an instance of a ping_pong_buffer
int32_t*
ping_pong_buffer_get_read_ptr(
    ping_pong_buffer_instance* pp_buffer
    );


// returns a pointer to one of the internal buffers
// automatically manages access rights and mode
// will go to fatal_error if no buffer is accessible
//
// pp_buffer        points to an instance of a ping_pong_buffer
int32_t*
ping_pong_buffer_get_write_ptr(
    ping_pong_buffer_instance* pp_buffer
    );


bool
ping_pong_buffer_can_read(
    ping_pong_buffer_instance* pp_buffer
    );


bool
ping_pong_buffer_can_write(
    ping_pong_buffer_instance* pp_buffer
    );
    

