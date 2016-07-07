cmake_minimum_required(VERSION 2.6.4)

include(FindPkgConfig)
set(PROJECT_NAME_STR lua-cassandra-driver)
set(PROJECT_LIB_NAME lua5.1-cassandra-driver)

# TODO: Figure out Mac OS X rpath
if(POLICY CMP0042)
  cmake_policy(SET CMP0042 OLD)
endif()

# Force OLD style of implicitly dereferencing variables
if(POLICY CMP0054)
  cmake_policy(SET CMP0054 OLD)
endif()

# Force OLD style of project versioning variables
if(POLICY CMP0048)
  cmake_policy(SET CMP0048 OLD)
endif()

# Title the project and define the versioning
project(${PROJECT_NAME_STR} C)

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

message(STATUS "Project version: ${PROJECT_VERSION_STRING}")

set(LUA_CASS_DRV_PROJECT_NAME ${PROJECT_NAME_STR})

# Assign specialized 32-bit compiler flags
if(32BIT)
  set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS FALSE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m32")
endif()

# Options
option(LUA_CASS_DRV_BUILD_SHARED "Build shared library" ON)
option(LUA_CASS_DRV_MULTICORE_COMPILATION "Enable multicore compilation" OFF)

# Determine which driver target should be used as a dependency
set(PROJECT_LIB_NAME_TARGET ${PROJECT_LIB_NAME})

# Ensure the driver is configured to build
if(NOT LUA_CASS_DRV_BUILD_SHARED)
  message(FATAL_ERROR "Driver is not Configured to Build: Ensure shared library is enabled")
endif()

# Ensure Windows platform is supported
if(WIN32)
  if(CMAKE_SYSTEM_VERSION GREATER 5.2 OR
     CMAKE_SYSTEM_VERSION EQUAL 5.2)
    add_definitions(-D_WIN32_WINNT=0x502)
  else()
    string(REPLACE "." "" WINDOWS_VERSION ${CMAKE_SYSTEM_VERSION})
    string(REGEX REPLACE "([0-9])" "0\\1" WINDOWS_VERSION ${WINDOWS_VERSION})
    message(FATAL_ERROR "Unable to build driver: Unsupported Windows platform 0x${WINDOWS_VERSION}")
  endif()
endif()

#---------------
# Dependencies
#---------------

# cassandra
pkg_check_modules(CASSANDRA REQUIRED cassandra)
set(LUA_CASS_DRV_INCLUDES ${LUA_CASS_DRV_INCLUDES} ${CASSANDRA_INCLUDE_DIRS})
set(LUA_CASS_DRV_LIB_DIRS ${LUA_CASS_DRV_LIB_DIRS} ${cassandra_LIBRARY_DIRS})
set(LUA_CASS_DRV_LIBS ${LUA_CASS_DRV_LIBS} ${CASSANDRA_LIBRARIES})

# lua5.1
find_package (Lua51)
if (LUA51_FOUND)
  set(LUA_CASS_DRV_INCLUDES ${LUA_CASS_DRV_INCLUDES} ${LUA_INCLUDE_DIR})
else()
  message(FATAL_ERROR "Lua5.1 not found")
endif()

# uuid
pkg_check_modules(UUID REQUIRED uuid)
set(LUA_CASS_DRV_INCLUDES ${LUA_CASS_DRV_INCLUDES} ${UUID_INCLUDE_DIRS})
set(LUA_CASS_DRV_LIB_DIRS ${LUA_CASS_DRV_LIB_DIRS} ${UUID_LIBRARY_DIRS})
set(LUA_CASS_DRV_LIBS ${LUA_CASS_DRV_LIBS} ${UUID_LIBRARIES})

#-------------------
# Compiler Flags
#-------------------

# Determine if all GNU extensions should be enabled
if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")
endif()

if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" OR
   "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  # CMAKE_C_COMPILER variables do not exist in 2.6.4 (min version)
  # Parse the -dumpversion argument into the variable not already set
  if("${CMAKE_C_COMPILER_VERSION}" STREQUAL "")
    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE CMAKE_C_COMPILER_VERSION)
  endif()
endif()

