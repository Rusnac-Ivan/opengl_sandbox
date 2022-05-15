#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "Scene.h"
#include <Core/Platform.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <map>
#include <glm/gtx/quaternion.hpp>

namespace tinygltf
{
	static inline int32_t GetTypeSizeInBytes(uint32_t ty)
	{
		if (ty == TINYGLTF_TYPE_SCALAR)
		{
			return 1;
		}
		else if (ty == TINYGLTF_TYPE_VEC2)
		{
			return 2;
		}
		else if (ty == TINYGLTF_TYPE_VEC3)
		{
			return 3;
		}
		else if (ty == TINYGLTF_TYPE_VEC4)
		{
			return 4;
		}
		else if (ty == TINYGLTF_TYPE_MAT2)
		{
			return 4;
		}
		else if (ty == TINYGLTF_TYPE_MAT3)
		{
			return 9;
		}
		else if (ty == TINYGLTF_TYPE_MAT4)
		{
			return 16;
		}
		else
		{
			// Unknown componenty type
			return -1;
		}
	}
}

namespace Scene
{
	BoundingBox::BoundingBox() {}
	BoundingBox::BoundingBox(glm::vec3 min, glm::vec3 max) : min(min), max(max) {}
	BoundingBox BoundingBox::getAABB(glm::mat4 m)
	{
		glm::vec3 min = glm::vec3(m[3]);
		glm::vec3 max = min;
		glm::vec3 v0, v1;

		glm::vec3 right = glm::vec3(m[0]);
		v0 = right * this->min.x;
		v1 = right * this->max.x;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		glm::vec3 up = glm::vec3(m[1]);
		v0 = up * this->min.y;
		v1 = up * this->max.y;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		glm::vec3 back = glm::vec3(m[2]);
		v0 = back * this->min.z;
		v1 = back * this->max.z;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		return BoundingBox(min, max);
	}

	void Texture::fromglTfImage(tinygltf::Image &gltfimage, gl::Texture2D::Sampler sampler)
	{
		unsigned char *buffer = nullptr;
		size_t bufferSize = 0;
		bool deleteBuffer = false;

		if (gltfimage.component == 3)
		{
			// Most devices don't support RGB only on Vulkan so convert if necessary
			// TODO: Check actual format support and transform only if required
			bufferSize = gltfimage.width * gltfimage.height * 4;
			buffer = new unsigned char[bufferSize];
			unsigned char *rgba = buffer;
			unsigned char *rgb = &gltfimage.image[0];
			for (int32_t i = 0; i < gltfimage.width * gltfimage.height; ++i)
			{
				for (int32_t j = 0; j < 3; ++j)
				{
					rgba[j] = rgb[j];
				}
				rgba += 4;
				rgb += 3;
			}
			deleteBuffer = true;
		}
		else if (gltfimage.component == 4)
		{
			buffer = &gltfimage.image[0];
			bufferSize = gltfimage.image.size();
		}
		else
			assert(false && "unsupported texture format");

		DataType type = static_cast<DataType>(gltfimage.pixel_type);

		texture = std::make_unique<gl::Texture2D>();

		texture->SetTexture2D(0, gl::Texture::Format::RGBA, gltfimage.width, gltfimage.height, 0, gl::Texture::Format::RGBA, type, buffer);
		texture->SetSampler(sampler);

		if (deleteBuffer)
			delete[] buffer;
	}

