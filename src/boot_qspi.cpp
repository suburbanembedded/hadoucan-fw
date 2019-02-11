#include "boot_qspi.hpp"

#include "common_util/Byte_util.hpp"

#include <array>

QSPI_CommandTypeDef W25Q16JV::get_write_enable_cmd()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg.Instruction = uint32_t(STD_CMD::WRITE_ENABLE);
	cmd_cfg.Address = 0;
	cmd_cfg.AlternateBytes = 0;
	cmd_cfg.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg.DummyCycles = 0;
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg.AddressMode = QSPI_ADDRESS_NONE;
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg.DataMode = QSPI_DATA_NONE;
	cmd_cfg.NbData = 0;
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return cmd_cfg;
}

QSPI_CommandTypeDef W25Q16JV::get_volatile_write_enable_cmd()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg.Instruction = uint32_t(STD_CMD::WRITE_ENABLE_VSR);
	cmd_cfg.Address = 0;
	cmd_cfg.AlternateBytes = 0;
	cmd_cfg.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg.DummyCycles = 0;
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg.AddressMode = QSPI_ADDRESS_NONE;
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg.DataMode = QSPI_DATA_NONE;
	cmd_cfg.NbData = 0;
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return cmd_cfg;
}

QSPI_CommandTypeDef W25Q16JV::get_write_disable_cmd()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg.Instruction = uint32_t(STD_CMD::WRITE_DISABLE);
	cmd_cfg.Address = 0;
	cmd_cfg.AlternateBytes = 0;
	cmd_cfg.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg.DummyCycles = 0;
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg.AddressMode = QSPI_ADDRESS_NONE;
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg.DataMode = QSPI_DATA_NONE;
	cmd_cfg.NbData = 0;
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return cmd_cfg;
}

QSPI_CommandTypeDef W25Q16JV::get_read_status_reg1_cmd()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg.Instruction = uint32_t(STD_CMD::READ_STATUS_1);
	cmd_cfg.Address = 0;
	cmd_cfg.AlternateBytes = 0;
	cmd_cfg.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg.DummyCycles = 0;
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg.AddressMode = QSPI_ADDRESS_NONE;
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg.DataMode = QSPI_DATA_1_LINE;
	cmd_cfg.NbData = 1;
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return cmd_cfg;
}

QSPI_CommandTypeDef W25Q16JV::get_write_status_reg1_cmd()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg.Instruction = uint32_t(STD_CMD::WRITE_STATUS_1);
	cmd_cfg.Address = 0;
	cmd_cfg.AlternateBytes = 0;
	cmd_cfg.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg.DummyCycles = 0;
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg.AddressMode = QSPI_ADDRESS_NONE;
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg.DataMode = QSPI_DATA_1_LINE;
	cmd_cfg.NbData = 1;
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return cmd_cfg;
}

QSPI_CommandTypeDef W25Q16JV::get_read_status_reg2_cmd()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg.Instruction = uint32_t(STD_CMD::READ_STATUS_2);
	cmd_cfg.Address = 0;
	cmd_cfg.AlternateBytes = 0;
	cmd_cfg.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg.DummyCycles = 0;
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg.AddressMode = QSPI_ADDRESS_NONE;
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg.DataMode = QSPI_DATA_1_LINE;
	cmd_cfg.NbData = 1;
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return cmd_cfg;
}
QSPI_CommandTypeDef W25Q16JV::get_write_status_reg2_cmd()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg.Instruction = uint32_t(STD_CMD::WRITE_STATUS_2);
	cmd_cfg.Address = 0;
	cmd_cfg.AlternateBytes = 0;
	cmd_cfg.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg.DummyCycles = 0;
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg.AddressMode = QSPI_ADDRESS_NONE;
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg.DataMode = QSPI_DATA_1_LINE;
	cmd_cfg.NbData = 1;
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return cmd_cfg;	
}
QSPI_CommandTypeDef W25Q16JV::get_read_status_reg3_cmd()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg.Instruction = uint32_t(STD_CMD::READ_STATUS_3);
	cmd_cfg.Address = 0;
	cmd_cfg.AlternateBytes = 0;
	cmd_cfg.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg.DummyCycles = 0;
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg.AddressMode = QSPI_ADDRESS_NONE;
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg.DataMode = QSPI_DATA_1_LINE;
	cmd_cfg.NbData = 1;
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return cmd_cfg;
}
QSPI_CommandTypeDef W25Q16JV::get_write_status_reg3_cmd()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg.Instruction = uint32_t(STD_CMD::WRITE_STATUS_3);
	cmd_cfg.Address = 0;
	cmd_cfg.AlternateBytes = 0;
	cmd_cfg.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg.DummyCycles = 0;
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg.AddressMode = QSPI_ADDRESS_NONE;
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg.DataMode = QSPI_DATA_1_LINE;
	cmd_cfg.NbData = 1;
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return cmd_cfg;
}

