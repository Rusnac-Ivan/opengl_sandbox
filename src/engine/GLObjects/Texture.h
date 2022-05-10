 #ifndef _GL_TEXTURE_
#define _GL_TEXTURE_

#include <Core/Platform.h>
#include <cassert>
#include <map>

namespace gl
{
	
	/*enum struct TextureType
	{
		TARGET_1D					= GL_TEXTURE_1D,
		TARGET_2D					= GL_TEXTURE_2D,
		TARGET_3D					= GL_TEXTURE_3D,
		TARGET_1D_ARRAY				= GL_TEXTURE_1D_ARRAY,
		TARGET_2D_ARRAY				= GL_TEXTURE_2D_ARRAY,
		TARGET_RECTANGLE			= GL_TEXTURE_RECTANGLE,
		
		TARGET_CUBE_MAP				= GL_TEXTURE_CUBE_MAP,
		TARGET_CUBE_MAP_ARRAY		= GL_TEXTURE_CUBE_MAP_ARRAY,
		TARGET_BUFFER				= GL_TEXTURE_BUFFER,
		TARGET_2D_MULTISAMPLE		= GL_TEXTURE_2D_MULTISAMPLE,
		TARGET_2D_MULTISAMPLE_ARRAY	= GL_TEXTURE_2D_MULTISAMPLE_ARRAY
	};*/

	

	

	
	
	/*enum struct ParamName
	{
		DEPTH_STENCIL_MODE = GL_DEPTH_STENCIL_TEXTURE_MODE, 
		BASE_LEVEL = GL_TEXTURE_BASE_LEVEL,
		COMPARE_FUNC = GL_TEXTURE_COMPARE_FUNC,
		COMPARE_MODE = GL_TEXTURE_COMPARE_MODE,
		LOD_BIAS = GL_TEXTURE_LOD_BIAS,
		MIN_FILTER = GL_TEXTURE_MIN_FILTER,
		MAG_FILTER = GL_TEXTURE_MAG_FILTER,
		MIN_LOD = GL_TEXTURE_MIN_LOD,
		MAX_LOD = GL_TEXTURE_MAX_LOD,
		MAX_LEVEL = GL_TEXTURE_MAX_LEVEL,
		SWIZZLE_G = GL_TEXTURE_SWIZZLE_G,
		SWIZZLE_B = GL_TEXTURE_SWIZZLE_B,
		SWIZZLE_A = GL_TEXTURE_SWIZZLE_A,
		WRAP_S = GL_TEXTURE_WRAP_S,
		WRAP_T = GL_TEXTURE_WRAP_T,
		WRAP_R = GL_TEXTURE_WRAP_R,

		NONE = 0xFFFFFFFF,
	};

	enum struct ParamValue
	{
		NEAREST = GL_NEAREST,
		LINEAR = GL_LINEAR,
		NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
		NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
		LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
		LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,

		CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
		CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,

		REPEAT = GL_REPEAT,

		MIRROR_CLAMP_TO_EDGE = GL_MIRROR_CLAMP_TO_EDGE,
		MIRRORED_REPEAT = GL_MIRRORED_REPEAT,

		UNKNOWN = 0xFFFFFFFF,
	};*/


	


	class Texture
	{
		public:
			enum Type
			{
				TARGET_1D = GL_TEXTURE_1D,
				TARGET_2D = GL_TEXTURE_2D,
				TARGET_3D = GL_TEXTURE_3D,
				TARGET_CUBE_MAP = GL_TEXTURE_CUBE_MAP,
				TARGET_2D_MULTISAMPLE = GL_TEXTURE_2D_MULTISAMPLE,

				UNKNOWN = -1
			};

			enum struct Format
			{
				//Base
				RED = GL_RED,
				RG = GL_RG,
				RGB = GL_RGB,
				RGBA = GL_RGBA,
				DEPTH = GL_DEPTH_COMPONENT,
				DEPTH_STENCIL = GL_DEPTH_STENCIL,

				//Sized
				R8 = GL_R8,
				R16 = GL_R16,
				RG8 = GL_RG8,
				RG16 = GL_RG16,
				R16F = GL_R16F,
				R32F = GL_R32F,
				RG16F = GL_RG16F,
				RG32F = GL_RG32F,
				RGB16 = GL_RGB16,
				RGB16F = GL_RGB16F,
				RGBA16 = GL_RGBA16,
				RGBA16F = GL_RGBA16F,
				RGB32F = GL_RGB32F,
				RGBA32F = GL_RGBA32F,
				DEPTH32F = GL_DEPTH_COMPONENT32F,

				UNKNOWN = -1
			};

			enum SamplerParam
			{
				MIN_FILTER = GL_TEXTURE_MIN_FILTER,
				MAG_FILTER = GL_TEXTURE_MAG_FILTER,
				WRAP_S = GL_TEXTURE_WRAP_S,
				WRAP_T = GL_TEXTURE_WRAP_T,
				WRAP_R = GL_TEXTURE_WRAP_R,
			};

			enum struct WrapMode
			{
				CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
				CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
				REPEAT = GL_REPEAT,
				MIRROR_CLAMP_TO_EDGE = GL_MIRROR_CLAMP_TO_EDGE,
				MIRRORED_REPEAT = GL_MIRRORED_REPEAT,

				UNKNOWN = -1
			};

			enum struct FilterMode
			{
				NEAREST = GL_NEAREST,
				LINEAR = GL_LINEAR,
				NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
				NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
				LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
				LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,

				UNKNOWN = -1
			};

			

		private:
			unsigned int mID;

		public:
			Texture();
			virtual ~Texture();

			Texture(const Texture&) = delete;
			Texture& operator=(const Texture&) = delete;

			void Bind(Type type);
			void UnBind(Type type);

			void Activate(Type type, const unsigned short& unit);
			void Deactivate(Type type, const unsigned short& unit);

			unsigned int GetId() { return mID; }
	};


	


	class Texture2D : public Texture
	{
		public:

			struct Sampler
			{
				FilterMode magFilter;
				FilterMode minFilter;
				WrapMode wrapModeS;
				WrapMode wrapModeT;
				bool generateMipmaps;
			};
		private:

			static constexpr Type mTarget = Type::TARGET_2D;

			uint32_t	mWidth;
			uint32_t	mHeight;
			Format		mFormat;
			Format		mInternalFormat;
			DataType	mPixelDataFormat;
			uint8_t*	mRawData;
			uint32_t	mRawDataSize;
			bool		mGenerateMipmap;

			

			Sampler		mSampler;

			//std::map<ParamName, ParamValue> mParameters;

		public:
			Texture2D();
			virtual ~Texture2D();

			Texture2D(const Texture2D&) = delete;
			Texture2D& operator=(const Texture2D&) = delete;

			uint32_t GetWidth() { return mWidth; };
			uint32_t GetHeight() { return mHeight; };
			Format GetFormat() { return mFormat; };
			Format GetInternalFormat() { return mInternalFormat; };

			void SetSampler(const Sampler& sampler);

			void Bind();
			void UnBind();

			void SetTexture2D(int32_t level, Format internalformat, uint32_t width, uint32_t height, uint32_t border, Format format, DataType type, void* pixels);

			void Update(uint32_t x, uint32_t y, uint32_t width, uint32_t height, void* buffer);

		private:

	};

	

}



#endif // !_GL_TEXTURE_