	void Primitive::setBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		bb.min = min;
		bb.max = max;
		bb.valid = true;
	}

	// Mesh
	Mesh::Mesh(glm::mat4 matrix){

	};

	Mesh::~Mesh()
	{
		for (Primitive *p : primitives)
			delete p;
	}

	void Mesh::setBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		bb.min = min;
		bb.max = max;
		bb.valid = true;
	}

	// Node
	glm::mat4 Node::localMatrix()
	{
		return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
	}

	glm::mat4 Node::getMatrix()
	{
		glm::mat4 m = localMatrix();
		Node *p = parent;
		while (p)
		{
			m = p->localMatrix() * m;
			p = p->parent;
		}
		return m;
	}

	void Node::update()
	{
		if (mesh)
		{
			glm::mat4 m = getMatrix();
			/*if (skin) {
				mesh->uniformBlock.matrix = m;
				// Update join matrices
				glm::mat4 inverseTransform = glm::inverse(m);
				size_t numJoints = std::min((uint32_t)skin->joints.size(), MAX_NUM_JOINTS);
				for (size_t i = 0; i < numJoints; i++) {
					vkglTF::Node* jointNode = skin->joints[i];
					glm::mat4 jointMat = jointNode->getMatrix() * skin->inverseBindMatrices[i];
					jointMat = inverseTransform * jointMat;
					mesh->uniformBlock.jointMatrix[i] = jointMat;
				}
				mesh->uniformBlock.jointcount = (float)numJoints;
				memcpy(mesh->uniformBuffer.mapped, &mesh->uniformBlock, sizeof(mesh->uniformBlock));
			}
			else {
				memcpy(mesh->uniformBuffer.mapped, &m, sizeof(glm::mat4));
			}*/
			// memcpy(mesh->uniformBuffer.mapped, &m, sizeof(glm::mat4));
		}

		for (auto &child : children)
		{
			child->update();
		}
	}

	Node::~Node()
	{
		if (mesh)
		{
			delete mesh;
		}
		for (auto &child : children)
		{
			delete child;
		}
	}

	void Model::destroy()
	{

		/*for (auto texture : textures) {
			texture.destroy();
		}*/
		textures.clear();

		for (auto node : nodes)
		{
			delete node;
		}
		// materials.resize(0);

		nodes.resize(0);
		// linearNodes.resize(0);

		/*for (auto skin : skins) {
			delete skin;
		}
		skins.resize(0);*/
		VBO.release();
		EBO.release();
		VAO.release();
	};

	void Model::loadNode(Node *parent, const tinygltf::Node &node, uint32_t nodeIndex, const tinygltf::Model &model, std::vector<uint32_t> &indexBuffer, std::vector<Vertex> &vertexBuffer, float globalscale)
	{
		Node *newNode = new Node{};
		newNode->index = nodeIndex;
		newNode->parent = parent;
		newNode->name = node.name;
		newNode->skinIndex = node.skin;
		newNode->matrix = glm::mat4(1.0f);

		// Generate local node matrix
		glm::vec3 translation = glm::vec3(0.0f);
		if (node.translation.size() == 3)
		{
			translation = glm::make_vec3(node.translation.data());
			newNode->translation = translation;
		}
		glm::mat4 rotation = glm::mat4(1.0f);
		if (node.rotation.size() == 4)
		{
			glm::quat q = glm::make_quat(node.rotation.data());
			newNode->rotation = glm::mat4(q);
		}
		glm::vec3 scale = glm::vec3(1.0f);
		if (node.scale.size() == 3)
		{
			scale = glm::make_vec3(node.scale.data());
			newNode->scale = scale;
		}
		if (node.matrix.size() == 16)
		{
			newNode->matrix = glm::make_mat4x4(node.matrix.data());
		}

		// Node with children
		if (node.children.size() > 0)
		{
			for (size_t i = 0; i < node.children.size(); i++)
			{
				loadNode(newNode, model.nodes[node.children[i]], node.children[i], model, indexBuffer, vertexBuffer, globalscale);
			}
		}

		// Node contains mesh data
		if (node.mesh > -1)
		{
			const tinygltf::Mesh mesh = model.meshes[node.mesh];

			Mesh *newMesh = new Mesh(newNode->matrix);
			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				const tinygltf::Primitive &primitive = mesh.primitives[j];

				gl::Primitive mode = static_cast<gl::Primitive>(primitive.mode);
				uint32_t indexStart = static_cast<uint32_t>(indexBuffer.size());
				uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
				uint32_t indexCount = 0;
				uint32_t vertexCount = 0;
				uint32_t byteOffset;

				assert(primitive.attributes.find("POSITION") != primitive.attributes.end());
				assert(primitive.attributes.find("NORMAL") != primitive.attributes.end());
				assert(primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end());

				const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
				const tinygltf::BufferView &posView = model.bufferViews[posAccessor.bufferView];
				const tinygltf::Buffer &posBuffer = model.buffers[posView.buffer];

				vertexCount = static_cast<uint32_t>(posAccessor.count);
				const float *bufferPos = reinterpret_cast<const float *>(&posBuffer.data[posView.byteOffset + posAccessor.byteOffset]);
				const int posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);
				glm::vec3 posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
				glm::vec3 posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);

				const tinygltf::Accessor &normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
				const tinygltf::BufferView &normView = model.bufferViews[normAccessor.bufferView];
				const tinygltf::Buffer &normBuffer = model.buffers[normView.buffer];

				const float *bufferNorm = reinterpret_cast<const float *>(&normBuffer.data[normView.byteOffset + normAccessor.byteOffset]);
				const int normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);

				const tinygltf::Accessor &texCoordAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
				const tinygltf::BufferView &texCoordView = model.bufferViews[texCoordAccessor.bufferView];
				const tinygltf::Buffer &texCoordBuffer = model.buffers[texCoordView.buffer];

				const float *bufferTexCoord = reinterpret_cast<const float *>(&texCoordBuffer.data[texCoordView.byteOffset + texCoordAccessor.byteOffset]);
				const int texCoordByteStride = normAccessor.ByteStride(texCoordView) ? (texCoordAccessor.ByteStride(texCoordView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC2);

				// std::unique_ptr<gl::VertexBuffer> vertex_buffer = std::make_unique<gl::VertexBuffer>();
				// std::unique_ptr<gl::IndexBuffer> index_buffer = std::make_unique<gl::IndexBuffer>();

				for (size_t v = 0; v < posAccessor.count; v++)
				{
					Vertex vert{};
					vert.pos = glm::vec4(glm::make_vec3(&bufferPos[v * posByteStride]), 1.0f);
					vert.normal = glm::normalize(glm::vec3(bufferNorm ? glm::make_vec3(&bufferNorm[v * normByteStride]) : glm::vec3(0.0f)));
					vert.uv0 = bufferTexCoord ? glm::make_vec2(&bufferTexCoord[v * texCoordByteStride]) : glm::vec3(0.0f);

					vertexBuffer.push_back(vert);
				}

				/*vertex_buffer->Data(vertexCount, vertexBuffer.size() * sizeof(Vertex), vertexBuffer.data(), gl::Buffer::UsageMode::STATIC_DRAW);
				vertex_buffer->AttributesPattern({
						gl::VertexBuffer::AttribType::POSITION,
						gl::VertexBuffer::AttribType::NORMAL,
						gl::VertexBuffer::AttribType::UV_0,
					});*/

				if (primitive.indices > -1)
				{
					const tinygltf::Accessor &accessor = model.accessors[primitive.indices > -1 ? primitive.indices : 0];
					const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

					indexCount = static_cast<uint32_t>(accessor.count);
					const void *dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

					switch (accessor.componentType)
					{
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
					{
						const uint32_t *buf = static_cast<const uint32_t *>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++)
						{
							indexBuffer.push_back((uint32_t)(buf[index] + vertexStart));
						}
						// index_buffer->Data(sizeof(uint32_t) * indexBuffer.size(), indexBuffer.data(), DataType::UNSIGNED_INT, gl::Buffer::UsageMode::STATIC_DRAW);
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
					{
						const uint16_t *buf = static_cast<const uint16_t *>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++)
						{
							indexBuffer.push_back((uint16_t)(buf[index] + vertexStart));
						}
						// index_buffer->Data(sizeof(uint16_t) * indexBuffer.size(), indexBuffer.data(), DataType::UNSIGNED_SHORT, gl::Buffer::UsageMode::STATIC_DRAW);
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
					{
						const uint8_t *buf = static_cast<const uint8_t *>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++)
						{
							indexBuffer.push_back((uint8_t)(buf[index] + vertexStart));
						}
						// index_buffer->Data(sizeof(uint8_t)* indexBuffer.size(), indexBuffer.data(), DataType::UNSIGNED_BYTE, gl::Buffer::UsageMode::STATIC_DRAW);
						break;
					}
					default:
						std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
						// assert(false && "Index component type %d not supported!", accessor.componentType);
						return;
					}
				}

				Primitive *newPrimitive = new Primitive(mode, indexStart, indexCount, vertexCount, primitive.material > -1 ? materials[primitive.material] : materials.back());
				newPrimitive->setBoundingBox(posMin, posMax);
				newMesh->primitives.push_back(newPrimitive);
			}

			for (auto p : newMesh->primitives)
			{
				if (p->bb.valid && !newMesh->bb.valid)
				{
					newMesh->bb = p->bb;
					newMesh->bb.valid = true;
				}
				newMesh->bb.min = glm::min(newMesh->bb.min, p->bb.min);
				newMesh->bb.max = glm::max(newMesh->bb.max, p->bb.max);
			}
			newNode->mesh = newMesh;
		}
		if (parent)
		{
			parent->children.push_back(newNode);
		}
		else
		{
			nodes.push_back(newNode);
		}
	}

	void Model::loadTextureSamplers(tinygltf::Model &gltfModel)
	{
		for (tinygltf::Sampler smpl : gltfModel.samplers)
		{
			gl::Texture2D::Sampler sampler{};
			if (smpl.minFilter > -1)
				sampler.minFilter = static_cast<gl::Texture::FilterMode>(smpl.minFilter);
			else
				sampler.minFilter = gl::Texture::FilterMode::LINEAR;
			if (smpl.magFilter > -1)
				sampler.magFilter = static_cast<gl::Texture::FilterMode>(smpl.magFilter);
			else
				sampler.magFilter = gl::Texture::FilterMode::LINEAR;
			if (smpl.wrapS > -1)
				sampler.wrapModeS = static_cast<gl::Texture::WrapMode>(smpl.wrapS);
			else
				sampler.wrapModeS = gl::Texture::WrapMode::REPEAT;
			if (smpl.wrapT > -1)
				sampler.wrapModeT = static_cast<gl::Texture::WrapMode>(smpl.wrapT);
			else
				sampler.wrapModeT = gl::Texture::WrapMode::REPEAT;
			textureSamplers.push_back(sampler);
		}
	}

	void Model::loadTextures(tinygltf::Model &gltfModel)
	{
		for (tinygltf::Texture &tex : gltfModel.textures)
		{
			tinygltf::Image image = gltfModel.images[tex.source];
			gl::Texture2D::Sampler textureSampler;

			if (tex.sampler == -1)
			{
				// No sampler specified, use a default one
				textureSampler.magFilter = gl::Texture::FilterMode::LINEAR;
				textureSampler.minFilter = gl::Texture::FilterMode::LINEAR;
				textureSampler.wrapModeS = gl::Texture::WrapMode::REPEAT;
				textureSampler.wrapModeT = gl::Texture::WrapMode::REPEAT;
				textureSampler.generateMipmaps = true;
			}
			else
			{
				textureSampler = textureSamplers[tex.sampler];
			}
			Texture texture;
			texture.fromglTfImage(image, textureSampler);
			textures.push_back(std::move(texture));
		}
	}

	void Model::loadMaterials(tinygltf::Model &gltfModel)
	{
		for (tinygltf::Material &mat : gltfModel.materials)
		{
			Material material{};
			if (mat.values.find("baseColorTexture") != mat.values.end())
			{
				material.baseColorTexture = textures[mat.values["baseColorTexture"].TextureIndex()].texture.get();
				material.texCoordSets.baseColor = mat.values["baseColorTexture"].TextureTexCoord();
			}
			if (mat.values.find("metallicRoughnessTexture") != mat.values.end())
			{
				material.metallicRoughnessTexture = textures[mat.values["metallicRoughnessTexture"].TextureIndex()].texture.get();
				material.texCoordSets.metallicRoughness = mat.values["metallicRoughnessTexture"].TextureTexCoord();
			}
			if (mat.values.find("roughnessFactor") != mat.values.end())
			{
				material.roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
			}
			if (mat.values.find("metallicFactor") != mat.values.end())
			{
				material.metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
			}
			if (mat.values.find("baseColorFactor") != mat.values.end())
			{
				material.baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
			}
			if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end())
			{
				material.normalTexture = textures[mat.additionalValues["normalTexture"].TextureIndex()].texture.get();
				material.texCoordSets.normal = mat.additionalValues["normalTexture"].TextureTexCoord();
			}
			if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end())
			{
				material.emissiveTexture = textures[mat.additionalValues["emissiveTexture"].TextureIndex()].texture.get();
				material.texCoordSets.emissive = mat.additionalValues["emissiveTexture"].TextureTexCoord();
			}
			if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end())
			{
				material.occlusionTexture = textures[mat.additionalValues["occlusionTexture"].TextureIndex()].texture.get();
				material.texCoordSets.occlusion = mat.additionalValues["occlusionTexture"].TextureTexCoord();
			}
			if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end())
			{
				tinygltf::Parameter param = mat.additionalValues["alphaMode"];
				if (param.string_value == "BLEND")
				{
					material.alphaMode = Material::ALPHAMODE_BLEND;
				}
				if (param.string_value == "MASK")
				{
					material.alphaCutoff = 0.5f;
					material.alphaMode = Material::ALPHAMODE_MASK;
				}
			}
			if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end())
			{
				material.alphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
			}
			if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end())
			{
				material.emissiveFactor = glm::vec4(glm::make_vec3(mat.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);
				material.emissiveFactor = glm::vec4(0.0f);
			}

			// Extensions
			// @TODO: Find out if there is a nicer way of reading these properties with recent tinygltf headers
			if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end())
			{
				auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
				if (ext->second.Has("specularGlossinessTexture"))
				{
					auto index = ext->second.Get("specularGlossinessTexture").Get("index");
					material.extension.specularGlossinessTexture = textures[index.Get<int>()].texture.get();
					auto texCoordSet = ext->second.Get("specularGlossinessTexture").Get("texCoord");
					material.texCoordSets.specularGlossiness = texCoordSet.Get<int>();
					material.pbrWorkflows.specularGlossiness = true;
				}
				if (ext->second.Has("diffuseTexture"))
				{
					auto index = ext->second.Get("diffuseTexture").Get("index");
					material.extension.diffuseTexture = textures[index.Get<int>()].texture.get();
				}
				if (ext->second.Has("diffuseFactor"))
				{
					auto factor = ext->second.Get("diffuseFactor");
					for (uint32_t i = 0; i < factor.ArrayLen(); i++)
					{
						auto val = factor.Get(i);
						material.extension.diffuseFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
					}
				}
				if (ext->second.Has("specularFactor"))
				{
					auto factor = ext->second.Get("specularFactor");
					for (uint32_t i = 0; i < factor.ArrayLen(); i++)
					{
						auto val = factor.Get(i);
						material.extension.specularFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
					}
				}
			}

			materials.push_back(material);
		}
		// Push a default material at the end of the list for meshes with no material assigned
		materials.push_back(Material());
	}

	void Model::loadFromFile(std::string filename, float scale)
	{

#ifndef __EMSCRIPTEN__

		tinygltf::Model gltfModel;
		tinygltf::TinyGLTF gltfContext;
		std::string error;
		std::string warning;

		bool binary = false;
		size_t extpos = filename.rfind('.', filename.length());
		if (extpos != std::string::npos)
		{
			binary = (filename.substr(extpos + 1, filename.length() - extpos) == "glb");
		}

		bool fileLoaded = binary ? gltfContext.LoadBinaryFromFile(&gltfModel, &error, &warning, filename.c_str()) : gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, filename.c_str());

		std::vector<uint32_t> indexBuffer;
		std::vector<Vertex> vertexBuffer;

		if (fileLoaded)
		{
			loadTextureSamplers(gltfModel);
			loadTextures(gltfModel);
			loadMaterials(gltfModel);
			// TODO: scene handling with no default scene
			const tinygltf::Scene &scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
			for (size_t i = 0; i < scene.nodes.size(); i++)
			{
				const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
				loadNode(nullptr, node, scene.nodes[i], gltfModel, indexBuffer, vertexBuffer, scale);
			}

			VBO = std::make_unique<gl::VertexBuffer>();

			VBO->Data(vertexBuffer.size(), vertexBuffer.size() * sizeof(Vertex), vertexBuffer.data(), gl::Buffer::UsageMode::STATIC_DRAW);
			VBO->AttributesPattern({
				gl::VertexBuffer::AttribType::POSITION,
				gl::VertexBuffer::AttribType::NORMAL,
				gl::VertexBuffer::AttribType::UV_0,
			});

			EBO = std::make_unique<gl::IndexBuffer>();

			EBO->Data(sizeof(uint32_t) * indexBuffer.size(), indexBuffer.data(), DataType::UNSIGNED_INT, gl::Buffer::UsageMode::STATIC_DRAW);

			VAO = std::make_unique<gl::VertexArray>();

			VAO->LinkVBO(nullptr, VBO.get());
			VAO->LinkEBO(EBO.get());
		}
		else
		{
			// TODO: throw
			std::cerr << "Could not load gltf file: " << error << std::endl;
			return;
		}

		getSceneDimensions();
