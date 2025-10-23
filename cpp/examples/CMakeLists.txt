# Example programs

add_executable(basic_usage basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE orsf)

add_executable(validation_example validation_example.cpp)
target_link_libraries(validation_example PRIVATE orsf)

add_executable(conversion_example conversion_example.cpp)
target_link_libraries(conversion_example PRIVATE orsf)

add_executable(adapter_example adapter_example.cpp)
target_link_libraries(adapter_example PRIVATE orsf)
