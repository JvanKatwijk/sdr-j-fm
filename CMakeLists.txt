cmake_minimum_required( VERSION 2.8.11 )
set (objectName fmreceiver-3.15)
set (CMAKE_CXX_FLAGS "${CMAKE_XCC_FLAGS} -Wall -std=c++14 -flto")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")

set(CMAKE_AUTORCC ON)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "/usr/local/bin" CACHE PATH "default install path" FORCE )
endif()
#set (CMAKE_INSTALL_PREFIX /usr/local/bin)
if(MINGW)
    add_definitions ( -municode)
#else (MINGW)
#       add_definitions (-DSSE_AVAILABLE)
endif()

IF(EXISTS (".git") AND IS_DIRECTORY (".git"))
   execute_process (
      COMMAND git rev-parse --short HEAD
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_COMMIT_HASH
      OUTPUT_STRIP_TRAILING_WHITESPACE
   )
endif ()

if(GIT_COMMIT_HASH)
   add_definitions("-DGITHASH=\"${GIT_COMMIT_HASH}\"")
else ()
   add_definitions ("-DGITHASH=\"       \"")
endif()

########################################################################
# select the release build type by default to get optimization flags
########################################################################
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

### make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

######################################################################
#

######################################################################
#       R E A D  T H I S
#####################################################################
#	adjust to your likings
#	if you want to use the SSE support on your machine, comment
#	set(NO_SSE_SUPPORT true)
#
option(AIRSPY OFF)       
option(RTLSDR OFF)       
option(SDRPLAY_V3 OFF)
option(HACKRF OFF)
option(LIMESDR OFF) 
option(RTL_TCP OFF) 
option(PLUTO OFF) 


########################################################################
	find_package (PkgConfig)
        find_package (LSBId)

        find_package (Qt5Core REQUIRED)
        find_package (Qt5Widgets REQUIRED)
        find_package (Qt5Network REQUIRED)
        find_package (Qt5Xml REQUIRED)
        find_package (Qt5Multimedia REQUIRED)
        include_directories (
          ${Qt5Network_INCLUDE_DIRS}
          ${Qt5Xml_INCLUDE_DIRS}
          ${Qt5Multimedia_INCLUDE_DIRS}
        )

	find_package(Qwt)
        if (NOT QWT_FOUND)
           message(FATAL_ERROR "please install qwt library")
        endif ()
        include_directories ( ${QWT_INCLUDE_DIRS})
        list(APPEND extraLibs ${QWT_LIBRARIES})

	find_library (DYNAMIC_LOAD dl)
	if(NOT(DYNAMIC_LOAD))
	   message(FATAL_ERROR "please install -ldl")
	else(NOT(DYNAMIC_LOAD))
	   set(extraLibs ${DYNAMIC_LOAD})
	endif(NOT(DYNAMIC_LOAD))
#

	include (FindPackageHandleStandardArgs)

	Find_Package (PkgConfig)

	pkg_check_modules (SAMPLERATE samplerate REQUIRED)
	link_directories (${SAMPLERATE})

	find_library (PORTAUDIO portaudio)
	if (NOT(PORTAUDIO))
	   message(FATAL_ERROR "please install portaudio V19")
	else(NOT(PORTAUDIO))
	   set (extraLibs  ${extraLibs} ${PORTAUDIO})
	endif(NOT(PORTAUDIO))

	SET (CMAKE_FIND_LIBRARY_SUFFIXES .so .a)

	find_library (USB1 usb-1.0 HINTS /usr/lib)
	if (NOT(USB1))
	   message (FATAL_ERROR "please install libusb-1.0")
	else(NOT(USB1))
	   set(extraLibs ${extraLibs} ${USB1})
	endif(NOT(USB1))

	find_library (SNDFILE sndfile)
	if (NOT(SNDFILE))
	   message (FATAL_ERROR "please install libsndfile")
	else (NOT(SNDFILE))
	   set (extraLibs ${extraLibs} ${SNDFILE})
	endif (NOT(SNDFILE))

	find_library (PTHREADS pthread)
	if (NOT(PTHREADS))
	   message (FATAL_ERROR "please install libpthread")
	else (NOT(PTHREADS))
	   set (extraLibs ${extraLibs} ${PTHREADS})
	endif (NOT(PTHREADS))
