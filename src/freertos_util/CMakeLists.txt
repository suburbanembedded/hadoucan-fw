add_library(freertos_util
	src/Queue_base.cpp
	src/Queue_static.cpp
	src/Queue_static_pod.cpp
	
	src/Critical_section_isr.cpp
	src/Critical_section.cpp
	
	src/Task_base.cpp
	src/Task_heap.cpp
	src/Task_static.cpp

	src/Mutex_base.cpp
	src/Mutex_static.cpp
	src/Mutex_heap.cpp

	src/object_pool/Object_pool.cpp
	src/object_pool/Object_pool_node.cpp
)


target_include_directories(freertos_util PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(freertos_util
	freertos_v9
)