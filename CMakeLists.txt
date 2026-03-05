cmake_minimum_required(VERSION 3.20...3.31)

project(water_pouring_problem
        VERSION 1.4.2.0 # PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH, PROJECT_VERSION_TWEAK.
        DESCRIPTION "Water pouring puzzles with three jugs, tap and sink solver"
        HOMEPAGE_URL "https://github.com/gunchev/water_pouring_problem/"
        LANGUAGES C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_VERBOSE_MAKEFILE TRUE) # or "cd build; make VERBOSE=1"

# Build types filter
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
            "Choose the type of build, options are: None Debug DebugCov RelWithDebInfo Release MinSizeRel." FORCE)
endif ()

# SOVERSION is ignored if NO_SONAME property is set.
# "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}"
set(${PROJECT_NAME}_SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
message("${PROJECT_NAME} VERSION ${PROJECT_VERSION} SOVERSION is ${${PROJECT_NAME}_SOVERSION}")

# Build shared library by default
option(BUILD_SHARED_LIBS "Create shared libraries" ON)

# Add link path(s) to RPATH in shared libraries and executables. Saves setting LD_LIBRARY_PATH each time...
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)

# Standard install directories (handles lib vs lib64 automatically)
include(GNUInstallDirs)

# -fPIC on all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# Compiler warning flags
if (EXISTS "/etc/fedora-release")
    add_compile_options(-Wall -Wextra)
else ()
    add_compile_options(-Wall -Wextra -Werror)
endif ()

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CMakePushCheckState)

# https://gcc.gnu.org/onlinedocs/gcc/Template-Instantiation.html#Template-Instantiation

function(CheckAndAddFlag flag)
    # We cannot check for -Wno-foo, check for -Wfoo, see https://stackoverflow.com/a/38786117/1136400 for details.
    string(REGEX REPLACE "^-Wno-" "-W" checkedFlag ${flag})
    string(REGEX REPLACE "[^A-Za-z0-9]" "_" varName ${checkedFlag})
    if (NOT C_CXX${varName}_CHECKED)
        CHECK_CXX_COMPILER_FLAG(${checkedFlag} CXX_FLAG${varName}_SUPPORTED)
        CHECK_C_COMPILER_FLAG(${checkedFlag} C_FLAG${varName}_SUPPORTED)
        set(C_CXX${varName}_CHECKED YES CACHE INTERNAL "")
    endif ()
    if (CXX_FLAG${varName}_SUPPORTED)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${flag}>)
    endif ()
    if (C_FLAG${varName}_SUPPORTED)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:${flag}>)
    endif ()
endfunction()

cmake_push_check_state(RESET)
set(CMAKE_REQUIRED_QUIET ON)

foreach (I IN ITEMS -fmessage-length=0 -fstack-protector-all -pedantic
        -Weffc++ -Wsign-compare -Wpointer-sign -Wpedantic -Wformat=2 -Wformat-security
        -Wformat-overflow=2 -Wconversion -Wcast-align -Wcast-qual -Wctor-dtor-privacy
        -Wdisabled-optimization -Wlogical-op -Wmissing-declarations -Wnoexcept
        -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo
        -Wstrict-null-sentinel -Wswitch-default -Wundef -Wfloat-equal -Wstrict-overflow)
    CheckAndAddFlag(${I})
endforeach ()

cmake_pop_check_state()


# clang-tidy — configuration lives in .clang-tidy
# https://ortogonal.github.io/cmake-clang-tidy/
# https://www.kitware.com/static-checks-with-cmake-cdash-iwyu-clang-tidy-lwyu-cpplint-and-cppcheck/
set(CMAKE_CXX_CLANG_TIDY clang-tidy)
set(CMAKE_LINK_WHAT_YOU_USE TRUE)

# Linker flags
# http://wiki.gentoo.org/wiki/Project:Quality_Assurance/As-needed
# http://www.bnikolic.co.uk/blog/gnu-ld-as-needed.html ,--no-undefined,--no-allow-shlib-undefined
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-as-needed")
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--no-as-needed")

# Build target flags
# Debug, add_compile_options flags are combined with CMAKE_C_FLAGS_DEBUG and so on, don't duplicate...
set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb3 -DDEBUG -fno-inline") # -fno-common -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb3 -DDEBUG -fno-inline") # -fno-common -fno-omit-frame-pointer -fsanitize=address")
#set(CMAKE_SHARED_LINKER_FLAGS_DEBUG          "")
#set(CMAKE_MODULE_LINKER_FLAGS_DEBUG          "")
# Debug with profiling info
set(CMAKE_C_FLAGS_DEBUGCOV "-O0 -ggdb3 -DDEBUG -fno-inline -fprofile-arcs -ftest-coverage") # -fno-common -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUGCOV "-O0 -ggdb3 -DDEBUG -fno-inline -fprofile-arcs -ftest-coverage") # -fno-common -fno-omit-frame-pointer -fsanitize=address")
# Release with debug info and extra checks, https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html#index-fsanitize_003daddress-676
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -ggdb3 -D_FORTIFY_SOURCE=2") # -fno-common -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -ggdb3 -D_FORTIFY_SOURCE=2") # -fno-common -fno-omit-frame-pointer -fsanitize=address")
# Release
set(CMAKE_C_FLAGS_RELEASE "-O3 -D_FORTIFY_SOURCE=2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -D_FORTIFY_SOURCE=2 -DNDEBUG")
# Minimum size release
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -D_FORTIFY_SOURCE=2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -D_FORTIFY_SOURCE=2 -DNDEBUG")


find_package(fmt REQUIRED)

add_executable(water src/water.cpp src/utils.h)
target_include_directories(water PRIVATE src)
target_link_libraries(water PRIVATE fmt::fmt)
install(TARGETS water DESTINATION ${CMAKE_INSTALL_BINDIR})
