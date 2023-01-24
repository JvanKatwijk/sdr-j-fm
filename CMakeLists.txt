cmake_minimum_required( VERSION 2.8.11 )
set (objectName fmreceiver-3.0)
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
#	if you want support for any of these devices, uncomment the line
#	set(PMSDR true)
#	set(SDRPLAY true)
#	set(SDRPLAY_V3 true)
#	set(AIRSPY true)
	set(DABSTICK true)
#	set(ELAD_S1 true)

########################################################################
	find_package (PkgConfig)
        find_package (LSBId)

        find_package (Qt5Core REQUIRED)
        find_package (Qt5Widgets REQUIRED)
        find_package (Qt5Network REQUIRED)
        find_package (Qt5Xml REQUIRED)
        include_directories (
          ${Qt5Network_INCLUDE_DIRS}
          ${Qt5Xml_INCLUDE_DIRS}
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
	if (FFTW_INCLUDE_DIRS)
#	Already in cache, be silent
	   set (FFTW_FIND_QUIETLY TRUE)
	endif (FFTW_INCLUDE_DIRS)

	find_path (FFTW_INCLUDE_DIR fftw3.h
	           HINTS
                   ENV FFTW_ROOT
                   PATHS 
	           PATH_SUFFIXES
	           include
	           Include
	)

	list (APPEND FFTW_INCLUDE_DIRS ${FFTW_INCLUDE_DIR})
	find_library (FFTW_LIBRARIES NAMES fftw3f
                      HINTS ENV FFTW_ROOT
                      PATHS
                      PATH_SUFFIXES lib64 Lib64 lib Lib)

	message (STATUS ${FFTW_LIBRARIES} ${FFTW_INCLUDE_DIRS})
#	handle the QUIETLY and REQUIRED arguments and set FFTW_FOUND to TRUE if
#	all listed variables are TRUE

	include (FindPackageHandleStandardArgs)
	find_package_handle_standard_args (
	                  FFTW DEFAULT_MSG FFTW_LIBRARIES FFTW_INCLUDE_DIRS)
	mark_as_advanced (FFTW_LIBRARIES FFTW_INCLUDE_DIRS)

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
	     ./includes/various/fft.h 
	     ./includes/various/oscillator.h 
	     ./includes/various/ringbuffer.h 
	     ./includes/various/pllC.h 
	     ./includes/various/sincos.h 
	     ./includes/various/newconverter.h 
	     ./includes/various/fir-filters.h 
	     ./includes/various/fft-filters.h 
	     ./includes/various/iir-filters.h 
	     ./includes/various/Xtan2.h 
	     ./includes/various/themechoser.h 
	     ./includes/output/audiosink.h 
	     ./includes/scopes-qwt6/iqdisplay.h 
	     ./includes/scopes-qwt6/scope.h 
	     ./includes/scopes-qwt6/spectrogramdata.h 
	     ./includes/scopes-qwt6/fft-scope.h 
	     ./includes/fm/fm-demodulator.h 
	     ./includes/fm/pilot-recover.h 
	     ./includes/fm/fm-processor.h 
	     ./includes/rds/rds-decoder.h 
	     ./includes/rds/rds-blocksynchronizer.h 
	     ./includes/rds/rds-group.h 
	     ./includes/rds/rds-groupdecoder.h  
	     ./includes/sdr/agc.h  
	     ./includes/sdr/costas.h  
	     ./includes/sdr/time_sync.h  
	     ./includes/sdr/shaping_filter.h  
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
	      ./src/various/fft.cpp 
	      ./src/various/oscillator.cpp 
	      ./src/various/pllC.cpp 
	      ./src/various/sincos.cpp 
	      ./src/various/newconverter.cpp 
	      ./src/various/fir-filters.cpp 
	      ./src/various/fft-filters.cpp 
	      ./src/various/iir-filters.cpp 
	      ./src/various/Xtan2.cpp 
	      ./src/various/themechoser.cpp 
	      ./src/output/audiosink.cpp 
	      ./src/scopes-qwt6/iqdisplay.cpp 
	      ./src/scopes-qwt6/scope.cpp 
	      ./src/scopes-qwt6/fft-scope.cpp 
	      ./src/fm/fm-demodulator.cpp 
	      ./src/fm/pilot-recover.cpp 
	      ./src/fm/fm-processor.cpp 
	      ./src/rds/rds-decoder.cpp 
	      ./src/rds/rds-blocksynchronizer.cpp 
	      ./src/rds/rds-group.cpp 
	      ./src/rds/rds-groupdecoder.cpp 
			./src/sdr/shaping_filter.cpp
			./devices/device-handler.cpp
			./devices/deviceselect.cpp
	      ./devices/filereader/filereader.cpp 
	      ./devices/filereader/filehulp.cpp
	)

	set ( ${objectName}_UIS 
	      ./forms/radio.ui 
	      ./forms.configwidget.ui
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
	   find_library (SDRPLAYLIB libmirsdrapi-rsp.so)
	   if (NOT(SDRPLAYLIB))
	      message (FATAL_ERROR "please install sdrplay library")
	   else (NOT(SDRPLAYLIB))
	      set (extraLibs ${extraLibs} ${SDRPLAYLIB})
	   endif()
	   find_path (SDRPLAYLIB_INCLUDE_DIR
	              NAMES mirsdrapi-rsp.h
	              PATHS
	              /usr/local/include/
	   )
	   include_directories (${SDRPLAYLIB_INCLUDE_DIR})

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
     find_library (SDRPLAYLIB_V3 libsdrplay_api.so)
     if (NOT(SDRPLAYLIB_V3))
        message (FATAL_ERROR "please install sdrplay-v3 library")
     else (NOT(SDRPLAYLIB_V3))
        set (extraLibs ${extraLibs} ${SDRPLAYLIB_V3})
     endif()
     find_path (SDRPLAYLIB_V3_INCLUDE_DIR
                NAMES sdrplay_api.h
                PATHS
                /usr/local/include/
     )
     #message ("Tatata" ${SDRPLAYLIB_V3_INCLUDE_DIR})

     include_directories (${SDRPLAYLIB_V3_INCLUDE_DIR})

     set (${objectName}_UIS
          ${${objectName}_UIS}
          ./devices/sdrplay-handler-v3/sdrplay-widget-v3.ui
     )

     set (${objectName}_MOCS
          ${${objectName}_MOCS}
          ./devices/sdrplay-handler-v3/sdrplay-handler-v3.h
          #./devices/sdrplay-handler/sdrplayselect.h
     )

     include_directories (
       ./devices/sdrplay-handler-v3
     )

     set ($(objectName)_HDRS
          ${${objectName}_HDRS}
          ./devices/sdrplay-handler-v3/sdrplay-handler.h
          ./devices/sdrplay-handler-v3/sdrplay-commands.h
     )

     set (${objectName}_SRCS
          ${${objectName}_SRCS}
          ./devices/sdrplay-handler-v3/sdrplay-handler-v3.cpp
          #./devices/sdrplay-handler-v3/sdrplayselect.cpp
     )

     add_definitions (-DHAVE_SDRPLAY_V3)
  endif (SDRPLAY_V3)

  if (AIRSPY)
	   find_library (AIRSPY_LIB airspy)
	   if (NOT(AIRSPY_LIB))
	      message (FATAL_ERROR "please install airspy library")
	   else (NOT(AIRSPY_LIB))
	      set (extraLibs ${extraLibs} ${AIRSPY_LIB})
	   endif()
	   find_path (AIRSPYLIB_INCLUDE_DIR
	              NAMES airspy.h
	              PATHS
	              /usr/local/include/
	              /usr/local/include/libairspy
	   )
	   include_directories (${AIRSPYLIB_INCLUDE_DIR})

	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/airspy/airspy-widget.ui 
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/airspy/airspy-handler.h
	   )

	   include_directories (
	     ./devices/airspy
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

	if (DABSTICK)
	   find_library (RTLSDR rtlsdr)
	   if (NOT(RTLSDR))
	      message (FATAL_ERROR "please install librtlsdr")
	   else (NOT(RTLSDR))
	      set (extraLibs ${extraLibs} ${RTLSDR})
	   endif()
	   find_path (DABSTICK_INCLUDE_DIR
	              NAMES rtl-sdr.h
	              PATHS
	              /usr/local/include/
	              /usr/include/
	   )
	   include_directories (${RTLSDR_INCLUDE_DIR})

	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/rtlsdr-handler/dabstick-widget.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/rtlsdr-handler/dongleselect.h
	        ./devices/rtlsdr-handler/rtlsdr-handler.h
	   )

	   include_directories (
	        ./devices/rtlsdr-handler/
	   )

	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	        ./devices/rtlsdr-handler/rtlsdr-handler.h 
	        ./devices/rtlsdr-handler/dongleselect.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ./devices/rtlsdr-handler/rtlsdr-handler.cpp
	        ./devices/rtlsdr-handler/dongleselect.cpp
	   )

	   add_definitions (-DHAVE_DABSTICK)
	endif()


	if (ELAD_S1)
	   set (${objectName}_UIS
	        ${${objectName}_UIS}
	        ./devices/sw-elad-s1/elad_widget.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ./devices/sw-elad-s1/elad-s1.h
	        ./devices/sw-elad-s1/elad-worker.h
	   )

	   include_directories (
	        ./devices/sw-elad-s1/
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
	      ./includes/fm/fm-processor.h
	      ./includes/scopes-qwt6/iqdisplay.h
	      ./includes/scopes-qwt6/scope.h
	      ./includes/various/keyboardfilter.h
	      ./includes/various/program-list.h
	      ./includes/various/squelchClass.h
	      ./devices/filereader/filereader.h
	      ./devices/filereader/filehulp.h
	      ./includes/scopes-qwt6/fft-scope.h
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
	)

	target_link_libraries (${objectName}
	                       Qt5::Widgets
	                       ${FFTW_LIBRARIES}
	                       ${extraLibs}
	                       ${SAMPLERATE_LIBRARIES}
	                       ${QWT_LIBRARIES})

	INSTALL (TARGETS ${objectName} DESTINATION ./linux-bin)