QSPI_CommandTypeDef W25Q16JV::get_read_data_cmd(const uint32_t addr, const size_t len)
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg.Instruction = uint32_t(STD_CMD::READ_DATA);
	cmd_cfg.Address = addr;
	cmd_cfg.AlternateBytes = 0;
	cmd_cfg.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg.DummyCycles = 0;
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg.AddressMode = QSPI_ADDRESS_1_LINE;
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg.DataMode = QSPI_DATA_1_LINE;
	cmd_cfg.NbData = len;
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return cmd_cfg;
}

bool W25Q16JV::get_page_prgm_cmd(const uint32_t addr, const size_t len, QSPI_CommandTypeDef* const cmd_cfg)
{
	if(len > 256)
	{
		return false;
	}

	*cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg->Instruction = uint32_t(STD_CMD::PAGE_PRGM);
	cmd_cfg->Address = addr;
	cmd_cfg->AlternateBytes = 0;
	cmd_cfg->AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg->AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg->DummyCycles = 0;
	cmd_cfg->InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg->AddressMode = QSPI_ADDRESS_1_LINE;
	cmd_cfg->AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg->DataMode = QSPI_DATA_1_LINE;
	cmd_cfg->NbData = len;
	cmd_cfg->DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg->DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg->SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return true;
}

bool W25Q16JV::get_sector_erase_cmd(const uint32_t sector_num, QSPI_CommandTypeDef* const cmd_cfg)
{
	if(sector_num > SECTOR_COUNT)
	{
		return false;
	}

	*cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg->Instruction = uint32_t(STD_CMD::SECTOR_ERASE);
	cmd_cfg->Address = sector_num;
	cmd_cfg->AlternateBytes = 0;
	cmd_cfg->AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg->AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg->DummyCycles = 0;
	cmd_cfg->InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg->AddressMode = QSPI_ADDRESS_1_LINE;
	cmd_cfg->AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg->DataMode = QSPI_DATA_NONE;
	cmd_cfg->NbData = 0;
	cmd_cfg->DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg->DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg->SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return true;
}

bool W25Q16JV::get_block64_erase_cmd(const uint32_t block64_num, QSPI_CommandTypeDef* const cmd_cfg)
{
	if(block64_num > BLOCK1_COUNT)
	{
		return false;
	}

	*cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg->Instruction = uint32_t(STD_CMD::BLOCK_64K_ERASE);
	cmd_cfg->Address = block64_num;
	cmd_cfg->AlternateBytes = 0;
	cmd_cfg->AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg->AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg->DummyCycles = 0;
	cmd_cfg->InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg->AddressMode = QSPI_ADDRESS_1_LINE;
	cmd_cfg->AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg->DataMode = QSPI_DATA_NONE;
	cmd_cfg->NbData = 0;
	cmd_cfg->DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg->DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg->SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return true;
}

QSPI_CommandTypeDef W25Q16JV::get_chip_erase_cmd()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg.Instruction = uint32_t(STD_CMD::CHIP_ERASE);
	cmd_cfg.Address = 0;
	cmd_cfg.AlternateBytes = 0;
	cmd_cfg.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg.DummyCycles = 0;
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg.AddressMode = QSPI_ADDRESS_NONE;
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg.DataMode = QSPI_DATA_NONE;
	cmd_cfg.NbData = 0;
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return cmd_cfg;
}

QSPI_CommandTypeDef W25Q16JV::get_power_down_cmd()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg.Instruction = uint32_t(STD_CMD::POWER_DOWN);
	cmd_cfg.Address = 0;
	cmd_cfg.AlternateBytes = 0;
	cmd_cfg.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg.DummyCycles = 0;
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg.AddressMode = QSPI_ADDRESS_NONE;
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg.DataMode = QSPI_DATA_NONE;
	cmd_cfg.NbData = 0;
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return cmd_cfg;
}

