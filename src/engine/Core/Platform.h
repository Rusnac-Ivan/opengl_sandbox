#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #include <emscripten/html5.h>
    #include <emscripten/val.h>
    #include <emscripten/bind.h>
    #define GL_GLEXT_PROTOTYPES
    #define EGL_EGLEXT_PROTOTYPES
#else
    #include <glad/glad.h>
#endif

#include <GLFW/glfw3.h>
#include <cassert>

enum struct DataType
{
	BYTE = GL_BYTE,
	UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
	SHORT = GL_SHORT,
	UNSIGNED_SHORT = GL_UNSIGNED_SHORT,
	INT = GL_INT,
	UNSIGNED_INT = GL_UNSIGNED_INT,
	HALF_FLOAT = GL_HALF_FLOAT,
	FLOAT = GL_FLOAT,
	DOUBLE = GL_DOUBLE
};


unsigned short DataSize(const DataType& dataType);



#define ASSERT(x) if(!(x)) __debugbreak();

#ifdef NDEBUG
    #define GL(line) gl##line
#else
    #ifdef __EMSCRIPTEN__
        #define GL(line)\
                gl##line;\
                do{\
                    GLenum error = glGetError( );\
                    if(error != GL_NO_ERROR)\
                    {\
                        emscripten::val error = emscripten::val::global("Error").new_();\
                        emscripten::val::global("console").call<void>("log", error["stack"]);\
                    }\
                    switch ( error )\
                    {\
                        case GL_INVALID_ENUM:                   assert( "GL_INVALID_ENUM" && NULL ); break;\
                        case GL_INVALID_VALUE:                  assert( "GL_INVALID_VALUE" && NULL ); break;\
                        case GL_INVALID_OPERATION:              assert( "GL_INVALID_OPERATION" && NULL ); break;\
                        case GL_STACK_OVERFLOW:                 assert( "GL_STACK_OVERFLOW" && NULL ); break;\
                        case GL_STACK_UNDERFLOW:                assert( "GL_STACK_UNDERFLOW" && NULL ); break;\
                        case GL_OUT_OF_MEMORY:                  assert( "GL_OUT_OF_MEMORY" && NULL ); break;\
                        case GL_INVALID_FRAMEBUFFER_OPERATION:  assert( "GL_INVALID_FRAMEBUFFER_OPERATION" && NULL ); break;\
                        default: break;\
                    }\
                } while (0);\

    #else
        #define GL(line)\
                gl##line;\
                do{\
                    GLenum error = glGetError( );\
                    switch ( error )\
                    {\
                        case GL_INVALID_ENUM:                   assert( "GL_INVALID_ENUM" && NULL ); break;\
                        case GL_INVALID_VALUE:                  assert( "GL_INVALID_VALUE" && NULL ); break;\
                        case GL_INVALID_OPERATION:              assert( "GL_INVALID_OPERATION" && NULL ); break;\
                        case GL_STACK_OVERFLOW:                 assert( "GL_STACK_OVERFLOW" && NULL ); break;\
                        case GL_STACK_UNDERFLOW:                assert( "GL_STACK_UNDERFLOW" && NULL ); break;\
                        case GL_OUT_OF_MEMORY:                  assert( "GL_OUT_OF_MEMORY" && NULL ); break;\
                        case GL_INVALID_FRAMEBUFFER_OPERATION:  assert( "GL_INVALID_FRAMEBUFFER_OPERATION" && NULL ); break;\
                        default: break;\
                    }\
                } while (0);\

    #endif
#endif


#ifdef __EMSCRIPTEN__
    #define GLSL(str) (const char *)"#version 300 es\n" #str
#define GL_ES
#else
    #define GLSL(str) (const char *)"#version 330 core\n" #str
#endif

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#endif