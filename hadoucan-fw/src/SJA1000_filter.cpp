#include "SJA1000_filter.hpp"

// Lawicel AB CAN232 compat mode
// Emulate SJA1000 filter
SJA1000_filter::SJA1000_filter()
{
	set_default();
}

SJA1000_filter::~SJA1000_filter()
{

}

void SJA1000_filter::set_default()
{
	enable      = true;
	mode        = FILTER_MODE::DUAL;
	accept_code = 0x00000000;
	accept_mask = 0xFFFFFFFF;
}

//True if filter permits this given STD frame
bool SJA1000_filter::is_std_msg_accepted(const uint32_t id, const bool rtr, const uint8_t datalen, const uint8_t db1, const uint8_t db2) const
{
	uint32_t temp_id = 0;
	
	temp_id |= uint32_t(id) << 21;
	temp_id |= (rtr) ? (1U << 20) : (0);
	temp_id |= uint32_t(db1) << 8;
	temp_id |= uint32_t(db2) << 0;

	return is_std_msg_accepted(temp_id, datalen);
}

//True if filter permits this given STD frame
//ID is SJA1000 packet format
//std id[10..0] in id[31..21], rtr in id[20], db1 in id[15..8], db2 in id[7..0]
bool SJA1000_filter::is_std_msg_accepted(const uint32_t id, const uint8_t datalen) const
{
	if(!enable)
	{
		return true;
	}

	bool filter_match = false;

	switch(mode)
	{
		case FILTER_MODE::DUAL:
		{
			// filter db1, db2 is dont't care
			const uint32_t cd = ( (((ACR1() & 0x000000F) << 4) | (ACR3() & 0x000000F) << 0) << 8) | 0x000000FF;
			uint32_t md = ( (((AMR1() & 0x000000F) << 4) | (AMR3() & 0x000000F) << 0) << 8) | 0x000000FF;

			//SJA1000 pg 46, in the event there is less data, ignore the data check
			switch(datalen)
			{
				case 0:
				{
					md |= 0x0000FFFF;
					break;
				}
				case 1:
				{
					md |= 0x000000FF;
					break;
				}
			}

			// filter ids
			// ac[31..20]
			const uint32_t c1 = (accept_code & 0xFFF00000) | 0x000F0000 | cd;
			const uint32_t m1 = (accept_mask & 0xFFF00000) | 0x000F0000 | md;
			// ac[15..4]
			const uint32_t c2 = ((accept_code >> 4) & 0x00000FFF) << 16 | 0x000F0000 | cd;
			const uint32_t m2 = ((accept_mask >> 4) & 0x00000FFF) << 16 | 0x000F0000 | md;
			
			// apply both filters
			const bool f1m = ((id & c1) | m1) == (c1 | m1);
			const bool f2m = ((id & c2) | m2) == (c2 | m2);

			filter_match = f1m || f2m;

			break;
		}
		case FILTER_MODE::SINGLE:
		{
			//SJA1000 pg 46, in the event there is less data, ignore the data check
			uint32_t temp_accept_mask = accept_mask;
			switch(datalen)
			{
				case 0:
				{
					temp_accept_mask |= 0x0000FFFF;
					break;
				}
				case 1:
				{
					temp_accept_mask |= 0x000000FF;
					break;
				}
			}

			filter_match = ((id & accept_code) | temp_accept_mask) == (accept_code | temp_accept_mask);
			break;
		}
		default:
		{
			filter_match = false;
			break;
		}
	}

	return filter_match;
}

//True if filter permits this given STD frame
bool SJA1000_filter::is_ext_msg_accepted(const uint32_t id, const bool rtr) const
{
	uint32_t temp_id = 0;
	
	temp_id |= uint32_t(id) << 3;
	temp_id |= (rtr) ? (1U << 2) : (0);

	return is_ext_msg_accepted(temp_id);
}

//True if filter permits this given ETD frame
//ID is SJA1000 packet format
//ext id[28..0] in id[31..03], rtr in id[02]
bool SJA1000_filter::is_ext_msg_accepted(const uint32_t id) const
{
	if(!enable)
	{
		return true;
	}

	bool filter_match = false;

	switch(mode)
	{
		case FILTER_MODE::DUAL:
		{
			const uint32_t c1 = (accept_code >> 16) & 0x0000FFFF;
			const uint32_t m1 = (accept_mask >> 16) & 0x0000FFFF;
			const uint32_t c2 = (accept_code >>  0) & 0x0000FFFF;
			const uint32_t m2 = (accept_mask >>  0) & 0x0000FFFF;

			const uint32_t id_28_13 = id >> 16;

			const bool f1m = ((id_28_13 & c1) | m1) == (c1 | m1);
			const bool f2m = ((id_28_13 & c2) | m2) == (c2 | m2);

			filter_match = f1m || f2m;
			break;
		}
		case FILTER_MODE::SINGLE:
		{
			filter_match = ((id & accept_code) | accept_mask) == (accept_code | accept_mask);
			break;
		}
		default:
		{
			filter_match = false;
			break;
		}
	}

	return filter_match;
}
