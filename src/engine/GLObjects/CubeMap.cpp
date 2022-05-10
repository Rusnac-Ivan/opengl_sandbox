#include "CubeMap.h"
#include <Core/Platform.h>
//#define STB_IMAGE_IMPLEMENTATION
//#define STBI_WINDOWS_UTF8

#ifdef __EMSCRIPTEN__
#include <emscripten/wget.h>
#endif

#include "stb_image.h"
#include <GLObjects/Shader.h>
#include <GLObjects/Pipeline.h>
#include <functional>

namespace gl
{
	static float skyboxVertices[] = {
		// positions
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f};

	CubeMap::CubeMap()
	{
		const char *vertShader = GLSL(
#ifdef GL_ES
			\nprecision highp int; \n
				precision highp float; \n
#endif \n
					layout(location = 0) in vec3 aPos;

			out vec3 TexCoords;

			uniform mat4 projection;
			uniform mat4 view;

			void main() {
				TexCoords = aPos;
				vec4 pos = projection * view * vec4(aPos * 100.0, 1.0);
				gl_Position = pos.xyww;
			});

		int vertShSize = strlen(vertShader);
		const char *fragShader = GLSL(
#ifdef GL_ES
			\nprecision highp int; \n
				precision highp float; \n
#endif \n
					out vec4 FragColor;

			in vec3 TexCoords;

			uniform samplerCube skybox;

			void main() {
				FragColor = texture(skybox, TexCoords);
			});
		int fragShSize = strlen(fragShader);

		gl::Shader<gl::ShaderType::VERTEX> vertSh;
		gl::Shader<gl::ShaderType::FRAGMENT> fragSh;

		vertSh.LoadSources(1, &vertShader, &vertShSize);
		fragSh.LoadSources(1, &fragShader, &fragShSize);

		mProgram.Attach(&vertSh, &fragSh, NULL);

		mProgram.Link();

		mVBO.Data(36, sizeof(skyboxVertices), skyboxVertices, Buffer::UsageMode::STATIC_DRAW);
		mVBO.AttributesPattern({VertexBuffer::AttribType::POSITION});

		mVAO.LinkVBO(&mProgram, &mVBO);
	}

	CubeMap::~CubeMap()
	{
	}
	void CubeMap::SetPositiveX(std::string file_name)
	{

#ifdef __EMSCRIPTEN__
		/*void *pBuff = nullptr;
		int num = 0;
		int error = 0;
		emscripten_wget_data(file_name.c_str(), &pBuff, &num, &error);
		if (error == 0)
		{
			printf("get data, pBuff: %p, num: %d\n", pBuff, num);

			int width, height, nrChannels;
			unsigned char *img_data = stbi_load_from_memory((unsigned char *)pBuff, num, &width, &height, &nrChannels, 0);
			// unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
			printf("load image w: %d, h: %d, c: %d\n", width, height, nrChannels);
			if (img_data)
			{
				printf("data: %p\n", img_data);
				Bind(mTarget);
				GL(TexImage2D(Target::POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data));
				UnBind(mTarget);
				stbi_image_free(img_data);
			}
			assert(img_data && "stb not load texture");

			delete[] pBuff;
		}
		else
		{
			printf("pError code: %d\n", error);
			assert(error == 0 && "emscripten_wget_data Error !");
		}*/

		emscripten_async_wget_data(
			file_name.c_str(), this, [](void *arg, void *data, int size)
			{
				int width, height, nrChannels;
				unsigned char *img_data = stbi_load_from_memory((unsigned char *)data, size, &width, &height, &nrChannels, 0);
				// unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
				printf("load image w: %d, h: %d, c: %d\n", width, height, nrChannels);
				if (img_data)
				{
					CubeMap *thiz = (CubeMap*)arg;
					thiz->Bind(thiz->mTarget);
					GL(TexImage2D(Target::POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data));
					thiz->UnBind(thiz->mTarget);
					stbi_image_free(img_data);
				}
				assert(img_data && "stb not load texture"); },
			[](void *arg)
			{
				CubeMap *thiz = (CubeMap *)arg;
				printf("emscripten_async_wget_data Error \n");
			});
		printf("file: %s\n", file_name.c_str());
#else
		int width, height, nrChannels;
		unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);