# Assign compiler specific flags
if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
  # Determine if multicore compilation should be enabled
  if(LUA_CASS_DRV_MULTICORE_COMPILATION)
    # Default multicore compilation with effective processors (see https://msdn.microsoft.com/en-us/library/bb385193.aspx)
    add_definitions("/MP")
  endif()

  # Remove/Replace linker flags in the event they are present
  string(REPLACE "/INCREMENTAL" "/INCREMENTAL:NO" CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
  string(REPLACE "/INCREMENTAL" "/INCREMENTAL:NO" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

  # Create specific linker flags
  set(WINDOWS_LINKER_FLAGS "/INCREMENTAL:NO /LTCG /NODEFAULTLIB:LIBCMT.LIB /NODEFAULTLIB:LIBCMTD.LIB")
  if(LUA_CASS_DRV_USE_STATIC_LIBS)
    set(PROJECT_C_LINKER_FLAGS "${WINDOWS_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${WINDOWS_LINKER_FLAGS}")
  endif()

  # On Visual C++ -pedantic flag is not used,
  # -fPIC is not used on Windows platform (all DLLs are
  # relocable), -Wall generates about 30k stupid warnings
  # that can hide useful ones.
  # Create specific warning disable compiler flags
  # TODO(mpenick): Fix these "possible loss of data" warnings
  add_definitions(/wd4244)
  add_definitions(/wd4267)
  add_definitions(/wd4800) # Performance warning due to automatic compiler casting from int to bool

  # Add preprocessor definitions for proper compilation
  add_definitions(-D_WIN32_WINNT=0x0501)      # Required for winsock (pre Windows XP wspiapi.h only)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)  # Remove warnings for not using safe functions (TODO: Fix codebase to be more secure for Visual Studio)
  add_definitions(-DNOMINMAX)                 # Does not define min/max macros

  # Create the project, example, and test flags
  set(PROJECT_C_FLAGS "${CMAKE_C_FLAGS} ${PROJECT_C_FLAGS} ${WARNING_COMPILER_FLAGS}")
  set(TEST_C_FLAGS "${PROJECT_C_FLAGS} ${WARNING_COMPILER_FLAGS}")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  # GCC specific compiler options
  # I disabled long-long warning because boost generates about 50 such warnings
  set(WARNING_COMPILER_FLAGS "-Wall -pedantic -Wextra -Wno-long-long -Wno-unused-parameter -Wno-variadic-macros")

  if(CMAKE_C_COMPILER_VERSION VERSION_EQUAL "4.8" OR
     CMAKE_C_COMPILER_VERSION VERSION_GREATER "4.8")
    set(WARNING_COMPILER_FLAGS "${WARNING_COMPILER_FLAGS} -Wno-unused-local-typedefs")
  endif()

  set(PROJECT_C_FLAGS "${CMAKE_C_FLAGS} ${PROJECT_C_FLAGS} ${WARNING_COMPILER_FLAGS} -Werror")
  set(TEST_C_FLAGS "${TEST_C_FLAGS} ${WARNING_COMPILER_FLAGS}")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  # Clang/Intel specific compiler options
  # I disabled long-long warning because boost generates about 50 such warnings
  set(WARNING_COMPILER_FLAGS "-Wall -pedantic -Wextra")
  set(WARNING_COMPILER_FLAGS "${WARNING_COMPILER_FLAGS} -Wno-variadic-macros -Wno-zero-length-array")
  set(WARNING_COMPILER_FLAGS "${WARNING_COMPILER_FLAGS} -Wno-unused-local-typedef -Wno-unknown-warning-option")

  set(PROJECT_C_FLAGS " ${CMAKE_C_FLAGS} ${PROJECT_C_FLAGS} ${WARNING_COMPILER_FLAGS} -Werror")
  set(TEST_C_FLAGS "${TEST_C_FLAGS} ${WARNING_COMPILER_FLAGS}")
else()
  message(FATAL_ERROR "Unsupported compiler: ${CMAKE_C_COMPILER_ID}")
endif()

#------------------------------
# lua-cassandra-driver
#------------------------------

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# Gather the header and source files
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.c)

# Create header and source groups (mainly for Visual Studio generator)
source_group("Source Files" FILES ${SRC_FILES})
source_group("Header Files" FILES ${API_HEADER_FILES} ${INC_FILES})
set(ALL_SOURCE_FILES ${SRC_FILES} ${API_HEADER_FILES} ${INC_FILES})

# Shorten the source file pathing for log messages
foreach(SRC_FILE ${SRC_FILES})
  string(REPLACE "${CMAKE_SOURCE_DIR}/" "" LOG_FILE_ ${SRC_FILE})
  set_source_files_properties(${SRC_FILE} PROPERTIES COMPILE_FLAGS -DLOG_FILE_=\\\"${LOG_FILE_}\\\")
endforeach()

# Create an object library for the driver (single build)
if(NOT CMAKE_VERSION VERSION_LESS "2.8.8")
  add_library(lua-wrapper OBJECT ${ALL_SOURCE_FILES})
  if(NOT WIN32)
    set_property(TARGET lua-wrapper PROPERTY COMPILE_FLAGS "${PROJECT_C_FLAGS} -fPIC")
  endif()

  # Build shared libraries
  set(LUA_CASS_DRV_BUILD_SHARED ON)
endif()

# Assign the include directories
include_directories(${LUA_CASS_DRV_INCLUDES})
link_directories(${LUA_CASS_DRV_LIB_DIRS})

# Determine if the dynamic/shared library should be built
if(CMAKE_VERSION VERSION_LESS "2.8.8")
  add_library(${PROJECT_LIB_NAME} SHARED ${ALL_SOURCE_FILES})
else()
  add_library(${PROJECT_LIB_NAME} SHARED $<TARGET_OBJECTS:lua-wrapper>)
endif()

target_link_libraries(${PROJECT_LIB_NAME} ${LUA_CASS_DRV_LIBS})
set_target_properties(${PROJECT_LIB_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_LIB_NAME})
set_target_properties(${PROJECT_LIB_NAME} PROPERTIES VERSION ${PROJECT_VERSION_STRING} SOVERSION ${PROJECT_VERSION_MAJOR})
set_target_properties(${PROJECT_LIB_NAME} PROPERTIES LINK_FLAGS "${PROJECT_C_LINKER_FLAGS}")
set_property(TARGET ${PROJECT_LIB_NAME}
    APPEND PROPERTY COMPILE_FLAGS "${PROJECT_C_FLAGS}")

# Allow libraries to be available for parent projects
if(LUA_CASS_DRV_IS_SUBPROJECT)
  set(LUA_CASS_DRV_LIBS ${LUA_CASS_DRV_LIBS} PARENT_SCOPE)
endif()

#-------------------------------------
# Directory installation information
#-------------------------------------

# Determine if the library directory needs to be determined
if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux" AND
      ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr" OR
       "${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local"))
    if(EXISTS "/etc/debian_version")
      set (CMAKE_INSTALL_LIBDIR "lib/${CMAKE_LIBRARY_ARCHITECTURE}")
    elseif(EXISTS "/etc/redhat-release" OR EXISTS "/etc/fedora-release" OR
           EXISTS "/etc/slackware-version" OR EXISTS "/etc/gentoo-release")
      if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set (CMAKE_INSTALL_LIBDIR "lib64")
      else()
        set (CMAKE_INSTALL_LIBDIR "lib")
      endif()
    else()
      set (CMAKE_INSTALL_LIBDIR "lib")
    endif()
  else()
    set (CMAKE_INSTALL_LIBDIR "lib")
  endif()
endif()

if(CMAKE_HOST_UNIX)
  set(LUA_CASS_DRV_LINK \$\(DESTDIR\)${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/lua/5.1/db/cassandra.so)
  add_custom_command(OUTPUT ${LUA_CASS_DRV_LINK}
    COMMAND mkdir -p \$\(DESTDIR\)${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/lua/5.1/db
    COMMAND ln -sf ../../../lib${PROJECT_LIB_NAME}.so ${LUA_CASS_DRV_LINK})
  
  add_custom_target(install_link DEPENDS ${LUA_CASS_DRV_LINK})
endif(CMAKE_HOST_UNIX)

#------------------
# Install targets
#------------------
install(TARGETS ${PROJECT_LIB_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
