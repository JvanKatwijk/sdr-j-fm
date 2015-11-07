
cmake_minimum_required( VERSION 2.8.11 )
set (CMAKE_BUILD_TYPE Release )
#
set (objectName sdr-j-fm-airspy)
set ( CMAKE_INSTALL_PREFIX ../../ )
add_definitions ( -Wall)
######################################################################
#       R E A D  T H I S
#####################################################################
#	adjust to your likings
#	if you want to use the SSE support on your machine, uncomment
#	set(NO_SSE_SUPPORT true)
#
#	if you want support for any of these devices, uncomment the line
#	set(SDRPLAY true)
	set(AIRSPY true)
#	set(DABSTICK true)
#	but do not uncomment more than one line
########################################################################
	find_package (Qt5Widgets REQUIRED)
	find_package (Qt5Declarative REQUIRED)

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
	   ../
	   ../includes
	   ../includes/output
	   ../includes/various
	   ../includes/fm
	   ../includes/rds
	   ../includes/various
	   ../src
	   ../src/output
	   ../src/various
	   ../src/fm
	   ../src/rds
	   ../src/various
	   ../input
	   ../input/filereader
	)

	set ( ${objectName}_HDRS
	      ./gui.h
	      ./popup-keypad.h
	      ../includes/fm-constants.h
	      ../includes/various/squelchClass.h
              ../includes/various/fft.h
	      ../includes/various/oscillator.h
              ../includes/various/ringbuffer.h
	      ../includes/various/pllC.h
	      ../includes/various/sincos.h
	      ../includes/various/resampler.h
	      ../includes/various/converter.h
	      ../includes/various/fir-filters.h
	      ../includes/various/fft-filters.h
	      ../includes/various/iir-filters.h
	      ../includes/various/Xtan2.h
	      ../includes/output/audiosink.h
	      ../includes/fm/fm-demodulator.h
	      ../includes/fm/fm-processor.h
	      ../includes/fm/fm-levels.h
	      ../includes/rds/rds-decoder.h
	      ../includes/rds/rds-blocksynchronizer.h
	      ../includes/rds/rds-group.h
	      ../includes/rds/rds-groupdecoder.h 
	      ../input/virtual-input.h
	)

	set ( ${objectName}_SRCS
	      ./main.cpp
	      ./gui.cpp
	      ./popup-keypad.cpp
              ../src/various/fft.cpp
	      ../src/various/oscillator.cpp
	      ../src/various/pllC.cpp
	      ../src/various/sincos.cpp
	      ../src/various/resampler.cpp
	      ../src/various/fir-filters.cpp
	      ../src/various/fft-filters.cpp
	      ../src/various/iir-filters.cpp
	      ../src/various/Xtan2.cpp
	      ../src/output/audiosink.cpp
	      ../src/fm/fm-demodulator.cpp
	      ../src/fm/fm-processor.cpp
	      ../src/fm/fm-levels.cpp
	      ../src/rds/rds-decoder.cpp
	      ../src/rds/rds-blocksynchronizer.cpp
	      ../src/rds/rds-group.cpp
	      ../src/rds/rds-groupdecoder.cpp
	      ../input/virtual-input.cpp
	)

	set ( ${objectName}_UIS ./sdrgui.ui 
	)

	set (${objectName}_MOCS ./popup-keypad.h)
#####################################################################
#

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
	        ../input/sdrplay/sdrplay-widget.ui 
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ../input/sdrplay/sdrplay.h
	        ../input/sdrplay/sdrplay-worker.h
	   )

	   include_directories (
	     ../input/sdrplay
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ../input/sdrplay/sdrplay.h
	        ../input/sdrplay/sdrplay-loader.h
	        ../input/sdrplay/sdrplay-worker.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ../input/sdrplay/sdrplay.cpp
	        ../input/sdrplay/sdrplay-worker.cpp
	        ../input/sdrplay/sdrplay-loader.cpp
	   )

	   add_definitions (-DHAVE_SDRPLAY)
	endif (SDRPLAY)

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
	        ../input/airspy/airspy-widget.ui 
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ../input/airspy/airspy-handler.h
	   )

	   include_directories (
	     ../input/airspy
	   )

	   set ($(objectName)_HDRS
	        ${${objectName}_HDRS}
	        ../input/airspy/airspy-handler.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ../input/airspy/airspy-handler.cpp
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
	        ../input/dabstick/dabstick-widget.ui
	   )

	   set (${objectName}_MOCS
	        ${${objectName}_MOCS}
	        ../input/dabstick/dongleselect.h
	        ../input/dabstick/dabstick.h
	   )

	   include_directories (
	        ../input/dabstick/
	   )

	   set (${objectName}_HDRS
	        ${${objectName}_HDRS}
	        ../input/dabstick/dabstick.h 
	        ../input/dabstick/dongleselect.h
	   )

	   set (${objectName}_SRCS
	        ${${objectName}_SRCS}
	        ../input/dabstick/dabstick.cpp
	        ../input/dabstick/dongleselect.cpp
	   )

	   add_definitions (-DHAVE_DABSTICK)
	endif()
#
	QT5_WRAP_UI(UIS ${${objectName}_UIS})

	set ( ${objectName}_MOCS
	      ${${objectName}_MOCS}
	      ./gui.h	
	      ./popup-keypad.h
	      ../input/virtual-input.h
	      ../includes/rds/rds-groupdecoder.h 
	      ../includes/rds/rds-blocksynchronizer.h
	      ../includes/rds/rds-decoder.h
	      ../includes/fm/fm-processor.h
	)

	QT5_WRAP_CPP (MOCS ${${objectName}_MOCS})

	include_directories (${QWT_INCLUDE_DIRS}
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

	INSTALL (TARGETS ${objectName} DESTINATION ../linux-bin)