#else
		this->fileName.clear();
		this->fileName = filename;

		emscripten_async_wget_data(
			filename.c_str(), this, [](void *arg, void *data, int size)
			{
				tinygltf::Model gltfModel;
				tinygltf::TinyGLTF gltfContext;
				std::string error;
				std::string warning;

				Scene::Model* model = (Scene::Model*)arg;

				bool binary = false;
				size_t extpos = model->fileName.rfind('.', model->fileName.length());
				if (extpos != std::string::npos) {
					binary = (model->fileName.substr(extpos + 1, model->fileName.length() - extpos) == "glb");
				}

				bool fileLoaded = false;

				if(binary)
					fileLoaded = gltfContext.LoadBinaryFromMemory(&gltfModel, &error, &warning, (const unsigned char*)data, size);
				else
					fileLoaded = gltfContext.LoadASCIIFromString(&gltfModel, &error, &warning, (const char*)data, size, "");

			
				std::vector<uint32_t> indexBuffer;
				std::vector<Vertex> vertexBuffer;

				if (fileLoaded)
				{
					model->loadTextureSamplers(gltfModel);
					model->loadTextures(gltfModel);
					model->loadMaterials(gltfModel);
					// TODO: scene handling with no default scene
					const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
					for (size_t i = 0; i < scene.nodes.size(); i++) {
						const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
						model->loadNode(nullptr, node, scene.nodes[i], gltfModel, indexBuffer, vertexBuffer, 1.f);
					}


					model->VBO = std::make_unique<gl::VertexBuffer>();

					model->VBO->Data(vertexBuffer.size(), vertexBuffer.size() * sizeof(Vertex), vertexBuffer.data(), gl::Buffer::UsageMode::STATIC_DRAW);
					model->VBO->AttributesPattern({
								gl::VertexBuffer::AttribType::POSITION,
								gl::VertexBuffer::AttribType::NORMAL,
								gl::VertexBuffer::AttribType::UV_0,
						});

					model->EBO = std::make_unique<gl::IndexBuffer>();

					model->EBO->Data(sizeof(uint32_t) * indexBuffer.size(), indexBuffer.data(), DataType::UNSIGNED_INT, gl::Buffer::UsageMode::STATIC_DRAW);

					model->VAO = std::make_unique<gl::VertexArray>();

					model->VAO->LinkVBO(nullptr, model->VBO.get());
					model->VAO->LinkEBO(model->EBO.get());
				}
				else {
					// TODO: throw
					std::cerr << "Could not load gltf file: " << error << std::endl;
					return;
				}

				model->getSceneDimensions(); },
			[](void *arg)
			{
				printf("emscripten_async_wget_data Error \n");
			});
