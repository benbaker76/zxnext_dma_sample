/*******************************************************************************
 * Original ASM by David Saphier (em00k2020) / Z88DK port by Ben Baker (headkaze)
 * 
 * DMA sample demo for Sinclair ZX Spectrum Next.
 ******************************************************************************/

#include <arch/zxn.h>
#include <input.h>
#include <z80.h>
#include <intrinsic.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "samples.h"
#include "dma.h"
#include "bank.h"
#include "globals.h"

#pragma output CRT_ORG_CODE = 0x8184
#pragma output CLIB_MALLOC_HEAP_SIZE = 0
#pragma output CRT_ORG_BANK_15 = SAMPLE_BANK
#pragma output CRT_ORG_BANK_16 = SAMPLE_BANK
#pragma output CRT_ORG_BANK_17 = SAMPLE_BANK
#pragma output CRT_ORG_BANK_18 = SAMPLE_BANK
#pragma printf = "%d %p %c %s"

static uint8_t scalers[] =
{
    // Not used in the multisample demo
    SAMPLE_SCALER,SAMPLE_SCALER*2,SAMPLE_SCALER*3,SAMPLE_SCALER*4,SAMPLE_SCALER*5,
    SAMPLE_SCALER*6,SAMPLE_SCALER*7,SAMPLE_SCALER*8,SAMPLE_SCALER*9,SAMPLE_SCALER*10,SAMPLE_SCALER*12,
    0xff
};

typedef struct
{
    uint8_t page;
    bool loop;
    void *start;
    void *end;
} sample_table_t;

static sample_table_t sample_table[] =
{
    // bank, loop, start, end
    { 15, true,  mothership_start, mothership_end },
    { 15, false, steam_start,      steam_end },
    { 16, false, explode_start,    explode_end },
    { 17, true,  zapzapdii_start,  zapzapdii_end },
    { 17, false, dub1_start,       dub1_end },
    { 17, false, dub2_start,       dub2_end },
    { 17, false, dub3_start,       dub3_end },
    { 17, false, dub4_start,       dub4_end },
    { 17, true,  wawawawa_start,   wawawawa_end },
    { 18, true,  zzzzrrrttt_start, zzzzrrrttt_end },
    { 0,  false, 0xffff,           0xffff }
};

static void hardware_init(void)
{
    // Make sure the Spectrum ROM is paged in initially.
    IO_7FFD = IO_7FFD_ROM0;

    // Put Z80 in 28 MHz turbo mode.
    ZXN_NEXTREG(REG_TURBO_MODE, RTM_28MHZ);

    // Disable RAM memory contention.
    // ZXN_NEXTREGA(REG_PERIPHERAL_3, ZXN_READ_REG(REG_PERIPHERAL_3) | RP3_DISABLE_CONTENTION | RP3_ENABLE_TURBOSOUND);
}

static void isr_init(void)
{
    // Set up IM2 interrupt service routine:
    // Put Z80 in IM2 mode with a 257-byte interrupt vector table located
    // at 0x8000 (before CRT_ORG_CODE) filled with 0x81 bytes. Install the
    // vt_play_isr() interrupt service routine at the interrupt service routine
    // entry at address 0x8181.

    intrinsic_di();
    im2_init((void *) 0x8000);
    memset((void *) 0x8000, 0x81, 257);
    z80_bpoke(0x8181, 0xFB);
    z80_bpoke(0x8182, 0xED);
    z80_bpoke(0x8183, 0x4D);
    intrinsic_ei();
}

int main(void)
{
    int sample_index = 0;

    hardware_init();
    isr_init();

    printCls();
    printAt(10, 8, "Enjoy the sound!\n");
    printAt(12, 1, "Press any key to cycle samples\n");

    while (true)
    {
        if (in_inkey() != 0)
        {
            in_wait_nokey();
            
            bank_set_8k(SAMPLE_MMU, sample_table[sample_index].page);
            uint8_t *sample_source = zxn_addr_from_mmu(SAMPLE_MMU) + (uint16_t)sample_table[sample_index].start;
            uint16_t sample_length = (uint16_t)sample_table[sample_index].end - (uint16_t)sample_table[sample_index].start;
            
            dma_transfer_sample((void *)sample_source, sample_length, SAMPLE_SCALER*12, sample_table[sample_index].loop);

            if (sample_table[++sample_index].start == (void *)0xffff)
                sample_index = 0;
            
            intrinsic_halt();
        }
    }
    
    // Trig a soft reset. The Next hardware registers and I/O ports will be reset by NextZXOS after a soft reset.
    ZXN_NEXTREG(REG_RESET, RR_SOFT_RESET);

    return 0;
}
