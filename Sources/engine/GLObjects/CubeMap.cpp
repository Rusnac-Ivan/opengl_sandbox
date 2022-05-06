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
	em_async_wget_onload_func OnLoad()
	{

	}
	void CubeMap::SetPositiveX(std::string file_name)
	{
		int width, height, nrChannels;
#ifdef __EMSCRIPTEN__

		EM_ASM(
			FS.mkdir('/resources');
			FS.mount(NODEFS, { root: '.' }, '/resources');
		);

		const char *file = nullptr;
		//emscripten_wget(file_name.c_str(), file);
		
		/*emscripten_async_wget_data(file_name.c_str(), NULL, [](void *arg, void *data, int size){
			int width, height, nrChannels;
			unsigned char *data = stbi_load_from_memory((unsigned char*)data, size, &width, &height, &nrChannels, 0);
			//unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
			if (data)
			{
				Bind(mTarget);
				GL(TexImage2D(Target::POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
				UnBind(mTarget);
				stbi_image_free(data);
			}
			assert(data && "stb not load texture");

		}, [](void *arg){

		});*/
		printf("file: %s\n", file_name.c_str());
		//assert(file && "emscripten not load texture");
		// unsigned char* data = stbi_load_from_memory(file, &width, &height, &nrChannels, 0);
		unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);
#else
		unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);
#endif
		if (data)
		{
			printf("data: %p\n", data);
			Bind(mTarget);
			GL(TexImage2D(Target::POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
			UnBind(mTarget);
			stbi_image_free(data);
		}
		assert(data && "stb not load texture");
	}
	
	void CubeMap::SetNegativeX(std::string file_name)
	{
		int width, height, nrChannels;
#ifdef __EMSCRIPTEN__
		const char *file = nullptr;
		emscripten_wget(file_name.c_str(), file);
		printf("file: %s\n", file);
		assert(file && "emscripten not load texture");
		// unsigned char* data = stbi_load_from_memory(file, &width, &height, &nrChannels, 0);
		unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
#else
		unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);
#endif
		if (data)
		{
			Bind(mTarget);
			GL(TexImage2D(Target::NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
			UnBind(mTarget);
			stbi_image_free(data);
		}
		assert(data && "stb not load texture");
	}
	void CubeMap::SetPositiveY(std::string file_name)
	{
		int width, height, nrChannels;
#ifdef __EMSCRIPTEN__
		const char *file = nullptr;
		emscripten_wget(file_name.c_str(), file);
		printf("file: %s\n", file);
		assert(file && "emscripten not load texture");
		unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
		// unsigned char* data = stbi_load_from_memory(file, &width, &height, &nrChannels, 0);
#else
		unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);
#endif
		if (data)
		{
			Bind(mTarget);
			GL(TexImage2D(Target::POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
			UnBind(mTarget);
			stbi_image_free(data);
		}
		assert(data && "stb not load texture");
	}
	void CubeMap::SetNegativeY(std::string file_name)
	{
		int width, height, nrChannels;
#ifdef __EMSCRIPTEN__
		const char *file = nullptr;
		emscripten_wget(file_name.c_str(), file);
		printf("file: %s\n", file);
		assert(file && "emscripten not load texture");
		// unsigned char* data = stbi_load_from_memory(file, &width, &height, &nrChannels, 0);
		unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
#else
		unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);
#endif
		if (data)
		{
			Bind(mTarget);
			GL(TexImage2D(Target::NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
			UnBind(mTarget);
			stbi_image_free(data);
		}
		assert(data && "stb not load texture");
	}
	void CubeMap::SetPositiveZ(std::string file_name)
	{
		int width, height, nrChannels;
#ifdef __EMSCRIPTEN__
		const char *file = nullptr;
		emscripten_wget(file_name.c_str(), file);
		printf("file: %s\n", file);
		assert(file && "emscripten not load texture");
		// unsigned char* data = stbi_load_from_memory(file, &width, &height, &nrChannels, 0);
		unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
#else
		unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);
#endif
		if (data)
		{
			Bind(mTarget);
			GL(TexImage2D(Target::POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
			UnBind(mTarget);
			stbi_image_free(data);
		}
		assert(data && "stb not load texture");
	}
	void CubeMap::SetNegativeZ(std::string file_name)
	{
		int width, height, nrChannels;
#ifdef __EMSCRIPTEN__
		const char *file = nullptr;
		emscripten_wget(file_name.c_str(), file);
		printf("file: %s\n", file);
		assert(file && "emscripten not load texture");
		// unsigned char* data = stbi_load_from_memory(file, &width, &height, &nrChannels, 0);
		unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
#else
		unsigned char *data = stbi_load(file_name.c_str(), &width, &height, &nrChannels, 0);
#endif
		if (data)
		{
			Bind(mTarget);
			GL(TexImage2D(Target::NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
			UnBind(mTarget);
			stbi_image_free(data);
		}
		assert(data && "stb not load texture");
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
		if (sampler.generateMipmaps)
		{
			GL(GenerateMipmap(mTarget));
		}
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