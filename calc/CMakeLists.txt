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

# RESOURCES
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/rc")
## Make copyright notice string
include(CopyrightMaker)
MAKE_COPYRIGHT_NOTICE(_calc_copyright "radj307 and StormChaos" "2023")
## Make windows resource file
if (WIN32)
	include(ResourceMaker)

	MAKE_STRINGRC_VERSIONINFO(
		_calc_stringrc_versioninfo
		"${calc_VERSION}"
		"${_calc_copyright}"
		"radj307 and StormChaos"
		"calc"
		"CLI calculator."
	)

	MAKE_RESOURCE("${CMAKE_CURRENT_BINARY_DIR}/rc/calc.rc" "${_calc_stringrc_versioninfo}")
endif()
MAKE_VERSION_HEADER("${CMAKE_CURRENT_BINARY_DIR}/rc/version.h" calc "${calc_VERSION_EXTENDED}")
MAKE_COPYRIGHT_HEADER_FROM_NOTICE("${CMAKE_CURRENT_BINARY_DIR}/rc/copyright.h" calc "${_calc_copyright}")

file(GLOB RESOURCES
	CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_BINARY_DIR}/rc/*"
)

add_executable(calc "${SRCS}" "${RESOURCES}")

set_property(TARGET calc PROPERTY CXX_STANDARD 20)
set_property(TARGET calc PROPERTY CXX_STANDARD_REQUIRED ON)

target_compile_options(calc PRIVATE "${307lib_compiler_commandline}")

target_include_directories(calc PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/rc")

target_sources(calc PUBLIC "${HEADERS}")

target_link_libraries(calc PRIVATE libcalc TermAPI filelib strlib)
