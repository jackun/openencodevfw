cmake_minimum_required(VERSION 2.8.5)
macro(getenv_path VAR)
   set(ENV_${VAR} $ENV{${VAR}})
   # replace won't work if var is blank
   if (ENV_${VAR})
     string( REGEX REPLACE "\\\\" "/" ENV_${VAR} ${ENV_${VAR}} )
   endif ()
endmacro(getenv_path)

SET(TargetName "OpenEncodeVFW")
project(${TargetName})

if(CMAKE_SIZEOF_VOID_P MATCHES "8")
	SET(DIR_POSTFIX "64")
	SET(LIB_POSTFIX "64")
	SET(LIB_DIR_POSTFIX "_64")
else()
	SET(DIR_POSTFIX "32")
	SET(LIB_POSTFIX "")
	SET(LIB_DIR_POSTFIX "")
endif()

set (PLUGIN_VERSION_MAJOR "0")
set (PLUGIN_VERSION_MINOR "0")
set (PLUGIN_VERSION_PATCH "1")
set (PLUGIN_VERSION "${PLUGIN_VERSION_MAJOR}.${PLUGIN_VERSION_MINOR}.${PLUGIN_VERSION_PATCH}")

set(HDRS
	OpenEncodeVFW/bitstream.h
	OpenEncodeVFW/bs.h
	OpenEncodeVFW/clconvert.h
	OpenEncodeVFW/log.h
	OpenEncodeVFW/OpenEncodeVFW.h
	OpenEncodeVFW/OvEncodeTypedef.h
	OpenEncodeVFW/perf.h
	OpenEncodeVFW/resource.h
	OpenEncodeVFW/stdafx.h
	OpenEncodeVFW/targetver.h
)

set(SRCS
	OpenEncodeVFW/bitstream.cpp
	OpenEncodeVFW/clconvert.cpp
	OpenEncodeVFW/compress.cpp
	OpenEncodeVFW/configure.cpp
	OpenEncodeVFW/decompress.cpp
	OpenEncodeVFW/device.cpp
	OpenEncodeVFW/dllmain.cpp
	OpenEncodeVFW/drvproc.cpp
	OpenEncodeVFW/log.cpp
	OpenEncodeVFW/OpenEncodeVFW.cpp
	OpenEncodeVFW/perf.cpp
	OpenEncodeVFW/stdafx.cpp
)

SOURCE_GROUP("Header Files" FILES ${HDRS})
SOURCE_GROUP("Source Files" FILES ${SRCS})	

### Find and setup AMD APP SDK
getenv_path(AMDAPPSDKROOT)
SET (BUILD_AMDAPPSDK_INCLUDE "${ENV_AMDAPPSDKROOT}/include" CACHE PATH "AMD APP SDK include dir")
SET (BUILD_AMDAPPSDK_LIB "${ENV_AMDAPPSDKROOT}/lib/x86${LIB_DIR_POSTFIX}" CACHE PATH "AMD APP SDK library dir")
include_directories(${BUILD_AMDAPPSDK_INCLUDE})
link_directories(${BUILD_AMDAPPSDK_LIB})
LIST(APPEND LIBS OpenCL OpenVideo${LIB_POSTFIX} WinMM)

file(GLOB RCS ./OpenEncodeVFW/*.rc)
file(GLOB DEF ./OpenEncodeVFW/*.def)

add_definitions(-D_UNICODE -DUNICODE) #Make VS use unicode instead of multi-byte
add_definitions(-DOPENENCODEVFW_EXPORTS)
add_library(${TargetName} SHARED ${SRCS} ${HDRS} ${RCS} ${DEF})
target_link_libraries(${TargetName} ${LIBS})
#set_target_properties(${TargetName} PROPERTIES DEBUG_POSTFIX _d)
set_target_properties(${TargetName} PROPERTIES DEBUG_POSTFIX "")
set_target_properties(${TargetName} PROPERTIES VERSION ${PLUGIN_VERSION} SOVERSION ${PLUGIN_VERSION_MAJOR})

# post-build copy for win32
IF(WIN32 AND NOT MINGW)
    add_custom_command( TARGET ${TargetName} PRE_BUILD
        COMMAND if not exist \"${CMAKE_SOURCE_DIR}/OpenEncode${DIR_POSTFIX}\" mkdir \"${CMAKE_SOURCE_DIR}/OpenEncode${DIR_POSTFIX}\" )
    add_custom_command( TARGET ${TargetName} POST_BUILD
        COMMAND copy \"$(TargetPath)\" \"${CMAKE_SOURCE_DIR}/OpenEncode${DIR_POSTFIX}\" )
ENDIF(WIN32 AND NOT MINGW)

# Left over from another CMakeList
# set the executable output path for UNIX and MinGW instead of post-build copy
IF(MINGW)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/OpenEncode${DIR_POSTFIX})
ENDIF(MINGW)

#-DCMAKE_INSTALL_PREFIX=some/where
#IF(WIN32)
#    install(TARGETS ${TargetName}
#        RUNTIME DESTINATION .
#        CONFIGURATIONS Release RelWithDebInfo MinSizeRel Debug) #All
#ENDIF(WIN32)
