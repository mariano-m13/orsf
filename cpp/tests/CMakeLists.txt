# Test executable
add_executable(orsf_tests
    test_main.cpp
    test_core.cpp
    test_validator.cpp
    test_utils.cpp
    test_mapping.cpp
    test_adapter.cpp
)

target_link_libraries(orsf_tests PRIVATE
    orsf
    Catch2::Catch2WithMain
)

# Add tests to CTest
include(CTest)
include(Catch)
catch_discover_tests(orsf_tests)

# Copy test data files if they exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/data)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()
