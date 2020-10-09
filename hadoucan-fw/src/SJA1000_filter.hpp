#pragma once

#include <cstdint>

// Lawicel AB CAN232 compat mode
// Emulate SJA1000 filter
class SJA1000_filter
{
public:
	SJA1000_filter();
	~SJA1000_filter();

	enum class FILTER_MODE
	{
		DUAL   = 0,
		SINGLE = 1
	};

	void set_default();

	bool is_enabled() const
	{
		return enable;
	}

	FILTER_MODE get_filter_mode() const
	{
		return mode;
	}

	//True if filter permits this given STD frame
	bool is_std_msg_accepted(const uint32_t id, const bool rtr, const uint8_t db1, const uint8_t db2) const;

	//True if filter permits this given STD frame
	//ID is SJA1000 packet format
	//std id[10..0] in id[31..21], rtr in id[20], db1 in id[15..8], db2 in id[7..0]
	bool is_std_msg_accepted(const uint32_t id) const;

	//True if filter permits this given STD frame
	bool is_ext_msg_accepted(const uint32_t id, const bool rtr) const;

	//True if filter permits this given ETD frame
	//ID is SJA1000 packet format
	//ext id[28..0] in id[31..03], rtr in id[02]
	bool is_ext_msg_accepted(const uint32_t id) const;

	uint32_t ACR0() const
	{
		return (accept_code >> 24) & 0x000000FF;
	}
	uint32_t ACR1() const
	{
		return (accept_code >> 16) & 0x000000FF;
	}
	uint32_t ACR2() const
	{
		return (accept_code >> 8) & 0x000000FF;
	}
	uint32_t ACR3() const
	{
		return (accept_code >> 0) & 0x000000FF;
	}

	uint32_t AMR0() const
	{
		return (accept_mask >> 24) & 0x000000FF;
	}
	uint32_t AMR1() const
	{
		return (accept_mask >> 16) & 0x000000FF;
	}
	uint32_t AMR2() const
	{
		return (accept_mask >> 8) & 0x000000FF;
	}
	uint32_t AMR3() const
	{
		return (accept_mask >> 0) & 0x000000FF;
	}

	bool enable;

	FILTER_MODE mode;

	// ACR0[7..0] | ACR1[7..0] | ACR2[7..0] | ACR3[7..0]
	uint32_t accept_code;

	// bit 1 = don't care
	// AMR0[7..0] | AMR1[7..0] | AMR2[7..0] | AMR3[7..0]
	uint32_t accept_mask;

	//In single frame mode
	//For Std, ACR0[7..0], ACR1[7..0], ACR2[7..0], ACR3[7..0] maps to can[10..0, RTR, X, X, X, X, data[7..0], data[15..8]
	//For Ext, ACR0[7..0], ACR1[7..0], ACR2[7..0], ACR3[7..0] maps to can[29..0, RTR, X, X]

	//In dual frame mode
	//For Std,
	//        ACR0[7..0], ACR1[7..4] maps to can[10..0, RTR] <-- either filter can accept
	//        ACR1[3..0], ACR3[3..0] maps to data[7..0]
	//        ACR2[3..0], ACR3[7..4] maps to can[10..0, RTR] <-- either filter can accept
	//For Ext,
	//        ACR0[7..0], ACR1[7..0] maps to can[28..13] <-- either filter can accept
	//        ACR2[3..0], ACR3[7..0] maps to can[28..13] <-- either filter can accept

	//single filter helpers
	uint32_t get_std_id_code() const
	{
		return (accept_code & 0xFFE00000) >> 21;
	}
	uint32_t get_std_id_mask() const
	{
		return (accept_mask & 0xFFE00000) >> 21;
	}
	uint32_t get_std_rtr_code() const
	{
		return (accept_code & 0x00100000) >> 20;
	}
	uint32_t get_std_rtr_mask() const
	{
		return (accept_mask & 0x00100000) >> 20;
	}
	bool accept_std_rtr() const
	{
		return get_std_rtr_code() || get_std_rtr_mask();
	}

	uint32_t get_ext_id_code() const
	{
		return (accept_code & 0xFFFFFFF8) >> 3;
	}
	uint32_t get_ext_id_mask() const
	{
		return (accept_mask & 0xFFFFFFF8) >> 3;
	}
	uint32_t get_ext_rtr_code() const
	{
		return (accept_code & 0x00000004) >> 2;
	}
	uint32_t get_ext_rtr_mask() const
	{
		return (accept_mask & 0x00000004) >> 2;
	}
	bool accept_ext_rtr() const
	{
		return get_ext_rtr_code() || get_ext_rtr_mask();
	}

	//dual filter helpers
	uint32_t get_std_id1_code() const
	{
		return (accept_code & 0xFFE00000) >> 21;
	}
	uint32_t get_std_id1_mask() const
	{
		return (accept_mask & 0xFFE00000) >> 21;
	}
	uint32_t get_std_rtr1_code() const
	{
		return (ACR1() & 0x10) ? (1) : (0);
	}
	uint32_t get_std_rtr1_mask() const
	{
		return (AMR1() & 0x10) ? (1) : (0);
	}
	bool accept_std_rtr1() const
	{
		return get_std_rtr1_code() || get_std_rtr1_mask();
	}

	uint32_t get_std_id2_code() const
	{
		return (accept_code & 0x0000FFE0) >> 4;
	}
	uint32_t get_std_id2_mask() const
	{
		return (accept_mask & 0x0000FFE0) >> 4;
	}
	uint32_t get_std_rtr2_code() const
	{
		return (ACR3() & 0x10) ? (1) : (0);
	}
	uint32_t get_std_rtr2_mask() const
	{
		return (AMR3() & 0x10) ? (1) : (0);
	}
	bool accept_std_rtr2() const
	{
		return get_std_rtr2_code() || get_std_rtr2_mask();
	}

	uint32_t get_ext_id1_code() const
	{
		return ((accept_code & 0xFFFF0000) >> 3) | 0x00001FFF;
	}
	uint32_t get_ext_id1_mask() const
	{
		return ((accept_mask & 0xFFFF0000) >> 3) | 0x00001FFF;
	}
	uint32_t get_ext_rtr1_code() const
	{
		return 1;
	}
	uint32_t get_ext_rtr1_mask() const
	{
		return 1;
	}
	bool accept_ext_rtr1() const
	{
		return get_ext_rtr1_code() || get_ext_rtr1_mask();
	}

	uint32_t get_ext_id2_code() const
	{
		return ((accept_code & 0x0000FFFF) << 13) | 0x00001FFF;
	}
	uint32_t get_ext_id2_mask() const
	{
		return ((accept_mask & 0x0000FFFF) << 13) | 0x00001FFF;
	}
	uint32_t get_ext_rtr2_code() const
	{
		return 1;
	}
	uint32_t get_ext_rtr2_mask() const
	{
		return 1;
	}
	bool accept_ext_rtr2() const
	{
		return get_ext_rtr2_code() || get_ext_rtr2_mask();
	}

};


