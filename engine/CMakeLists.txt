#--------------------------------------------------------------------------
# engine
#--------------------------------------------------------------------------

SET(engine_files 
	window.h
	window.cc
	vec3.h
	color.h
	mat4.h
	hit_result.h
	pbr.h
	ray.h
	raytracer.h
	raytracer.cc
	sphere.h
	random.h
	random.cc
	material.h
	material.cc
	mempool.h
	threadpool.h
	threadpool.cpp
	settings.h
)
SOURCE_GROUP("engine" FILES ${engine_files})
ADD_LIBRARY(engine STATIC ${engine_files})
ADD_DEPENDENCIES(engine glew glfw)
TARGET_INCLUDE_DIRECTORIES(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(engine PUBLIC exts glew glfw ${OPENGL_LIBS})