#endif
	}

	/*void Model::loadFromMemory(const uint32_t dataSize, const unsigned char* data, float scale = 1.0f)
	{
		tinygltf::Model gltfModel;
		tinygltf::TinyGLTF gltfContext;
		std::string error;
		std::string warning;

		bool binary = false;
		size_t extpos = filename.rfind('.', filename.length());
		if (extpos != std::string::npos) {
			binary = (filename.substr(extpos + 1, filename.length() - extpos) == "glb");
		}

		gltfContext.LoadBinaryFromMemory(&gltfModel, &error, &warning, data, dataSize);
		gltfContext.LoadASCIIFromString(&gltfModel, &error, &warning, (const char*)data, dataSize, "");

		bool fileLoaded = binary ? gltfContext.LoadBinaryFromFile(&gltfModel, &error, &warning, filename.c_str()) : gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, filename.c_str());

		std::vector<uint32_t> indexBuffer;
		std::vector<Vertex> vertexBuffer;

		if (fileLoaded)
		{
			loadTextureSamplers(gltfModel);
			loadTextures(gltfModel);
			loadMaterials(gltfModel);
			// TODO: scene handling with no default scene
			const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
			for (size_t i = 0; i < scene.nodes.size(); i++) {
				const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
				loadNode(nullptr, node, scene.nodes[i], gltfModel, indexBuffer, vertexBuffer, scale);
			}
		}
		else {
			// TODO: throw
			std::cerr << "Could not load gltf file: " << error << std::endl;
			return;
		}

		getSceneDimensions();
	}*/

	void Model::drawNode(gl::Program *program, Node *node)
	{
		if (node->mesh)
		{
			for (Primitive *primitive : node->mesh->primitives)
			{
				if (primitive->material.baseColorTexture)
					primitive->material.baseColorTexture->Bind();
				VAO->Bind();
				glDrawElements((GLenum)primitive->primitiveMode, primitive->indexCount, GL_UNSIGNED_INT, (char *)NULL + primitive->firstIndex * sizeof(GLuint));
				VAO->UnBind();
				// VAO->Draw(primitive->primitiveMode, primitive->indexCount, DataType::UNSIGNED_INT, BUFFER_OFFSET(primitive->firstIndex));
			}
		}
		for (auto &child : node->children)
		{
			drawNode(program, child);
		}
	}

	void Model::draw(gl::Program *program, const glm::mat4 &model)
	{
		program->Use();
		for (auto &node : nodes)
		{
			program->SetMatrix4(program->Uniform("model"), model * node->getMatrix());
			// program->SetMatrix4(program->Uniform("model"), model);
			drawNode(program, node);
		}
	}

	void Model::calculateBoundingBox(Node *node, Node *parent)
	{
		BoundingBox parentBvh = parent ? parent->bvh : BoundingBox(dimensions.min, dimensions.max);

		if (node->mesh)
		{
			if (node->mesh->bb.valid)
			{
				node->aabb = node->mesh->bb.getAABB(node->getMatrix());
				if (node->children.size() == 0)
				{
					node->bvh.min = node->aabb.min;
					node->bvh.max = node->aabb.max;
					node->bvh.valid = true;
				}
			}
		}

		parentBvh.min = glm::min(parentBvh.min, node->bvh.min);
		parentBvh.max = glm::min(parentBvh.max, node->bvh.max);

		for (auto &child : node->children)
		{
			calculateBoundingBox(child, node);
		}
	}

	void Model::getSceneDimensions()
	{
		// Calculate binary volume hierarchy for all nodes in the scene
		for (auto node : nodes)
		{
			calculateBoundingBox(node, nullptr);
		}

		dimensions.min = glm::vec3(FLT_MAX);
		dimensions.max = glm::vec3(-FLT_MAX);

		for (auto node : nodes)
		{
			if (node->bvh.valid)
			{
				dimensions.min = glm::min(dimensions.min, node->bvh.min);
				dimensions.max = glm::max(dimensions.max, node->bvh.max);
			}
		}

		// Calculate scene aabb
		aabb = glm::scale(glm::mat4(1.0f), glm::vec3(dimensions.max[0] - dimensions.min[0], dimensions.max[1] - dimensions.min[1], dimensions.max[2] - dimensions.min[2]));
		aabb[3][0] = dimensions.min[0];
		aabb[3][1] = dimensions.min[1];
		aabb[3][2] = dimensions.min[2];
	}

	Node *Model::findNode(Node *parent, uint32_t index)
	{
		Node *nodeFound = nullptr;
		if (parent->index == index)
		{
			return parent;
		}
		for (auto &child : parent->children)
		{
			nodeFound = findNode(child, index);
			if (nodeFound)
			{
				break;
			}
		}
		return nodeFound;
	}

	Node *Model::nodeFromIndex(uint32_t index)
	{
		Node *nodeFound = nullptr;
		for (auto &node : nodes)
		{
			nodeFound = findNode(node, index);
			if (nodeFound)
			{
				break;
			}
		}
		return nodeFound;
	}
}