#--------------------------------------------------------------------------
# tester
#--------------------------------------------------------------------------

PROJECT(tester)

SET(tester_files 
	stb_image_write.h
	tester.cpp
)
SOURCE_GROUP("code" FILES ${tester_files})

ADD_EXECUTABLE(tester ${tester_files})
TARGET_LINK_LIBRARIES(tester engine)
ADD_DEPENDENCIES(tester engine)

IF(MSVC)
	SET_PROPERTY(TARGET tester PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
ENDIF(MSVC)