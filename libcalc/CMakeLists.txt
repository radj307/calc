# calc/libcalc

file(GLOB_RECURSE HEADERS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"include/*.h*"
)
file(GLOB_RECURSE SRCS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"src/*.c*"
)

add_library(libcalc STATIC "${SRCS}" "cpp.hint")

set_property(TARGET libcalc PROPERTY CXX_STANDARD 20)
set_property(TARGET libcalc PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET libcalc PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET libcalc PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_compile_options(libcalc PRIVATE "${307lib_compiler_commandline}")

include(PrependEach)
PREPEND_EACH(HEADERS_ABS "${HEADERS}" "${CMAKE_CURRENT_SOURCE_DIR}/")

target_sources(libcalc PUBLIC "${HEADERS_ABS}")

target_include_directories(libcalc PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include;${CMAKE_CURRENT_SOURCE_DIR}/src")

if (CMAKE_MINOR_VERSION GREATER_EQUAL 24)
	# Use extraction timestamp
	# See https://cmake.org/cmake/help/latest/policy/CMP0135.html
	cmake_policy(SET CMP0135 NEW)
endif()

# Fetch Boost Multiprecision & Math libs
include(FetchContent)
# For arbitrary precision numbers
FetchContent_Declare(
	boostmp
	GIT_REPOSITORY	https://github.com/boostorg/multiprecision.git
	GIT_TAG			Boost_1_84_0
)
# Fixes a bug in boostmp: https://github.com/boostorg/multiprecision/issues/586
FetchContent_Declare(
	boostmath #< required for float->int conversions.
	GIT_REPOSITORY	https://github.com/boostorg/math.git
	GIT_TAG			boost-1.84.0
)
# For boostmp arbitrary base conversions
FetchContent_Declare(
	bisc
	GIT_REPOSITORY	https://github.com/radj307/bisc.git
	GIT_TAG			1.0.0
)

FetchContent_MakeAvailable(boostmp boostmath bisc)

target_link_libraries(libcalc PUBLIC shared Boost::multiprecision Boost::math bisc)
