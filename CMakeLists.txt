cmake_minimum_required(VERSION 3.20.0)  # RHEL 8

project(water_pouring_problem
    VERSION 1.4.2.0 # PROJECT_VERSION_MAJOR, PROJECT_VERSION_MINOR, PROJECT_VERSION_PATCH, and PROJECT_VERSION_TWEAK.
    DESCRIPTION "Water pouring puzzles with three jugs, tap and sink solver"
    HOMEPAGE_URL "https://github.com/gunchev/water_pouring_problem/"
    LANGUAGES C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_VERBOSE_MAKEFILE TRUE) # or "cd build; make VERBOSE=1"

# Build types filter
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Choose the type of build, options are: None Debug DebugCov RelWithDebInfo Release MinSizeRel." FORCE)
endif()

# SOVERSION is ignored if NO_SONAME property is set.
# "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}"
set(${PROJECT_NAME}_SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
message("${PROJECT_NAME} VERSION ${PROJECT_VERSION} SOVERSION is ${${PROJECT_NAME}_SOVERSION}")

# Build shared library by default
if(NOT BUILD_SHARED_LIBS)
    option(BUILD_SHARED_LIBS "Create shared libraries" ON)
endif()

# Add link path(s) to RPATH in shared libraries and executables. Saves setting LD_LIBRARY_PATH each time...
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)

# Library directory - lib or lib64?
# rpm: '%{?_cmake_lib_suffix64}' - https://src.fedoraproject.org/rpms/kid3/blob/rawhide/f/kid3.spec
# https://gitlab.kitware.com/cmake/cmake/-/merge_requests/2558
if (NOT LIB_SUFFIX)
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(LIB_SUFFIX "")
    else ()
        set(LIB_SUFFIX 64)
    endif ()
endif ()

# -fPIC on all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# Compiler/linker flags
if(EXISTS "/etc/fedora-release")
set(CMAKE_C_FLAGS   "-Wall -Wextra")
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
else()
set(CMAKE_C_FLAGS   "-Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")
endif()

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

# https://gcc.gnu.org/onlinedocs/gcc/Template-Instantiation.html#Template-Instantiation

set(CMAKE_REQUIRED_QUIET_OLD ${CMAKE_REQUIRED_QUIET})
set(CMAKE_REQUIRED_QUIET ON)
macro(CheckAndAddFlag flag)
    # We cannot check for -Wno-foo, check for -Wfoo, see https://stackoverflow.com/a/38786117/1136400 for details.
    string(REGEX REPLACE "^-Wno-" "-W" checkedFlag ${flag})
    string(REGEX REPLACE "[^A-Za-z0-9]" "_" varName ${checkedFlag})
    if(NOT C_CXX${varName}_CHECKED)
        CHECK_CXX_COMPILER_FLAG(${checkedFlag} CXX_FLAG${varName}_SUPPORTED)
        CHECK_C_COMPILER_FLAG(${checkedFlag} C_FLAG${varName}_SUPPORTED)
        set(C_CXX${varName}_CHECKED YES CACHE INTERNAL "")
    endif()
    if(CXX_FLAG${varName}_SUPPORTED)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    endif()
    if(C_FLAG${varName}_SUPPORTED)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}")
    endif()
    unset(varName)
    unset(checkedFlag)
endmacro()

foreach (I IN ITEMS -fmessage-length=0 -fstack-protector-all -pedantic
        -Weffc++ -Wsign-compare -Wpointer-sign -Wpedantic -Wformat=2 -Wformat-security
        -Wformat-overflow=2 -Wconversion -Wcast-align -Wcast-qual -Wctor-dtor-privacy
        -Wdisabled-optimization -Wlogical-op -Wmissing-declarations -Wnoexcept
        -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo
        -Wstrict-null-sentinel -Wswitch-default -Wundef -Wfloat-equal -Wstrict-overflow)
    CheckAndAddFlag(${I})
endforeach ()
set(CMAKE_REQUIRED_QUIET ${CMAKE_REQUIRED_QUIET_OLD})
unset(CMAKE_REQUIRED_QUIET_OLD)


# Linker flags
# http://wiki.gentoo.org/wiki/Project:Quality_Assurance/As-needed
# http://www.bnikolic.co.uk/blog/gnu-ld-as-needed.html ,--no-undefined,--no-allow-shlib-undefined
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-as-needed")
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--no-as-needed")

# Build target flags
# Debug, CMAKE_C_FLAGS is prepended to CMAKE_C_FLAGS_DEBUG and so on, don't duplicate...
set(CMAKE_C_FLAGS_DEBUG                      "-O0 -ggdb3 -DDEBUG -fno-inline") # -fno-common -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUG                    "-O0 -ggdb3 -DDEBUG -fno-inline") # -fno-common -fno-omit-frame-pointer -fsanitize=address")
#set(CMAKE_SHARED_LINKER_FLAGS_DEBUG          "")
#set(CMAKE_MODULE_LINKER_FLAGS_DEBUG          "")
# Debug with profiling info
set(CMAKE_C_FLAGS_DEBUGCOV                   "-O0 -ggdb3 -DDEBUG -fno-inline -fprofile-arcs -ftest-coverage") # -fno-common -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUGCOV                 "-O0 -ggdb3 -DDEBUG -fno-inline -fprofile-arcs -ftest-coverage") # -fno-common -fno-omit-frame-pointer -fsanitize=address")
# Release with debug info and extra checks, https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html#index-fsanitize_003daddress-676
set(CMAKE_C_FLAGS_RELWITHDEBINFO             "-O3 -ggdb3 -D_FORTIFY_SOURCE=2") # -fno-common -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO           "-O3 -ggdb3 -D_FORTIFY_SOURCE=2") # -fno-common -fno-omit-frame-pointer -fsanitize=address")
# Release
set(CMAKE_C_FLAGS_RELEASE                    "-O3 -ggdb3 -D_FORTIFY_SOURCE=2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE                  "-O3 -ggdb3 -D_FORTIFY_SOURCE=2 -DNDEBUG")
# Minimum size release
set(CMAKE_C_FLAGS_MINSIZEREL                 "-Os -ggdb3 -D_FORTIFY_SOURCE=2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL               "-Os -ggdb3 -D_FORTIFY_SOURCE=2 -DNDEBUG")



# Enable testing?
if(CPPUNIT_FOUND)
    enable_testing()
    include(CTest)
    find_program(CTEST_COVERAGE_COMMAND NAMES gcov)
    find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
endif()


# Threads library
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Find all third party packages
find_package(fmt REQUIRED)
include_directories(${FMT_INCLUDE_DIR})

# binaries
add_executable(water src/water.cpp)
target_link_libraries(water PRIVATE fmt::fmt) # Threads::Threads)
install(TARGETS water DESTINATION bin)
