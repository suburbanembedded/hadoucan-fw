#include "freertos_cpp_util/Task_static.hpp"

class Status_task : public Task_static<1024>
{
public:

	Status_task();

	void work() override;
protected:
	const char* state_to_str(const eTaskState state);
};