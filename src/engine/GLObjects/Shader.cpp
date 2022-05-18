#include "Shader.h"
#include <cstdio>
#include <cstdlib>
#include <cassert>

namespace gl
{
	template <ShaderType Shader_Type>
	Shader<Shader_Type>::Shader() : IShader(), mID(0)
	{
		mID = GL(CreateShader(static_cast<GLenum>(Shader_Type)));
		assert(mID != 0 && "Failed to create shader!");
	}

	template <ShaderType Shader_Type>
	Shader<Shader_Type>::~Shader()
	{
		if (!mID)
		{
			GL(DeleteShader(mID));
		}
	}

	template <ShaderType Shader_Type>
	void Shader<Shader_Type>::LoadSources(const unsigned short &count, const char **shaderCodes, const int *codeLengths)
	{
		GL(ShaderSource(mID, count, shaderCodes, codeLengths));

		GL(CompileShader(mID));

		CheckCompile();
	}

	template <ShaderType Shader_Type>
	void Shader<Shader_Type>::CheckCompile()
	{
		GLint is_success;
		char infoLog[1024];

		GL(GetShaderiv(mID, GL_COMPILE_STATUS, &is_success));
		if (!is_success)
		{

			GL(GetShaderInfoLog(mID, 1024, nullptr, infoLog));
			if (Shader_Type == ShaderType::FRAGMENT)
			{
				fprintf(stderr, "Failed to compile fragment shader: %s\n", infoLog);
				assert(false);
			}
			else if (Shader_Type == ShaderType::VERTEX)
			{
				fprintf(stderr, "Failed to compile vertex shader: %s\n", infoLog);
				assert(false);
			}

			GL(DeleteShader(mID));
			mID = 0;

			exit(1);
		}
	}

	template class Shader<ShaderType::VERTEX>;
	template class Shader<ShaderType::FRAGMENT>;
	template class Shader<ShaderType::GEOMETRY>;
	template class Shader<ShaderType::TESS_CONTROL>;
	template class Shader<ShaderType::TESS_EVALUATION>;
}