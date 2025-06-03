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
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cordic.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cryp.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cryp_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dcmi.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dfsdm.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dfsdm_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma2d.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dsi.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dts.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_fdcan.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_fmac.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gfxmmu.c
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
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdios.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mmc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mmc_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_msp_template.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_nand.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_nor.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_opamp.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_opamp_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ospi.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_otfdec.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pssi.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_qspi.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ramecc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sd.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sd_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sdio.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sdram.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smartcard.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smartcard_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smbus.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smbus_ex.c
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
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_usart_ex.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_wwdg.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_adc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_bdma.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_comp.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_cordic.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_crc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_crs.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_dac.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_delayblock.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_dma2d.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_dma.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_exti.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmac.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_gpio.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_hrtim.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_i2c.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_lptim.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_lpuart.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_mdma.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_opamp.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_pwr.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_rcc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_rng.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_rtc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_spi.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_swpmi.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_tim.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usart.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.c
	${STM32CUBEH7_DIR}/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_utils.c
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

add_library(freertos_v10

	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/croutine.c
	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/list.c
	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/queue.c
	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/tasks.c
	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/timers.c

	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/port.c

	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
)

add_library(freertos_v10_heap1
	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_1.c
)

add_library(freertos_v10_heap2
	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_2.c
)

add_library(freertos_v10_heap3
	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_3.c
)

add_library(freertos_v10_heap4
	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
)

add_library(freertos_v10_heap5
	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_5.c
)

add_library(freertos_v10_cmsis
	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c
)

target_include_directories(freertos_v10 PUBLIC
	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1
)

target_link_libraries(freertos_v10_heap1
	freertos_v10
)

target_link_libraries(freertos_v10_heap2
	freertos_v10
)

target_link_libraries(freertos_v10_heap3
	freertos_v10
)

target_link_libraries(freertos_v10_heap4
	freertos_v10
)

target_link_libraries(freertos_v10_heap5
	freertos_v10
)

target_link_libraries(freertos_v10_cmsis
	freertos_v10
	stm32h750xx_cmsis
)

target_include_directories(freertos_v10_cmsis PUBLIC
	${STM32CUBEH7_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS
)
