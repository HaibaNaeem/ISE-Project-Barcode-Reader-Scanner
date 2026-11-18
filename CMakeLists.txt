cmake_minimum_required(VERSION 3.20)
project(BarcodeDesktopApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

enable_testing()   # lets `ctest` run the tests

find_package(Qt6 CONFIG REQUIRED COMPONENTS Widgets)
find_package(OpenCV REQUIRED)
find_package(ZXing CONFIG REQUIRED)
find_package(zint CONFIG REQUIRED)

# ---- Services library (generator, decoder, camera, validator) ----
file(GLOB SERVICE_SOURCES CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/src/services/*.cpp"
     "${CMAKE_SOURCE_DIR}/src/services/*.h")

add_library(services STATIC ${SERVICE_SOURCES})
target_include_directories(services PUBLIC "${CMAKE_SOURCE_DIR}/src/services")
target_link_libraries(services PUBLIC ${OpenCV_LIBS} ZXing::ZXing zint::zint)

# ---- Core library (NEW, Increment 2): all of src/core/*.cpp, no Qt ----
# New .cpp files in src/core are picked up automatically (re-run `cmake --preset default` after adding one).
file(GLOB CORE_SOURCES CONFIGURE_DEPENDS
     "${CMAKE_SOURCE_DIR}/src/core/*.cpp"
     "${CMAKE_SOURCE_DIR}/src/core/*.h")

add_library(core STATIC ${CORE_SOURCES})
target_include_directories(core PUBLIC "${CMAKE_SOURCE_DIR}/src/core")
target_link_libraries(core PUBLIC services)   # core may use services, never the other way round

# ---- Test programs: every tests/test_*.cpp becomes its own exe ----
# Every test except test_camera (needs a real camera and a person) is also run by CTest.
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/test_*.cpp")
foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_include_directories(${test_name} PRIVATE "${CMAKE_SOURCE_DIR}/tests")   # TestHelpers.h
    target_compile_definitions(${test_name} PRIVATE
        SAMPLE_DATA_DIR="${CMAKE_SOURCE_DIR}/sample_data")                         # for test_integration
    target_link_libraries(${test_name} PRIVATE core)
    if(NOT test_name STREQUAL "test_camera")
        # Run from the repo root: the Increment 1 tests use relative paths like output/ and tests/test_images/
        add_test(NAME ${test_name} COMMAND ${test_name} WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
    endif()
endforeach()
# test_decoder reads barcode images written by test_generator
if(TEST test_decoder AND TEST test_generator)
    set_tests_properties(test_decoder PROPERTIES DEPENDS test_generator)
endif()

# ---- Main GUI application (built once src/main.cpp exists) ----
if(EXISTS "${CMAKE_SOURCE_DIR}/src/main.cpp")
    file(GLOB GUI_SOURCES CONFIGURE_DEPENDS
         "${CMAKE_SOURCE_DIR}/src/gui/*.cpp"
         "${CMAKE_SOURCE_DIR}/src/gui/*.h")
    add_executable(BarcodeDesktopApp WIN32 src/main.cpp ${GUI_SOURCES})
    target_include_directories(BarcodeDesktopApp PRIVATE "${CMAKE_SOURCE_DIR}/src/gui")
    target_link_libraries(BarcodeDesktopApp PRIVATE core services Qt6::Widgets)
    # Copy sample_data/ next to the .exe after every build (DataPaths fills data/ from it on first run)
    add_custom_command(TARGET BarcodeDesktopApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/sample_data" "$<TARGET_FILE_DIR:BarcodeDesktopApp>/sample_data"
        COMMENT "Copying sample_data next to the exe")
    # Copy Qt plugins (platforms\qwindows.dll etc.) next to the exe after every build
    add_custom_command(TARGET BarcodeDesktopApp POST_BUILD
        COMMAND "${VCPKG_INSTALLED_DIR}/x64-windows/tools/Qt6/bin/windeployqt.exe"
                $<IF:$<CONFIG:Debug>,--debug,--release>
                --no-translations --no-compiler-runtime
                "$<TARGET_FILE:BarcodeDesktopApp>"
        COMMENT "Deploying Qt plugins")
endif()