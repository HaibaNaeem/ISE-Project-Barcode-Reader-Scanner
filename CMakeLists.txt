cmake_minimum_required(VERSION 3.20)
project(BarcodeDesktopApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

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

# ---- Test programs: every tests/test_*.cpp becomes its own exe ----
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/tests/test_*.cpp")
foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} PRIVATE services)
endforeach()

# ---- Main GUI application (built once src/main.cpp exists) ----
if(EXISTS "${CMAKE_SOURCE_DIR}/src/main.cpp")
    file(GLOB GUI_SOURCES CONFIGURE_DEPENDS
         "${CMAKE_SOURCE_DIR}/src/gui/*.cpp"
         "${CMAKE_SOURCE_DIR}/src/gui/*.h")
    add_executable(BarcodeDesktopApp WIN32 src/main.cpp ${GUI_SOURCES})
    target_include_directories(BarcodeDesktopApp PRIVATE "${CMAKE_SOURCE_DIR}/src/gui")
    target_link_libraries(BarcodeDesktopApp PRIVATE services Qt6::Widgets)
endif()