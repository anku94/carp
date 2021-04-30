set(minicarp-srcs
        minicarp.cc
        )

#add_executable(minicarp driver.cc ${minicarp-srcs})
add_library(minicarp ${minicarp-srcs})
target_link_libraries(minicarp pdlfs-common)
target_include_directories(minicarp PRIVATE ..)

add_executable(minicarp-runner driver.cc)
target_link_libraries(minicarp-runner minicarp)

add_executable(minicarp-test minicarp-tests.cc)
target_link_libraries(minicarp-test minicarp)
# some ctest hook, not tested
add_test(minicarp minicarp)

install(TARGETS minicarp-runner RUNTIME DESTINATION bin)
