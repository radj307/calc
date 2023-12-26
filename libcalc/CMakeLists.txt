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

target_link_libraries(libcalc PRIVATE shared)
