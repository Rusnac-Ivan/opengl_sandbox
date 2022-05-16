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
			//uniform vec3 viewPos;
			//uniform float uTime = 0.0;

			float hash(float n)
			{
				return fract(sin(n) * 43758.5453123);
			}

			float noise(vec3 x)
			{
				vec3 f = fract(x);
				float n = dot(floor(x), vec3(1.0, 157.0, 113.0));
				return mix(mix(mix(hash(n + 0.0), hash(n + 1.0), f.x),
					mix(hash(n + 157.0), hash(n + 158.0), f.x), f.y),
					mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
						mix(hash(n + 270.0), hash(n + 271.0), f.x), f.y), f.z);
			}

			const mat3 m = mat3(0.0, 1.60, 1.20, -1.6, 0.72, -0.96, -1.2, -0.96, 1.28);
			float fbm(vec3 p)
			{
				float f = 0.0;
				f += noise(p) / 2.0; p = m * p * 1.1;
				f += noise(p) / 4.0; p = m * p * 1.2;
				f += noise(p) / 6.0; p = m * p * 1.3;
				f += noise(p) / 12.0; p = m * p * 1.4;
				f += noise(p) / 24.0;
				return f;
			}

			void main() {

				/*float cirrus = 0.4;
				float cumulus = 0.3;

				const float g = 0.98;
				const float Br = 0.0025;
				const float Bm = 0.0003;

				const vec3 nitrogen = vec3(0.680, 0.550, 0.440);
				const vec3 Kr = Br / pow(nitrogen, vec3(4.0));
				const vec3 Km = Bm / pow(nitrogen, vec3(0.84));

				vec3 color;
				//vec3 view = normalize(fragPos - viewPos);
				vec3 view = TexCoords;

				// Atmosphere Scattering pos
				float mu = dot(normalize(view), normalize(sunPos));

				vec3 extinction = mix(exp(-exp(-((view.y + sunPos.y * 4.0) * (exp(-view.y * 16.0) + 0.1) / 80.0) / Br) * (exp(-view.y * 16.0) + 0.1) * Kr / Br) * exp(-view.y * exp(-view.y * 8.0) * 4.0) * exp(-view.y * 2.0) * 4.0, vec3(1.0 - exp(sunPos.y)) * 0.2, -sunPos.y * 0.2 + 0.5);

				color.rgb = 3.0 / (8.0 * 3.14) * (1.0 + mu * mu) * (Kr + Km * (1.0 - g * g) / (2.0 + g * g) / pow(1.0 + g * g - 2.0 * g * mu, 1.5)) / (Br + Bm) * extinction;

				// Cirrus Clouds
				float density = smoothstep(1.0 - cirrus, 1.0, fbm(view.xyz / view.y * 2.0 + uTime * 0.05)) * 0.3;
				color.rgb = mix(color.rgb, extinction * 4.0, density * max(view.y, 0.0));

				// Cumulus Clouds
				for (int i = 0; i < 7; i++)
				{
					float density = smoothstep(1.0 - cumulus, 1.0, fbm((0.7 + float(i) * 0.01) * view.xyz / view.y + uTime / 4.0 * 0.3));
					color.rgb = mix(color.rgb, extinction * density * 5.0, min(density, 1.0) * max(view.y, 0.0));
				}

				// Dithering Noise
				color.rgb += noise(view * 1000.0) * 0.05;

				float exposure = 1.0;
				color = 1.0 - exp(-exposure * color);

				if (viewPos.y < 0.0)
				{
					vec3 a = view;
					vec3 P0 = viewPos;
					vec3 n = vec3(0.0, 1.0, 0.0);

					float d = 1.0;

					vec3 P = P0 - (d + dot(n, P0)) / (dot(n, a)) * a;

					vec3 ground_color;

					ground_color = uGroundColor;
					//ground_color = textureColor(P, texture(Texture, P.xz).rgb);

					float dist = distance(viewPos, P);

					float fog = 1.0 / (1.0 + exp(-0.2 * (dist - 30.0)));

					if (length(color) > 0.0)
						color = mix(ground_color, color, fog);
					else
						color = ground_color;
				}*/

				FragColor = texture(skybox, TexCoords);
				//FragColor = vec4(color, 1.0);
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
		mVAO.Draw(Primitive::TRIANGLES, 0, DataType::UNSIGNED_INT, NULL);

		Pipeline::SetDepthFunc(CompareFunc::LESS);

		mProgram.StopUsing();
	}
}