#######################################################################

	include_directories (
	   ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	   ./
	   ./includes
	   ./includes/output
	   ./includes/various
	   ./includes/fm
	   ./includes/rds
	   ./includes/scopes-qwt6
	   ./src
	   ./src/output
	   ./src/various
	   ./src/fm
	   ./src/rds
	   ./src/various
	   ./devices
	   ./devices/filereader
	)

	set ( ${objectName}_HDRS
             ./includes/popup-keypad.h 
             ./includes/fm-constants.h 
	     ./includes/various/program-list.h 
             ./includes/various/keyboardfilter.h 
	     ./includes/various/squelchClass.h 
	     ./includes/various/oscillator.h 
	     ./includes/various/ringbuffer.h 
	     ./includes/various/pllC.h 
	     ./includes/various/sincos.h 
	     ./includes/various/newconverter.h 
	     ./includes/various/fft-complex.h
	     ./includes/various/fir-filters.h 
	     ./includes/various/fft-filters.h 
	     ./includes/various/iir-filters.h 
	     ./includes/various/Xtan2.h 
	     ./includes/various/themechoser.h 
	     ./includes/various/agc.h  
	     ./includes/various/costas.h  
	     ./includes/various/shaping_filter.h  
	     ./includes/output/audiosink.h 
	     ./includes/scopes-qwt6/iqdisplay.h 
	     ./includes/scopes-qwt6/scope.h 
	     ./includes/scopes-qwt6/spectrogramdata.h 
	     ./includes/scopes-qwt6/hs-scope.h 
	     ./includes/scopes-qwt6/ls-scope.h 
	     ./includes/fm/fm-demodulator.h 
	     ./includes/fm/pilot-recover.h 
	     ./includes/fm/stereo-separation.h 
	     ./includes/fm/fm-processor.h 
	     ./includes/rds/rds-decoder.h 
	     ./includes/rds/rds-decoder-1.h 
	     ./includes/rds/rds-decoder-2.h 
	     ./includes/rds/rds-decoder-3.h 
	     ./includes/rds/rds-blocksynchronizer.h 
	     ./includes/rds/rds-group.h 
	     ./includes/rds/rds-groupdecoder.h  
	     ./devices/device-handler.h 
	     ./devices/deviceselect.h
	     ./devices/filereader/filereader.h 
	     ./devices/filereader/filehulp.h
	)

	set ( ${objectName}_SRCS
	      ./main.cpp
	      ./radio.cpp 
	      ./src/popup-keypad.cpp 
	      ./src/various/program-list.cpp 
	      ./src/various/keyboardfilter.cpp 
	      ./src/various/squelchClass.cpp 
#	      ./src/various/fft.cpp 
	      ./src/various/oscillator.cpp 
	      ./src/various/pllC.cpp 
	      ./src/various/sincos.cpp 
	      ./src/various/newconverter.cpp 
	      ./src/various/fft-complex.cpp 
	      ./src/various/fir-filters.cpp 
	      ./src/various/fft-filters.cpp 
	      ./src/various/iir-filters.cpp 
	      ./src/various/Xtan2.cpp 
	      ./src/various/themechoser.cpp 
	      ./src/various/shaping_filter.cpp
	      ./src/output/audiosink.cpp 
	      ./src/scopes-qwt6/iqdisplay.cpp 
	      ./src/scopes-qwt6/scope.cpp 
	      ./src/scopes-qwt6/hs-scope.cpp 
	      ./src/scopes-qwt6/ls-scope.cpp 
	      ./src/fm/fm-demodulator.cpp 
	      ./src/fm/pilot-recover.cpp 
	      ./src/fm/stereo-separation.cpp 
	      ./src/fm/fm-processor.cpp 
	      ./src/rds/rds-decoder.cpp 
	      ./src/rds/rds-decoder-1.cpp 
	      ./src/rds/rds-decoder-2.cpp 
	      ./src/rds/rds-decoder-3.cpp 
	      ./src/rds/rds-blocksynchronizer.cpp 
	      ./src/rds/rds-group.cpp 
	      ./src/rds/rds-groupdecoder.cpp 
	      ./devices/device-handler.cpp
	      ./devices/deviceselect.cpp
	      ./devices/filereader/filereader.cpp 
	      ./devices/filereader/filehulp.cpp
	)

	set ( ${objectName}_UIS 
	      ./forms/radio.ui 
	      ./forms/configwidget.ui
	      ./devices/filereader/filereader-widget.ui
	)