		if (data)
		{
			printf("data: %p\n", data);
			Bind(mTarget);
			GL(TexImage2D(Target::POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
			UnBind(mTarget);
			stbi_image_free(data);
		}
		assert(data && "stb not load texture");
#endif
	}

	void CubeMap::SetNegativeX(std::string file_name)
	{
#ifdef __EMSCRIPTEN__
		emscripten_async_wget_data(
			file_name.c_str(), this, [](void *arg, void *data, int size)
			{
				int width, height, nrChannels;
				unsigned char *img_data = stbi_load_from_memory((unsigned char *)data, size, &width, &height, &nrChannels, 0);
				// unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
				printf("load image w: %d, h: %d, c: %d\n", width, height, nrChannels);
				if (img_data)
				{
					CubeMap *thiz = (CubeMap*)arg;
					thiz->Bind(thiz->mTarget);
					GL(TexImage2D(Target::NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data));
					thiz->UnBind(thiz->mTarget);
					stbi_image_free(img_data);
				}
				assert(img_data && "stb not load texture"); },
			[](void *arg)
			{
				CubeMap *thiz = (CubeMap *)arg;
				printf("emscripten_async_wget_data Error \n");
			});
		printf("file: %s\n", file_name.c_str());
#else
		int width, height, nrChannels;
		unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			Bind(mTarget);
			GL(TexImage2D(Target::NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
			UnBind(mTarget);
			stbi_image_free(data);
		}
		assert(data && "stb not load texture");
#endif
	}
	void CubeMap::SetPositiveY(std::string file_name)
	{
#ifdef __EMSCRIPTEN__
		emscripten_async_wget_data(
			file_name.c_str(), this, [](void *arg, void *data, int size)
			{
				int width, height, nrChannels;
				unsigned char *img_data = stbi_load_from_memory((unsigned char *)data, size, &width, &height, &nrChannels, 0);
				// unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
				printf("load image w: %d, h: %d, c: %d\n", width, height, nrChannels);
				if (img_data)
				{
					CubeMap *thiz = (CubeMap*)arg;
					thiz->Bind(thiz->mTarget);
					GL(TexImage2D(Target::POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data));
					thiz->UnBind(thiz->mTarget);
					stbi_image_free(img_data);
				}
				assert(img_data && "stb not load texture"); },
			[](void *arg)
			{
				CubeMap *thiz = (CubeMap *)arg;
				printf("emscripten_async_wget_data Error \n");
			});
		printf("file: %s\n", file_name.c_str());
#else
		int width, height, nrChannels;
		unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			Bind(mTarget);
			GL(TexImage2D(Target::POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
			UnBind(mTarget);
			stbi_image_free(data);
		}
		assert(data && "stb not load texture");
#endif
	}
	void CubeMap::SetNegativeY(std::string file_name)
	{

#ifdef __EMSCRIPTEN__
		emscripten_async_wget_data(
			file_name.c_str(), this, [](void *arg, void *data, int size)
			{
				int width, height, nrChannels;
				unsigned char *img_data = stbi_load_from_memory((unsigned char *)data, size, &width, &height, &nrChannels, 0);
				// unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
				printf("load image w: %d, h: %d, c: %d\n", width, height, nrChannels);
				if (img_data)
				{
					CubeMap *thiz = (CubeMap*)arg;
					thiz->Bind(thiz->mTarget);
					GL(TexImage2D(Target::NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data));
					thiz->UnBind(thiz->mTarget);
					stbi_image_free(img_data);
				}
				assert(img_data && "stb not load texture"); },
			[](void *arg)
			{
				CubeMap *thiz = (CubeMap *)arg;
				printf("emscripten_async_wget_data Error \n");
			});
		printf("file: %s\n", file_name.c_str());
#else
		int width, height, nrChannels;
		unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			Bind(mTarget);
			GL(TexImage2D(Target::NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
			UnBind(mTarget);
			stbi_image_free(data);
		}
		assert(data && "stb not load texture");
#endif
	}
	void CubeMap::SetPositiveZ(std::string file_name)
	{

#ifdef __EMSCRIPTEN__
		emscripten_async_wget_data(
			file_name.c_str(), this, [](void *arg, void *data, int size)
			{
				int width, height, nrChannels;
				unsigned char *img_data = stbi_load_from_memory((unsigned char *)data, size, &width, &height, &nrChannels, 0);
				// unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
				printf("load image w: %d, h: %d, c: %d\n", width, height, nrChannels);
				if (img_data)
				{
					CubeMap *thiz = (CubeMap*)arg;
					thiz->Bind(thiz->mTarget);
					GL(TexImage2D(Target::POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data));
					thiz->UnBind(thiz->mTarget);
					stbi_image_free(img_data);
				}
				assert(img_data && "stb not load texture"); },
			[](void *arg)
			{
				CubeMap *thiz = (CubeMap *)arg;
				printf("emscripten_async_wget_data Error \n");
			});
		printf("file: %s\n", file_name.c_str());
#else
		int width, height, nrChannels;
		unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			Bind(mTarget);
			GL(TexImage2D(Target::POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
			UnBind(mTarget);
			stbi_image_free(data);
		}
		assert(data && "stb not load texture");
#endif
	}
	void CubeMap::SetNegativeZ(std::string file_name)
	{

#ifdef __EMSCRIPTEN__
		emscripten_async_wget_data(
			file_name.c_str(), this, [](void *arg, void *data, int size)
			{
				int width, height, nrChannels;
				unsigned char *img_data = stbi_load_from_memory((unsigned char *)data, size, &width, &height, &nrChannels, 0);
				// unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
				printf("load image w: %d, h: %d, c: %d\n", width, height, nrChannels);
				if (img_data)
				{
					CubeMap *thiz = (CubeMap*)arg;
					thiz->Bind(thiz->mTarget);
					GL(TexImage2D(Target::NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data));
					thiz->UnBind(thiz->mTarget);
					stbi_image_free(img_data);
				}
				assert(img_data && "stb not load texture"); },
			[](void *arg)
			{
				CubeMap *thiz = (CubeMap *)arg;
				printf("emscripten_async_wget_data Error \n");
			});
		printf("file: %s\n", file_name.c_str());
#else
		int width, height, nrChannels;
		unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			Bind(mTarget);
			GL(TexImage2D(Target::NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
			UnBind(mTarget);
			stbi_image_free(data);
		}
		assert(data && "stb not load texture");
#endif
	}

	void CubeMap::SetSampler(const Sampler &sampler)
	{
		mSampler = sampler;
		Bind(mTarget);
		GLenum min_fil = static_cast<GLenum>(sampler.minFilter);
		GLenum mag_fil = static_cast<GLenum>(sampler.magFilter);
		GLenum wrap_s = static_cast<GLenum>(sampler.wrapModeS);
		GLenum wrap_t = static_cast<GLenum>(sampler.wrapModeT);
		GLenum wrap_r = static_cast<GLenum>(sampler.wrapModeR);
		GL(TexParameteri(mTarget, SamplerParam::MIN_FILTER, min_fil));
		GL(TexParameteri(mTarget, SamplerParam::MAG_FILTER, mag_fil));
		GL(TexParameteri(mTarget, SamplerParam::WRAP_S, wrap_s));
		GL(TexParameteri(mTarget, SamplerParam::WRAP_T, wrap_t));
		GL(TexParameteri(mTarget, SamplerParam::WRAP_R, wrap_r));

		UnBind(mTarget);
	}

	void CubeMap::Draw(const glm::mat4 &view, const glm::mat4 &proj)
	{
		mProgram.Use();

		mProgram.SetMatrix4(mProgram.Uniform("view"), view);
		mProgram.SetMatrix4(mProgram.Uniform("projection"), proj);

		Pipeline::SetDepthFunc(CompareFunc::LEQUAL);

		Activate(mTarget, 0);
		mVAO.Draw(Primitive::TRIANGLES);

		Pipeline::SetDepthFunc(CompareFunc::LESS);

		mProgram.StopUsing();
	}
}