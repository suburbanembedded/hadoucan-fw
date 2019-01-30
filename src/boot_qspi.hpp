#pragma once

class Boot_qspi
{

	Boot_qspi();

	bool init();

protected:
	QSPI_HandleTypeDef* m_qspi_handle;
};

class Boot_qspi_mmap
{

	Boot_qspi_mmap();

	bool init();

protected:

	bool config_mmap_read();
};

class Boot_qspi_indirect
{

	Boot_qspi_indirect();

	bool init();

	void read(uint8_t* ptr, uint32_t len, uint32_t timeout_ms);
	void write(uint8_t* ptr, uint32_t len, uint32_t timeout_ms);

protected:

	bool config_indirect_read();
	bool config_indirect_write();
};
