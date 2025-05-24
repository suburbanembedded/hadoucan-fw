add_library(stm32h743xx_cmsis 
	${STM32CUBEH7_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h743xx.s
)

target_include_directories(stm32h743xx_cmsis PUBLIC
	${STM32CUBEH7_DIR}/Drivers/CMSIS/Include
	${STM32CUBEH7_DIR}/Drivers/CMSIS/Device/ST
	${STM32CUBEH7_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
)

add_library(stm32h750xx_cmsis 
	${STM32CUBEH7_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h750xx.s
)

target_include_directories(stm32h750xx_cmsis PUBLIC
	${STM32CUBEH7_DIR}/Drivers/CMSIS/Include
	${STM32CUBEH7_DIR}/Drivers/CMSIS/Device/ST
	${STM32CUBEH7_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
)

add_library(stm32h753xx_cmsis 
	${STM32CUBEH7_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h753xx.s
)

target_include_directories(stm32h753xx_cmsis PUBLIC
	${STM32CUBEH7_DIR}/Drivers/CMSIS/Include
	${STM32CUBEH7_DIR}/Drivers/CMSIS/Device/ST
	${STM32CUBEH7_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
)

add_library(stm32h7xx_hal
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cec.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_comp.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cryp.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cryp_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dcmi.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dfsdm.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma2d.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_fdcan.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hash.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hash_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hcd.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hrtim.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2s.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2s_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_irda.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_iwdg.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_jpeg.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_lptim.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdios.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mmc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mmc_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_nand.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_nor.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_opamp.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_opamp_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_qspi.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sd.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sd_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sdram.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smartcard.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smartcard_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smbus.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spdifrx.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sram.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_swpmi.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_usart.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_wwdg.c
)

target_include_directories(stm32h7xx_hal PUBLIC
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
)

target_link_libraries(stm32h7xx_hal
	stm32h750xx_cmsis
)

add_library(stm32h7xx_ll
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_delayblock.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.c
)

target_include_directories(stm32h7xx_ll PUBLIC
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
)

target_link_libraries(stm32h7xx_ll
	stm32h750xx_cmsis
)
