#ifndef _CUBE_MAP_H_
#define _CUBE_MAP_H_

#include <GLObjects/Texture.h>
namespace gl
{
	class CubeMap : public Texture
	{
	public:
		enum Target
		{
			RIGHT	= GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			LEFT	= GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			TOP		= GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			BOTTOM	= GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			BACK	=  GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
			FRONT	= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
		};

		struct Sampler
		{
			FilterMode magFilter;
			FilterMode minFilter;
			WrapMode wrapModeS;
			WrapMode wrapModeT;
			bool generateMipmaps;
		};
	private:

		static constexpr Type mTarget = Type::TARGET_CUBE_MAP;

		Sampler mSampler;
	public:
		CubeMap();
		~CubeMap();

	private:

	};

	CubeMap::CubeMap()
	{
	}

	CubeMap::~CubeMap()
	{

	}
}


#endif