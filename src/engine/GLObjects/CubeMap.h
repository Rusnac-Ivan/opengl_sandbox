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

		glm::vec3 mSunPos;
		float mSpeed;
		float mAmount;

		float mHoriz;
		float mVert;

		bool isPosX;
		bool isNegX;
		bool isPosY;
		bool isNegY;
		bool isPosZ;
		bool isNegZ;

	public:
		CubeMap();
		~CubeMap();

		void SetSunPos(const glm::vec3& pos) { mSunPos = pos; }
		void SetSpeed(float speed) { mSpeed = speed; }
		void SetAmount(float amount) { mAmount = amount; }
		void SetHoriz(float horiz) 
		{ 
			mHoriz = horiz; 
			float sin_phi = glm::sin(glm::radians(mHoriz));
			float cos_phi = glm::cos(glm::radians(mHoriz));
			float sin_theta = glm::sin(glm::radians(mVert));
			float cos_theta = glm::cos(glm::radians(mVert));

			//mLook = ndVector3(cos_phi * sin_theta, cos_theta, sin_phi * sin_theta) * -1.f;
			mSunPos = glm::vec3(sin_phi * cos_theta, sin_theta, cos_phi * cos_theta);
			//mLook.Normalize();
			mSunPos = glm::normalize(mSunPos);
		}
		void SetVert(float vert) 
		{ 
			mVert = vert; 

			float sin_phi = glm::sin(glm::radians(mHoriz));
			float cos_phi = glm::cos(glm::radians(mHoriz));
			float sin_theta = glm::sin(glm::radians(mVert));
			float cos_theta = glm::cos(glm::radians(mVert));

			//mLook = ndVector3(cos_phi * sin_theta, cos_theta, sin_phi * sin_theta) * -1.f;
			mSunPos = glm::vec3(sin_phi * cos_theta, sin_theta, cos_phi * cos_theta);
			//mLook.Normalize();
			mSunPos = glm::normalize(mSunPos);
		}

		glm::vec3 GetSunPos() { return mSunPos; }
		float GetSpeed() { return mSpeed; }
		float GetAmount() { return mAmount; }
		float GetHoriz() { return mHoriz; }
		float GetVert() { return mVert; }

		void SetPositiveX(std::string file_name);
		void SetNegativeX(std::string file_name);
		void SetPositiveY(std::string file_name);
		void SetNegativeY(std::string file_name);
		void SetPositiveZ(std::string file_name);
		void SetNegativeZ(std::string file_name);

		void SetSampler(const Sampler &sampler);

		void Draw(const glm::vec3& viewPos, const glm::mat4 &view, const glm::mat4 &proj);

	private:
	};
}

#endif