#ifndef _CUBE_MAP_H_
#define _CUBE_MAP_H_

#include <GLObjects/Texture.h>
#include <string>
#include <GLObjects/Program.h>
#include <GLObjects/VertexBuffer.h>
#include <GLObjects/VertexArray.h>
#include <glm/glm.hpp>

namespace gl
{
	class CubeMap : public Texture
	{
	public:
		enum Target
		{
			POSITIVE_X = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			NEGATIVE_X = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			POSITIVE_Y = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			NEGATIVE_Y = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			POSITIVE_Z = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
			NEGATIVE_Z = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
		};

		struct Sampler
		{
			FilterMode magFilter;
			FilterMode minFilter;
			WrapMode wrapModeS;
			WrapMode wrapModeT;
			WrapMode wrapModeR;
		};

	private:
		static constexpr Type mTarget = Type::TARGET_CUBE_MAP;

		Sampler mSampler;

		Program mProgram;

		VertexBuffer mVBO;
		VertexArray mVAO;

	public:
		CubeMap();
		~CubeMap();

		void SetPositiveX(std::string file_name);
		void SetNegativeX(std::string file_name);
		void SetPositiveY(std::string file_name);
		void SetNegativeY(std::string file_name);
		void SetPositiveZ(std::string file_name);
		void SetNegativeZ(std::string file_name);

		void SetSampler(const Sampler &sampler);

		void Draw(const glm::mat4 &view, const glm::mat4 &proj);

	private:
	};
}

#endif