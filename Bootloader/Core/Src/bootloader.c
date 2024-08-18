#include "bootloader.h"
#include "flash.h"
#include <string.h>

void goto_application(void) {
	HAL_RCC_DeInit(); //turn off peripherals, clear interrupt flags
	HAL_DeInit(); //clear pending interrupt request, turn off System Tick
	SCB->SHCSR &= ~(SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk
			| SCB_SHCSR_MEMFAULTENA_Msk);
	__set_MSP(*((volatile uint32_t*) FIRMWARE_ADDRESS));
	void (*reset_handler)(void) = (void*)(*((volatile uint32_t*) (FIRMWARE_ADDRESS + 4U)));
	reset_handler();
}

int update_firmware(u32 len) {
    u32 cur_addr_rd = FIRMWARE_TEM;
    u32 cur_addr_wr = FIRMWARE_ADDRESS;
    u8 page_data[PAGESIZE] = {0};
    u8 verify[PAGESIZE] = {0};
    u32 n_pages = len / PAGESIZE + 1;

    if (!len) {
    	return -1;
    }

    for (int i = 0; i < n_pages; i++) {
re_write:
        flash_read(cur_addr_rd, page_data, PAGESIZE);
        flash_write_data(cur_addr_wr, page_data, PAGESIZE); /* Write */

        /* Verify */
        flash_read(cur_addr_wr, verify, PAGESIZE);
        if (memcmp(page_data, verify, PAGESIZE)) { // data miss match
        	goto re_write;
        } else {
        	cur_addr_rd += PAGESIZE;
        	cur_addr_wr += PAGESIZE;
        }
    }
    return 0;
}
