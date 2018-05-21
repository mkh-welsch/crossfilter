find_package (Boost COMPONENTS date_time REQUIRED)
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/3dparty/
    ${Boost_INCLUDE_DIRS}
    )
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -Wall -Wextra -Werror -g -pthread -std=c++14 -Ofast")
set (CMAKE_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fno-omit-frame-pointer -pthread")

#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Werror")

link_directories(${PROJECT_SOURCE_DIR}/3dparty/benchmark/)
file(GLOB BENCH_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
#AUX_SOURCE_DIRECTORY(. TEST_SRCS)
#Run through each source
foreach(benchSrc ${BENCH_SRCS})
	message("test=${benchSrc}")
        #Extract the filename without an extension (NAME_WE)
        get_filename_component(benchName ${benchSrc} NAME_WE)

        #Add compile target
        add_executable(${benchName} ${benchSrc})

        #link to Boost libraries AND your targets and dependencies
         target_link_libraries(${benchName} ${Boost_LIBRARIES})

        #I like to move testing binaries into a testBin directory
        set_target_properties(${benchName} PROPERTIES 
            RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}/benchBin)
#	set_property(TARGET ${testName} PROPERTY CXX_STANDARD 17)
        #Finally add it to test execution - 
        #Notice the WORKING_DIRECTORY and COMMAND
endforeach(benchSrc)
