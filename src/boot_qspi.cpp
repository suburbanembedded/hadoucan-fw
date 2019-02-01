#include "boot_qspi.hpp"

bool Boot_qspi::init()
{

	*m_qspi_handle = QSPI_HandleTypeDef();

	m_qspi_handle->Instance = QUADSPI;

	m_qspi_handle->Init = QSPI_InitTypeDef();

	//AHB clock divider
	//AHB clock is 100MHz
	m_qspi_handle->Init.ClockPrescaler = 20;//5MHz - 200ns
	m_qspi_handle->Init.FifoThreshold = 32;//1-32, used for timeout in mmap mode
	m_qspi_handle->Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;//or QSPI_SAMPLE_SHIFTING_HALFCYCLE
	m_qspi_handle->Init.FlashSize = 23 ;
	m_qspi_handle->Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
	m_qspi_handle->Init.ClockMode = QSPI_CLOCK_MODE_0;//clk idle low
	m_qspi_handle->Init.FlashID = QSPI_FLASH_ID_2;
	m_qspi_handle->Init.DualFlash = QSPI_DUALFLASH_DISABLE;

	// m_qspi_handle->pTxBuffPtr
	// m_qspi_handle->TxXferSize
	// m_qspi_handle->TxXferCount

	// m_qspi_handle->pRxBuffPtr
	// m_qspi_handle->RxXferSize
	// m_qspi_handle->RxXferCount

	HAL_QSPI_Init(m_qspi_handle);

	return true;
}

bool Boot_qspi_mmap::config_mmap_read()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();
	cmd_cfg.Instruction//?
	cmd_cfg.Address//?
	cmd_cfg.AlternateBytes//0
	cmd_cfg.AddressSize//QSPI_ADDRESS_24_BITS
	cmd_cfg.AlternateBytesSize//QSPI_ALTERNATE_BYTES_8_BITS?
	cmd_cfg.DummyCycles//0 - 31
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;//QSPI_INSTRUCTION_4_LINE
	cmd_cfg.AddressMode = QSPI_ADDRESS_1_LINE;//later QSPI_ADDRESS_4_LINE
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_1_LINE;//later QSPI_ALTERNATE_BYTES_4_LINE
	cmd_cfg.DataMode = QSPI_DATA_1_LINE;//later, QSPI_DATA_4_LINE
	cmd_cfg.NbData//0 - FFFFFFFF
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;//might want QSPI_SIOO_INST_ONLY_FIRST_CMD, dep on flash

	QSPI_MemoryMappedTypeDef mm_cfg = QSPI_MemoryMappedTypeDef();
	mm_cfg.TimeOutPeriod = 0xFFFF;//TODO enable this, for lower power, in clk cycles
	mm_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;//TODO enable this, for lower power
	
	HAL_QSPI_MemoryMapped(m_qspi_handle, &cmd_cfg, &mm_cfg);

	return true;
}

bool Boot_qspi_indirect::config_indirect_read()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();
	cmd_cfg.Instruction//?
	cmd_cfg.Address//?
	cmd_cfg.AlternateBytes//0
	cmd_cfg.AddressSize//QSPI_ADDRESS_24_BITS
	cmd_cfg.AlternateBytesSize//QSPI_ALTERNATE_BYTES_8_BITS?
	cmd_cfg.DummyCycles//0 - 31
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;//QSPI_INSTRUCTION_4_LINE
	cmd_cfg.AddressMode = QSPI_ADDRESS_1_LINE;//later QSPI_ADDRESS_4_LINE
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_1_LINE;//later QSPI_ALTERNATE_BYTES_4_LINE
	cmd_cfg.DataMode = QSPI_DATA_1_LINE;//later, QSPI_DATA_4_LINE
	cmd_cfg.NbData//0 - FFFFFFFF
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;//might want QSPI_SIOO_INST_ONLY_FIRST_CMD, dep on flash

	//indirect mode
	//timeout in ticks
	HAL_QSPI_Command(m_qspi_handle, &cmd_cfg, 0xFFFF);
	HAL_QSPI_Transmit();
	HAL_QSPI_Receive();

	return true;
}
bool Boot_qspi_indirect::config_indirect_write()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();
	cmd_cfg.Instruction//?
	cmd_cfg.Address//?
	cmd_cfg.AlternateBytes//0
	cmd_cfg.AddressSize//QSPI_ADDRESS_24_BITS
	cmd_cfg.AlternateBytesSize//QSPI_ALTERNATE_BYTES_8_BITS?
	cmd_cfg.DummyCycles//0 - 31
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;//QSPI_INSTRUCTION_4_LINE
	cmd_cfg.AddressMode = QSPI_ADDRESS_1_LINE;//later QSPI_ADDRESS_4_LINE
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_1_LINE;//later QSPI_ALTERNATE_BYTES_4_LINE
	cmd_cfg.DataMode = QSPI_DATA_1_LINE;//later, QSPI_DATA_4_LINE
	cmd_cfg.NbData//0 - FFFFFFFF
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;//might want QSPI_SIOO_INST_ONLY_FIRST_CMD, dep on flash
	
	//indirect mode
	//timeout in ticks
	HAL_QSPI_Command(m_qspi_handle, &cmd_cfg, 10);

	return true;
}

extern "C"
{
	/*
	void HAL_QSPI_ErrorCallback(QSPI_HandleTypeDef *hqspi)
	{

	}
	void HAL_QSPI_AbortCpltCallback(QSPI_HandleTypeDef *hqspi)
	{

	}
	void HAL_QSPI_CmdCpltCallback(QSPI_HandleTypeDef *hqspi)
	{

	}
	void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi)
	{

	}
	void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *hqspi)
	{

	}
	void HAL_QSPI_RxHalfCpltCallback(QSPI_HandleTypeDef *hqspi)
	{

	}
	void HAL_QSPI_TxHalfCpltCallback(QSPI_HandleTypeDef *hqspi)
	{

	}
	void HAL_QSPI_FifoThresholdCallback(QSPI_HandleTypeDef *hqspi)
	{

	}
	void HAL_QSPI_StatusMatchCallback(QSPI_HandleTypeDef *hqspi)
	{

	}
	void HAL_QSPI_TimeOutCallback(QSPI_HandleTypeDef *hqspi)
	{

	}
	*/
}