#####################################################################
#
	if (PMSDR)
	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/pmsdr/pmsdr-widget.ui 
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/pmsdr/pmsdr.h
	        ./devices/pmsdr/pa-reader.h
	        ./devices/pmsdr/pmsdr-usb.h
	   )

	   include_directories (
	     ./devices/pmsdr
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./devices/pmsdr/pmsdr.h
	        ./devices/pmsdr/pmsdr-comm.h
	        ./devices/pmsdr/pmsdr-usb.h
	        ./devices/pmsdr/pa-reader.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/pmsdr/pmsdr.cpp
	        ./devices/pmsdr/pmsdr-comm.cpp
	        ./devices/pmsdr/pmsdr-usb.cpp
	        ./devices/pmsdr/pa-reader.cpp
	   )

	   add_definitions (-DHAVE_PMSDR)
	endif (PMSDR)
	
	if (SDRPLAY)
	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/sdrplay-handler/sdrplay-widget.ui 
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/sdrplay-handler/sdrplay-handler.h
	        ./devices/sdrplay-handler/sdrplayselect.h
	   )

	   include_directories (
	     ./devices/sdrplay-handler
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./devices/sdrplay-handler/sdrplay-handler.h
	        ./devices/sdrplay-handler/sdrplayselect.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/sdrplay-handler/sdrplay-handler.cpp
	        ./devices/sdrplay-handler/sdrplayselect.cpp
	   )

	   add_definitions (-DHAVE_SDRPLAY)
	endif (SDRPLAY)

	if (SDRPLAY_V3)

	   set (${objectName}_UIS
                ${${objectName}_UIS}
                ./devices/sdrplay-handler-v3/sdrplay-widget-v3.ui
           )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/sdrplay-handler-v3/sdrplay-handler-v3.h
                ./devices/sdrplay-handler-v3/Rsp-device.h
           )

	   include_directories ( ./devices/sdrplay-handler-v3)
	   include_directories ( ./devices/sdrplay-handler-v3/include)

	   set ($(objectName)_HDRS
                  ${${objectName}_HDRS}
                  ./devices/sdrplay-handler-v3/sdrplay-handler-v3.h
	          ./devices/sdrplay-handler-v3/sdrplay-commands.h
	          ./devices/sdrplay-handler-v3/Rsp-device.h
                  ./devices/sdrplay-handler-v3/RspI-handler.h
                  ./devices/sdrplay-handler-v3/Rsp1A-handler.h
                  ./devices/sdrplay-handler-v3/RspDuo-handler.h
                  ./devices/sdrplay-handler-v3/RspDx-handler.h
                  ./devices/sdrplay-handler-v3/RspII-handler.h)
	

	   set (${objectName}_SRCS
	           ${${objectName}_SRCS}
                  ./devices/sdrplay-handler-v3/sdrplay-handler-v3.cpp
	          ./devices/sdrplay-handler-v3/Rsp-device.cpp
	          ./devices/sdrplay-handler-v3/RspI-handler.cpp
	          ./devices/sdrplay-handler-v3/Rsp1A-handler.cpp
	          ./devices/sdrplay-handler-v3/RspDuo-handler.cpp
	          ./devices/sdrplay-handler-v3/RspDx-handler.cpp
	          ./devices/sdrplay-handler-v3/RspII-handler.cpp)

	   add_definitions (-DHAVE_SDRPLAY_V3)
	endif (SDRPLAY_V3)

	if (AIRSPY)

	   include_directories (
	     ./devices/airspy
	     ./devices/airspy/libairspy
	   )
	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/airspy/airspy-widget.ui 
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/airspy/airspy-handler.h
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ./devices/airspy/airspy-handler.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/airspy/airspy-handler.cpp
	   )

	   add_definitions (-DHAVE_AIRSPY)
	endif (AIRSPY)

	if (RTLSDR)

	   include_directories (
	        ./devices/rtlsdr-handler/
	   )

	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/rtlsdr-handler/dabstick-widget.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/rtlsdr-handler/dongleselect.h
	        ./devices/rtlsdr-handler/rtlsdr-handler.h
	   )

	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	        ./devices/rtlsdr-handler/rtlsdr-handler.h 
	        ./devices/rtlsdr-handler/dongleselect.h
	        ./devices/rtlsdr-handler/rtl-sdr.h
	        ./devices/rtlsdr-handler/rtl-sdr_export.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/rtlsdr-handler/rtlsdr-handler.cpp
	        ./devices/rtlsdr-handler/dongleselect.cpp
	   )

	   add_definitions (-DHAVE_DABSTICK)
	endif()

	if (HACKRF)

	   include_directories ( ./devices/hackrf-handler
	                         ./devices/hackrf-handler/libhackrf)

	   set (${objectName}_UIS
                ${${objectName}_UIS}
                ./devices/hackrf-handler/hackrf-widget.ui
           )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/hackrf-handler/hackrf-handler.h
           )

	   set ($(objectName)_HDRS
                  ${${objectName}_HDRS}
                  ./devices/hackrf-handler/hackrf-handler.h)

	   set (${objectName}_SRCS ${${objectName}_SRCS}
                 ./devices/hackrf-handler/hackrf-handler.cpp)

	   add_definitions (-DHAVE_HACKRF)
	endif (HACKRF)

	if (LIME)

	   include_directories ( ./devices/lime-handler)

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/lime-handler/lime-handler.h
           )

	   set ($(objectName)_HDRS
                  ${${objectName}_HDRS}
                  ./devices/lime-handler/lime-handler.h
                  ./devices/lime-handler/lime-widget.h 
                  ./devices/lime-handler/LMS7002M_parameters.h)

	   set (${objectName}_SRCS
	          ${${objectName}_SRCS}
                  ./devices/lime-handler/lime-handler.cpp)

	   add_definitions (-DHAVE_LIME)
	endif (LIME)
