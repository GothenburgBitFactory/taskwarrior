
# 
# Usage: rustc [OPTIONS] INPUT
# 
# Options:
#     -h --help           Display this message
#     --cfg SPEC          Configure the compilation environment
#     -L [KIND=]PATH      Add a directory to the library search path. The
#                         optional KIND can be one of dependency, crate, native,
#                         framework or all (the default).
#     -l [KIND=]NAME      Link the generated crate(s) to the specified native
#                         library NAME. The optional KIND can be one of static,
#                         dylib, or framework. If omitted, dylib is assumed.
#     --crate-type [bin|lib|rlib|dylib|cdylib|staticlib|metadata]
#                         Comma separated list of types of crates for the
#                         compiler to emit
#     --crate-name NAME   Specify the name of the crate being built
#     --emit [asm|llvm-bc|llvm-ir|obj|link|dep-info]
#                         Comma separated list of types of output for the
#                         compiler to emit
#     --print [crate-name|file-names|sysroot|cfg|target-list|target-cpus|target-features|relocation-models|code-models]
#                         Comma separated list of compiler information to print
#                         on stdout
#     -g                  Equivalent to -C debuginfo=2
#     -O                  Equivalent to -C opt-level=2
#     -o FILENAME         Write output to <filename>
#     --out-dir DIR       Write output to compiler-chosen filename in <dir>
#     --explain OPT       Provide a detailed explanation of an error message
#     --test              Build a test harness
#     --target TARGET     Target triple for which the code is compiled
#     -W --warn OPT       Set lint warnings
#     -A --allow OPT      Set lint allowed
#     -D --deny OPT       Set lint denied
#     -F --forbid OPT     Set lint forbidden
#     --cap-lints LEVEL   Set the most restrictive lint level. More restrictive
#                         lints are capped at this level
#     -C --codegen OPT[=VALUE]
#                         Set a codegen option
#     -V --version        Print version info and exit
#     -v --verbose        Use verbose output
# 
# Additional help:
#     -C help             Print codegen options
#     -W help             Print 'lint' options and default settings
#     -Z help             Print internal options for debugging rustc
#     --help -v           Print the full set of options rustc accepts
# 

# <TARGET> <TARGET_BASE> <OBJECT> <OBJECTS> <LINK_LIBRARIES> <FLAGS> <LINK_FLAGS> <SOURCE> <SOURCES>

include(CMakeLanguageInformation)

if(UNIX)
	set(CMAKE_Rust_OUTPUT_EXTENSION .o)
else()
	set(CMAKE_Rust_OUTPUT_EXTENSION .obj)
endif()

set(CMAKE_Rust_ECHO_ALL "echo \"TARGET: <TARGET> TARGET_BASE: <TARGET_BASE> ")
set(CMAKE_Rust_ECHO_ALL "${CMAKE_Rust_ECHO_ALL} OBJECT: <OBJECT> OBJECTS: <OBJECTS> OBJECT_DIR: <OBJECT_DIR> SOURCE: <SOURCE> SOURCES: <SOURCES> ")
set(CMAKE_Rust_ECHO_ALL "${CMAKE_Rust_ECHO_ALL} LINK_LIBRARIES: <LINK_LIBRARIES> FLAGS: <FLAGS> LINK_FLAGS: <LINK_FLAGS> \"")

if(NOT CMAKE_Rust_CREATE_SHARED_LIBRARY)
	set(CMAKE_Rust_CREATE_SHARED_LIBRARY
		"echo \"CMAKE_Rust_CREATE_SHARED_LIBRARY\""
		"${CMAKE_Rust_ECHO_ALL}"
		)
endif()

if(NOT CMAKE_Rust_CREATE_SHARED_MODULE)
	set(CMAKE_Rust_CREATE_SHARED_MODULE
		"echo \"CMAKE_Rust_CREATE_SHARED_MODULE\""
		"${CMAKE_Rust_ECHO_ALL}"
		)
endif()

if(NOT CMAKE_Rust_CREATE_STATIC_LIBRARY)
	set(CMAKE_Rust_CREATE_STATIC_LIBRARY
		"echo \"CMAKE_Rust_CREATE_STATIC_LIBRARY\""
		"${CMAKE_Rust_ECHO_ALL}"
		)
endif()

if(NOT CMAKE_Rust_COMPILE_OBJECT)
	set(CMAKE_Rust_COMPILE_OBJECT
		"echo \"CMAKE_Rust_COMPILE_OBJECT\""
		"${CMAKE_Rust_ECHO_ALL}"
		"${CMAKE_Rust_COMPILER} --emit obj <SOURCE> -o <OBJECT>")
endif()

if(NOT CMAKE_Rust_LINK_EXECUTABLE)
	set(CMAKE_Rust_LINK_EXECUTABLE
		"echo \"CMAKE_Rust_LINK_EXECUTABLE\""
		"${CMAKE_Rust_ECHO_ALL}"
		)
endif()

mark_as_advanced(
	CMAKE_Rust_FLAGS
	CMAKE_Rust_FLAGS_DEBUG
	CMAKE_Rust_FLAGS_MINSIZEREL
	CMAKE_Rust_FLAGS_RELEASE
	CMAKE_Rust_FLAGS_RELWITHDEBINFO)

set(CMAKE_Rust_INFORMATION_LOADED 1)

