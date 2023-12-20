# calc/calc

file(GLOB_RECURSE HEADERS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"*.h*"
)
file(GLOB_RECURSE SRCS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"*.c*"
)

add_executable(calc "${SRCS}")

set_property(TARGET calc PROPERTY CXX_STANDARD 20)
set_property(TARGET calc PROPERTY CXX_STANDARD_REQUIRED ON)

target_compile_options(calc PRIVATE "${307lib_compiler_commandline}")

target_sources(calc PRIVATE "${HEADERS}")

target_link_libraries(calc PRIVATE TermAPI filelib strlib)