QSPI_CommandTypeDef W25Q16JV::get_release_power_down_cmd()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg.Instruction = uint32_t(STD_CMD::RELEASE_POWER_DOWN);
	cmd_cfg.Address = 0;
	cmd_cfg.AlternateBytes = 0;
	cmd_cfg.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg.DummyCycles = 0;
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg.AddressMode = QSPI_ADDRESS_NONE;
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg.DataMode = QSPI_DATA_NONE;
	cmd_cfg.NbData = 0;
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return cmd_cfg;
}

QSPI_CommandTypeDef W25Q16JV::get_read_jdec_id_cmd()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg.Instruction = uint32_t(STD_CMD::JEDEC_ID);
	cmd_cfg.Address = 0;
	cmd_cfg.AlternateBytes = 0;
	cmd_cfg.AddressSize = QSPI_ADDRESS_24_BITS;
	cmd_cfg.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg.DummyCycles = 0;
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg.AddressMode = QSPI_ADDRESS_NONE;
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg.DataMode = QSPI_DATA_1_LINE;
	cmd_cfg.NbData = 3;
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return cmd_cfg;
}

QSPI_CommandTypeDef W25Q16JV::get_unique_id_cmd()
{
	QSPI_CommandTypeDef cmd_cfg = QSPI_CommandTypeDef();

	cmd_cfg.Instruction = uint32_t(STD_CMD::UNIQUE_ID);
	cmd_cfg.Address = 0;
	cmd_cfg.AlternateBytes = 0;
	cmd_cfg.AddressSize = QSPI_ADDRESS_32_BITS;//use address as 4 dummy bytes
	cmd_cfg.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	cmd_cfg.DummyCycles = 0;
	cmd_cfg.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	cmd_cfg.AddressMode = QSPI_ADDRESS_1_LINE;
	cmd_cfg.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	cmd_cfg.DataMode = QSPI_DATA_1_LINE;
	cmd_cfg.NbData = 8;
	cmd_cfg.DdrMode = QSPI_DDR_MODE_DISABLE;
	cmd_cfg.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	cmd_cfg.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	return cmd_cfg;
}

bool W25Q16JV::get_jdec_id(uint8_t* const out_mfg_id, uint16_t* const out_part_id)
{
	QSPI_CommandTypeDef cmd = W25Q16JV::get_read_jdec_id_cmd();
	std::array<uint8_t, 3> flash_jdec_id;
	flash_jdec_id.fill(0);

	HAL_StatusTypeDef ret = HAL_QSPI_Command(m_qspi_handle, &cmd, 1000);
	if(ret != HAL_OK)
	{
		return false;
	}

	ret = HAL_QSPI_Receive(m_qspi_handle, flash_jdec_id.data(), 1000);
	if(ret != HAL_OK)
	{
		return false;
	}

	*out_mfg_id = flash_jdec_id[0];
	*out_part_id = Byte_util::make_u16(flash_jdec_id[1], flash_jdec_id[2]);

	return true;
}

bool W25Q16JV::get_unique_id(uint64_t* const out_unique_id)
{
	QSPI_CommandTypeDef cmd = W25Q16JV::get_unique_id_cmd();
	std::array<uint8_t, 8> flash_unique_id;
	flash_unique_id.fill(0);

	HAL_StatusTypeDef ret = HAL_QSPI_Command(m_qspi_handle, &cmd, 1000);
	if(ret != HAL_OK)
	{
		return false;
	}

	ret = HAL_QSPI_Receive(m_qspi_handle, flash_unique_id.data(), 1000);
	if(ret != HAL_OK)
	{
		return false;
	}

	*out_unique_id = Byte_util::make_u64(
		flash_unique_id[0], flash_unique_id[1], flash_unique_id[2], flash_unique_id[3],
		flash_unique_id[4], flash_unique_id[5], flash_unique_id[6], flash_unique_id[7]
	);

	return true;
}

bool W25Q16JV::read(const uint32_t addr, const size_t len, uint8_t* const out_data)
{
	QSPI_CommandTypeDef cmd = get_read_data_cmd(addr, len);
	HAL_StatusTypeDef ret = HAL_QSPI_Command(m_qspi_handle, &cmd, 1000);
	if(ret != HAL_OK)
	{
		return false;
	}

	ret = HAL_QSPI_Receive(m_qspi_handle, out_data, 1000);
	if(ret != HAL_OK)
	{
		return false;
	}

	return true;
}

