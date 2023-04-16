#--------------------------------------------------------------------------
# main
#--------------------------------------------------------------------------

PROJECT(main)

SET(main_files 
	main.cc
)
SOURCE_GROUP("code" FILES ${main_files})

ADD_EXECUTABLE(main ${main_files})
TARGET_LINK_LIBRARIES(main engine)
ADD_DEPENDENCIES(main engine)

IF(MSVC)
	SET_PROPERTY(TARGET main PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
ENDIF(MSVC)