#pragma once

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_qspi.h"

class W25Q16JV
{
public:
	static constexpr uint8_t MFG_ID = 0xEF;
	static constexpr uint16_t JEDEC_ID = 0x4015;

	static constexpr size_t PAGE_LEN = 256;

	static constexpr size_t SECTOR_LEN = 4*1024;

	static constexpr size_t BLOCK1_LEN = 32*1024;
	static constexpr size_t BLOCK2_LEN = 64*1024;

	static constexpr size_t PAGE_COUNT   = 8192;
	static constexpr size_t SECTOR_COUNT = 512;
	static constexpr size_t BLOCK1_COUNT = 64;
	static constexpr size_t BLOCK2_COUNT = 32;

	static constexpr size_t RESET_ASSERT_US = 1;
	static constexpr size_t RESET_DELAY_US = 30;

	static constexpr size_t STATUS_PRGM_TIME_TYP_MS = 10;
	static constexpr size_t STATUS_PRGM_TIME_MAX_MS = 15;

	static constexpr size_t PAGE_PRGM_TIME_TYP_US = 400;
	static constexpr size_t PAGE_PRGM_TIME_MAX_US = 3000;

	static constexpr size_t BLOCK1_ERASE_TIME_TYP_MS = 120;
	static constexpr size_t BLOCK1_ERASE_TIME_MAX_MS = 1600;

	static constexpr size_t BLOCK2_ERASE_TIME_TYP_MS = 150;
	static constexpr size_t BLOCK2_ERASE_TIME_MAX_MS = 2000;

	static constexpr size_t CHIP_ERASE_TIME_TYP_MS = 5000;
	static constexpr size_t CHIP_ERASE_TIME_MAX_MS = 25000;

	static constexpr uint32_t SECURITY_REG_1_ADDR = 0x001000;
	static constexpr uint32_t SECURITY_REG_2_ADDR = 0x002000;
	static constexpr uint32_t SECURITY_REG_3_ADDR = 0x003000;

	class STATUS_REG_1
	{
	public:
		bool get_SRP()  const;//b7
		bool get_SEC()  const;//b6
		bool get_TB()   const;//b5
		bool get_BP2()  const;//b4
		bool get_BP1()  const;//b3
		bool get_BP0()  const;//b2
		bool get_WEL()  const;//b1
		bool get_BUSY() const;//b0
	protected:
		uint8_t reg;
	};

	class STATUS_REG_2
	{
	public:
		bool get_SUS()  const;//b7
		bool get_CMP()  const;//b6
		bool get_LB3()  const;//b5
		bool get_LB2()  const;//b4
		bool get_LB1()  const;//b3
		bool get_RES2() const;//b2
		bool get_QE()   const;//b1
		bool get_SRL()  const;//b0
	protected:
		uint8_t reg;
	};

	class STATUS_REG_3
	{
	public:
		bool get_RES7() const;//b7
		bool get_DRV1() const;//b6
		bool get_DRV2() const;//b5
		bool get_RES4() const;//b4
		bool get_RES3() const;//b3
		bool get_WPS()  const;//b2
		bool get_RES1() const;//b1
		bool get_RES0() const;//b0
	protected:
		uint8_t reg;
	};

	//SPI CMD
	enum class STD_CMD : uint8_t
	{
		WRITE_ENABLE  = 0x06,
		WRITE_ENABLE_VSR  = 0x50,
		WRITE_DISABLE = 0x04,

		RELEASE_POWER_DOWN = 0xAB,
		MFG_DEV_ID = 0x90,
		JEDEC_ID = 0x9F,
		UNIQUE_ID = 0x4B,

		READ_DATA = 0x03,//MAX clk fR = 50 MHz
		FAST_READ = 0x0B,//MAX clk FR = 104 MHz in [2V7, 3V0], 133 MHz in [3V0, 3V6]

		PAGE_PRGM = 0x02,

		SECTOR_ERASE = 0x20,
		BLOCK_32K_ERASE = 0x52,
		BLOCK_64K_ERASE = 0xD8,
		CHIP_ERASE = 0xC7,

		READ_STATUS_1  = 0x05,
		WRITE_STATUS_1 = 0x01,
		READ_STATUS_2  = 0x35,
		WRITE_STATUS_2 = 0x31,
		READ_STATUS_3  = 0x15,
		WRITE_STATUS_3 = 0x11,

		POWER_DOWN = 0xB9,

		ENABLE_RESET = 0x66,
		RESET_DEVICE = 0x99
	};

	//Dual/Quad SPI CMD
	enum class DQ_CMD
	{

	};

	static QSPI_CommandTypeDef get_write_enable_cmd();
	static QSPI_CommandTypeDef get_volatile_write_enable_cmd();
	static QSPI_CommandTypeDef get_write_disable_cmd();
	static QSPI_CommandTypeDef get_read_status_reg1_cmd();
	static QSPI_CommandTypeDef get_write_status_reg1_cmd();
	static QSPI_CommandTypeDef get_read_data_cmd(const uint32_t addr, const size_t len);
	static bool get_page_prgm_cmd(const uint32_t addr, const size_t len, QSPI_CommandTypeDef* const cmd_cfg);
	static bool get_sector_erase_cmd(const uint32_t sector_num, QSPI_CommandTypeDef* const cmd_cfg);
	static bool get_block64_erase_cmd(const uint32_t block64_num, QSPI_CommandTypeDef* const cmd_cfg);
	static QSPI_CommandTypeDef get_chip_erase_cmd();
	static QSPI_CommandTypeDef get_power_down_cmd();
	static QSPI_CommandTypeDef get_release_power_down_cmd();
	static QSPI_CommandTypeDef get_read_mfg_dev_id_cmd();
	static QSPI_CommandTypeDef get_unique_id_cmd();
};

class Boot_qspi
{

	Boot_qspi();

	bool init();

protected:
	QSPI_HandleTypeDef* m_qspi_handle;
};

class QSPI_iface : public Boot_qspi
{
public:

protected:

};

class Boot_qspi_mmap : public Boot_qspi
{

	Boot_qspi_mmap();

	bool init();

protected:

	bool config_mmap_read();
};

class Boot_qspi_indirect : public Boot_qspi
{

	Boot_qspi_indirect();

	bool init();

	void read(uint8_t* ptr, uint32_t len, uint32_t timeout_ms);
	void write(uint8_t* ptr, uint32_t len, uint32_t timeout_ms);

protected:

	bool config_indirect_read();
	bool config_indirect_write();
};