#
	if (PLUTO)

	   include_directories (./devices/pluto-handler )

	   set (${objectName}_UIS
                ${${objectName}_UIS}
                ./devices/pluto-handler/pluto-widget.ui
           )
	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/pluto-handler/pluto-handler.h
           )

	   set ($(objectName)_HDRS
                  ${${objectName}_HDRS}
                  ./devices/pluto-handler/pluto-handler.h
	          ./devices/pluto-handler/fmFilter.h)

	   set (${objectName}_SRCS ${${objectName}_SRCS}
                  ./devices/pluto-handler/pluto-handler.cpp)

	   add_definitions (-DHAVE_PLUTO)
	endif (PLUTO)
#
	if (ELAD_S1)
	   include_directories (
	        ./devices/sw-elad-s1/
	   )

	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/sw-elad-s1/elad_widget.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/sw-elad-s1/elad-s1.h
	        ./devices/sw-elad-s1/elad-worker.h
	   )

	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	        ./devices/sw-elad-s1/elad-s1.h
	        ./devices/sw-elad-s1/elad-loader.h
	        ./devices/sw-elad-s1/elad-worker.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/sw-elad-s1/elad-s1.cpp
	        ./devices/sw-elad-s1/elad-loader.cpp
	        ./devices/sw-elad-s1/elad-worker.cpp
	   )

	   add_definitions (-DHAVE_ELAD_S1)
	endif()
#
	QT5_WRAP_UI(UIS ${${objectName}_UIS})

	set ( ${objectName}_MOCS
	      ${${objectName}_MOCS}
	      ./radio.h	
	      ./includes/popup-keypad.h
	      ./devices/device-handler.h
	      ./includes/rds/rds-groupdecoder.h 
	      ./includes/rds/rds-blocksynchronizer.h
	      ./includes/rds/rds-decoder.h
	      ./includes/rds/rds-decoder-1.h
	      ./includes/rds/rds-decoder-2.h
	      ./includes/rds/rds-decoder-3.h
	      ./includes/fm/fm-processor.h
	      ./includes/scopes-qwt6/iqdisplay.h
	      ./includes/scopes-qwt6/scope.h
	      ./includes/various/keyboardfilter.h
	      ./includes/various/program-list.h
	      ./includes/various/squelchClass.h
	      ./devices/filereader/filereader.h
	      ./devices/filereader/filehulp.h
	      ./devices/deviceselect.h
	      ./includes/scopes-qwt6/hs-scope.h
	      ./includes/scopes-qwt6/ls-scope.h
	)

	QT5_WRAP_CPP (MOCS ${${objectName}_MOCS})

	include_directories (SYSTEM
		                  ${QWT_INCLUDE_DIRS}
	                     ${FFTW_INCLUDE_DIRS}
	                     ${SAMPLERATE_INCLUDE_DIRS}
	                     ${PORTAUDIO_INCLUDE_DIRS}
	                     ${SNDFILES_INCLUDE_DIRS})

	add_executable (${objectName}
	                ${${objectName}_SRCS}
	                ${UIS}
	                ${MOCS}
	                resources.qrc
	)

	target_link_libraries (${objectName}
	                       Qt5::Widgets
	                       Qt5::Xml
	                       Qt5::Multimedia
	                       ${FFTW_LIBRARIES}
	                       ${extraLibs}
	                       ${SAMPLERATE_LIBRARIES}
	                       ${QWT_LIBRARIES})

	INSTALL (TARGETS ${objectName} DESTINATION ./linux-bin)
