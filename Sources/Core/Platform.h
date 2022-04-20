#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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



#define ASSERT(x) if(!(x)) __debugbreak();

#define GL(line)\
        {\
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
            } while (0); \
        }

#endif