bool W25Q16JV::write_page(const uint32_t addr, const size_t len, const uint8_t* data)
{
	if(len > PAGE_LEN)
	{
		return false;
	}

	//TODO: volatile write enable

	QSPI_CommandTypeDef cmd;
	if(!get_page_prgm_cmd(addr, len, &cmd))
	{
		return false;
	}

	HAL_StatusTypeDef ret = HAL_QSPI_Command(m_qspi_handle, &cmd, 1000);
	if(ret != HAL_OK)
	{
		return false;
	}

	ret = HAL_QSPI_Transmit(m_qspi_handle, const_cast<uint8_t*>(data), 1000);
	if(ret != HAL_OK)
	{
		return false;
	}

	//TODO: poll for finished status

	return true;
}

bool W25Q16JV::write(const uint32_t addr, const size_t len, const uint8_t* data)
{
	const uint32_t page_addr = addr & (~PAGE_LEN);

	uint32_t current_addr = addr;
	size_t num_written = 0;
	
	//first unaligned page
	if(page_addr != addr)
	{
		const size_t num_to_write = (page_addr + PAGE_LEN) - addr;

		if(!write_page(current_addr, num_to_write, data + num_written))
		{
			return false;
		}

		current_addr += num_to_write;
		num_written += num_to_write;
	}

	//aligned pages
	while((num_written + PAGE_LEN) < len)
	{
		if(!write_page(current_addr, PAGE_LEN, data + num_written))
		{
			return false;
		}

		current_addr += PAGE_LEN;
		num_written += PAGE_LEN;
	}

	//last unaligned page
	if(num_written != len)
	{
		const size_t num_to_write = len - num_written;

		if(!write_page(current_addr, num_to_write, data + num_written))
		{
			return false;
		}

		current_addr += num_to_write;
		num_written += num_to_write;
	}

	return true;
}

bool W25Q16JV::init()
{

	*m_qspi_handle = QSPI_HandleTypeDef();

	m_qspi_handle->Instance = QUADSPI;

	m_qspi_handle->Init = QSPI_InitTypeDef();

	//AHB clock divider
	//AHB clock is 100MHz
	// m_qspi_handle->Init.ClockPrescaler = 20;//5MHz  - 200ns
	m_qspi_handle->Init.ClockPrescaler = 100;//1MHz - 1000ns
	m_qspi_handle->Init.FifoThreshold = 1;//1-32, used for timeout in mmap mode
	m_qspi_handle->Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;//or QSPI_SAMPLE_SHIFTING_HALFCYCLE
	m_qspi_handle->Init.FlashSize = 24;
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

	HAL_StatusTypeDef ret = HAL_QSPI_Init(m_qspi_handle);
	if(ret != HAL_OK)
	{
		return false;
	}

	return true;
}
/*
bool Boot_qspi_mmap::config_mmap_read()
{
	QSPI_CommandTypeDef cmd_cfg = W25Q16JV::get_read_data_cmd(0, 0);

	QSPI_MemoryMappedTypeDef mm_cfg = QSPI_MemoryMappedTypeDef();
	mm_cfg.TimeOutPeriod = 0xFFFF;//TODO enable this, for lower power, in clk cycles
	mm_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;//TODO enable this, for lower power
	
	HAL_QSPI_MemoryMapped(m_qspi_handle, &cmd_cfg, &mm_cfg);

	return true;
}
*/
/*
bool Boot_qspi_indirect::config_indirect_read()
{
	QSPI_CommandTypeDef cmd_cfg = W25Q16JV::get_read_data_cmd(0, 0);

	//indirect mode
	//timeout in ticks
	HAL_QSPI_Command(this->m_qspi_handle, &cmd_cfg, 0xFFFF);
	//HAL_QSPI_Transmit();
	//HAL_QSPI_Receive();

	return true;
}
*/
/*
bool Boot_qspi_indirect::config_indirect_write()
{
	QSPI_CommandTypeDef cmd_cfg;
	if(!W25Q16JV::get_page_prgm_cmd(0, 0, &cmd_cfg))
	{
		return false;
	}

	//indirect mode
	//timeout in ticks
	HAL_QSPI_Command(this->m_qspi_handle, &cmd_cfg, 10);

	return true;
}
*/

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