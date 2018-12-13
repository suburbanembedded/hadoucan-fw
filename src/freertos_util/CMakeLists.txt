add_library(freertos_util
	src/Queue_base.cpp
	src/Queue_static.cpp
	src/Task_base.cpp
	src/Task_heap.cpp
	src/Task_static.cpp

)


target_include_directories(freertos_util PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(freertos_util
	freertos_v9
)