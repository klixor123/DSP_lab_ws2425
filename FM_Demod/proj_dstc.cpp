#include "global.h"




void 
init_DSTC(
    void (*rx_handler)(void), 
    int32_t* rx_buf_ptr,
    void (*tx_handler)(void),
    int32_t* tx_buf_ptr
    )
{
	stc_dstc_config_t   	stcDstcConfig;	// dstc config structure

	// clear config and data structures
	PDL_ZERO_STRUCT(stcDstcConfig);
	PDL_ZERO_STRUCT(stcDES[0]);
	PDL_ZERO_STRUCT(stcDES[1]);

	while ( Ok != Dstc_ReleaseStandBy()){};	// wait till DSTC goes to normal state

	// populate descriptor structures

	// channel 0 = Reads from memory (dma_tx_buffer_ping and dma_tx_buffer_pong) and transfer to the I2S peripheral
	// CH0, DES0
	stcDES[0].DES0.DV    	= 0x03;		// Don't execute the DES close process after transfer ends
	stcDES[0].DES0.ST    	= 0u;		// Just a default, DSTC updates this on completion of transfer with status information
	stcDES[0].DES0.MODE  	= 1u;      	// Mode 1 -> single transfer for 1 trigger
	stcDES[0].DES0.ORL   	= 3u;      	// Outer reload for DES1, 2
	stcDES[0].DES0.TW    	= 2u;      	// 32-bit transfer width
	stcDES[0].DES0.SAC   	= 0u;      	// source address is increased by TW * 1 at every transfer without InnerReload
	stcDES[0].DES0.DAC   	= 5u;       // Destination address remains unchanged during the transfer
	stcDES[0].DES0.CHRS  	= 0x10u;    // Interrupt flag is set when IRM = 1 and ORM = 1. No Chain start
	stcDES[0].DES0.DMSET 	= 1u;       // Set DQMSK = 1 when DES close process is executed
	stcDES[0].DES0.CHLK  	= 0u;       // No Chain start transfer
	stcDES[0].DES0.ACK   	= 1u;       // Output DMA transfer acknowledge to peripheral
	stcDES[0].DES0.RESERVED = 0u;   	// Required
	stcDES[0].DES0.PCHK  	= DSTC_PCHK_CALC(stcDES[0].u32DES0);	// parity

	// CH0, DES1, counters can be set to 1 - 256, and multiples of 256
	stcDES[0].DES1_mode1.ORM = ((BLOCK_SIZE -1) >> 8) + 1;			// outer loop count
	stcDES[0].DES1_mode1.IIN = (stcDES[0].DES1_mode1.ORM > 1) ? 0 : BLOCK_SIZE & 0XFF;  // Inner loop, max 256; 256 = 0
	stcDES[0].DES1_mode1.IRM = stcDES[0].DES1_mode1.IIN;			// Same as IIN

	// CH0, DES2
	stcDES[0].DES2 = (uint32_t)tx_buf_ptr;   	// Source address (incremented by TW * 1 for every transfer. Configured in DES0.SAC)

	// CH0, DES3
	stcDES[0].DES3 = (uint32_t)&FM4_I2S0->TXFDAT;      	// Destination address - I2S Transmission data register (Same for every transfer,
														// configured in DES0.DAC)
	// CH0, DES4
	stcDES[0].DES4_mode1 = stcDES[0u].DES1_mode1;		// Used to reload DES1

	// Ch0, DES5
	stcDES[0].DES5 = stcDES[0].DES2;					// Used to reload DES 2 in outer reload

	Dstc_SetHwdesp(DSTC_IRQ_NUMBER_I2S0_TX, 0);			// descriptor pointer start address at DESTP + offset 0 for HW channel 219

	// channel 1 = Reads from the I2S peripheral and transfer to (dma_rx_buffer_ping and dma_rx_buffer_pong)
	// CH1, DES0
	stcDES[1].DES0.DV    = 0x03;           	// Don't Execute the DES close process after transfer ends
	stcDES[1].DES0.ST    = 0u;             	// Just a default, DSTC updates this on completion of transfer with status information
	stcDES[1].DES0.MODE  = 1u;             	// Mode 1 -> single transfer for 1 trigger
	stcDES[1].DES0.ORL   = 5u;             	// Outer reload for DES1, 3
	stcDES[1].DES0.TW    = 0x2;            	// 32-bit transfer width
	stcDES[1].DES0.SAC   = 5u;             	// Source address remains unchanged during the transfer
	stcDES[1].DES0.DAC   = 0u;             	// Destination address is incremented by TW * 1 at every transfer without reload
	stcDES[1].DES0.CHRS  = 0x10u;          	// Interrupt flag is set when IRM = 1 and ORM = 1. No Chain start
	stcDES[1].DES0.DMSET = 1u;             	// Set DQMSK = 1 when DES close process is executed
	stcDES[1].DES0.CHLK  = 0u;             	// No Chain start transfer
	stcDES[1].DES0.ACK   = 1u;             	// Output DMA transfer acknowledge to peripheral
	stcDES[1].DES0.RESERVED = 0u;			// Required
	stcDES[1].DES0.PCHK  = DSTC_PCHK_CALC(stcDES[1u].u32DES0);

	// CH1, DES1, counters can be set to 1 - 256, and multiples of 256
	stcDES[1].DES1_mode1.ORM = ((BLOCK_SIZE -1) >> 8) + 1;		// outer loop count
	stcDES[1].DES1_mode1.IIN = (stcDES[1].DES1_mode1.ORM > 1) ? 0 : BLOCK_SIZE & 0XFF;  // Inner loop, max 256; 256 = 0
	stcDES[1].DES1_mode1.IRM = stcDES[1].DES1_mode1.IIN;   		// Same as IIN

	// CH1, DES2
	stcDES[1].DES2 = (uint32_t)&FM4_I2S0->RXFDAT ;     	// Source address

	// CH1, DES3
	stcDES[1].DES3 = (uint32_t)rx_buf_ptr;   	// Destination address - I2S Transmission data register (Same for every transfer. Configured in DES0.DAC)

	// CH1, DES4
	stcDES[1].DES4_mode1 = stcDES[1].DES1_mode1;      	// used to reload DES1

	// CH1, DES6
	stcDES[1].DES6 = stcDES[1].DES3;					// Used to reload DES 3 in outer reload

	Dstc_SetHwdesp(DSTC_IRQ_NUMBER_I2S0_RX, 0x1C);		// descriptor pointer start address DESTP + offset 0x1C for HW channel 218
														// (7 DES0 x 4 Bytes each = 0x1C)
	// populate dstc config structure

	// DES Top, start Address of DES Area (must be aligned to 32 Bit!)
	stcDstcConfig.u32Destp = (uint32_t) &stcDES[0];
	// TRUE: Software Interrupt enabled
	stcDstcConfig.bSwInterruptEnable = FALSE;
	// TRUE: Error Interrupt enabled
	stcDstcConfig.bErInterruptEnable = FALSE;
	// TRUE: Read Skip Buffer disabled
	stcDstcConfig.bReadSkipBufferDisable = FALSE;		// no further changes made in DES0 after init
	// TRUE: Enables Error Stop
	stcDstcConfig.bErrorStopEnable = FALSE;
	// SW transfer priority
	stcDstcConfig.enSwTransferPriority = PriorityLowest;
	// TRUE: enable NVIC
	stcDstcConfig.bTouchNvic = TRUE;
	// interrupt handler
 	stcDstcConfig.pfnDstcI2s0TxCallback = tx_handler;		// pointer to interrupt service routine
	stcDstcConfig.pfnDstcI2s0RxCallback = rx_handler;


 	Dstc_Init(&stcDstcConfig);		// write config structure to CFG register and setup interrupt callback mechanism

 	Dstc_SetCommand(CmdSwclr);		// Command to clear the SWINT interrupt
	Dstc_SetCommand(CmdErclr);		// Command to clear ERINT interrupt. MONERS.EST = 0, MONERS.DER = 0, MONERS.ESTOP = 0
	Dstc_SetCommand(CmdMkclr);		// Command to clear all DQMSK[n] registers

	Dstc_SetDreqenbBit(DSTC_IRQ_NUMBER_I2S0_TX);		// enable HW channel 219
	Dstc_SetHwintclrBit(DSTC_IRQ_NUMBER_I2S0_TX);		// Clear HWINT6 register bit corresponding to HW channel 219(I2S tx)
	Dstc_SetDreqenbBit(DSTC_IRQ_NUMBER_I2S0_RX);		// enable HW channel 218
	Dstc_SetHwintclrBit(DSTC_IRQ_NUMBER_I2S0_RX);		// Clear HWINT6 register bit corresponding to HW channel 218 (I2S rx)

}
