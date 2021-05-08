set(minicarp-srcs
        minicarp.cc
        whole_file_reader.cc
        ../common.cc)

add_library(minicarp ${minicarp-srcs})
#target_include_directories(minicarp PUBLIC /Users/schwifty/Repos/dfsman/carp/src)
target_link_libraries(minicarp pdlfs-common)
target_link_libraries(minicarp deltafs)

add_executable(minicarp-runner driver.cc)
target_link_libraries(minicarp-runner minicarp)
target_include_directories(minicarp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(minicarp-test minicarp-tests.cc)
target_link_libraries(minicarp-test minicarp)
target_include_directories(minicarp-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
# some ctest hook, not tested
add_test(minicarp minicarp)

install(TARGETS minicarp-runner RUNTIME DESTINATION bin)
