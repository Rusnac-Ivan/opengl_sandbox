#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "Scene.h"
#include <Core/Platform.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <map>


namespace tinygltf
{
	static inline int32_t GetTypeSizeInBytes(uint32_t ty) {
		if (ty == TINYGLTF_TYPE_SCALAR) {
			return 1;
		}
		else if (ty == TINYGLTF_TYPE_VEC2) {
			return 2;
		}
		else if (ty == TINYGLTF_TYPE_VEC3) {
			return 3;
		}
		else if (ty == TINYGLTF_TYPE_VEC4) {
			return 4;
		}
		else if (ty == TINYGLTF_TYPE_MAT2) {
			return 4;
		}
		else if (ty == TINYGLTF_TYPE_MAT3) {
			return 9;
		}
		else if (ty == TINYGLTF_TYPE_MAT4) {
			return 16;
		}
		else {
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

	void Texture::fromglTfImage(tinygltf::Image& gltfimage, gl::Texture2D::Sampler sampler)
	{
		unsigned char* buffer = nullptr;
		size_t bufferSize = 0;
		bool deleteBuffer = false;

		if (gltfimage.component == 3)
		{
			// Most devices don't support RGB only on Vulkan so convert if necessary
			// TODO: Check actual format support and transform only if required
			bufferSize = gltfimage.width * gltfimage.height * 4;
			buffer = new unsigned char[bufferSize];
			unsigned char* rgba = buffer;
			unsigned char* rgb = &gltfimage.image[0];
			for (int32_t i = 0; i < gltfimage.width * gltfimage.height; ++i) {
				for (int32_t j = 0; j < 3; ++j) {
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

	// Primitive
	Primitive::Primitive(std::unique_ptr<gl::VertexBuffer>& vertex_buffer, std::unique_ptr<gl::IndexBuffer>& index_buffer, std::unique_ptr<gl::VertexArray>& vao, Material& material) :
		vertices(std::move(vertex_buffer)),
		indices(std::move(index_buffer)),
		VAO(std::move(vao)),
		material(material) 
	{
		hasIndices = indices.get() != nullptr;
	};

	void Primitive::setBoundingBox(glm::vec3 min, glm::vec3 max) 
	{
		bb.min = min;
		bb.max = max;
		bb.valid = true;
	}

	// Mesh
	Mesh::Mesh(glm::mat4 matrix) 
	{
		
		
		
	};

	Mesh::~Mesh() 
	{
		for (Primitive* p : primitives)
			delete p;
	}

	void Mesh::setBoundingBox(glm::vec3 min, glm::vec3 max) 
	{
		bb.min = min;
		bb.max = max;
		bb.valid = true;
	}

	// Node
	glm::mat4 Node::localMatrix() {
		return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
	}

	glm::mat4 Node::getMatrix() {
		glm::mat4 m = localMatrix();
		Node* p = parent;
		while (p) {
			m = p->localMatrix() * m;
			p = p->parent;
		}
		return m;
	}

	void Node::update() 
	{
		if (mesh) {
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
			//memcpy(mesh->uniformBuffer.mapped, &m, sizeof(glm::mat4));
		}

		for (auto& child : children) {
			child->update();
		}
	}

	Node::~Node() {
		if (mesh) {
			delete mesh;
		}
		for (auto& child : children) {
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
		//materials.resize(0);

		nodes.resize(0);
		//linearNodes.resize(0);

		/*for (auto skin : skins) {
			delete skin;
		}
		skins.resize(0);*/
	};
	

	void Model::loadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer, float globalscale)
	{
		Node* newNode = new Node{};
		newNode->index = nodeIndex;
		newNode->parent = parent;
		newNode->name = node.name;
		newNode->skinIndex = node.skin;
		newNode->matrix = glm::mat4(1.0f);

		// Generate local node matrix
		glm::vec3 translation = glm::vec3(0.0f);
		if (node.translation.size() == 3) {
			translation = glm::make_vec3(node.translation.data());
			newNode->translation = translation;
		}
		glm::mat4 rotation = glm::mat4(1.0f);
		if (node.rotation.size() == 4) {
			glm::quat q = glm::make_quat(node.rotation.data());
			newNode->rotation = glm::mat4(q);
		}
		glm::vec3 scale = glm::vec3(1.0f);
		if (node.scale.size() == 3) {
			scale = glm::make_vec3(node.scale.data());
			newNode->scale = scale;
		}
		if (node.matrix.size() == 16) {
			newNode->matrix = glm::make_mat4x4(node.matrix.data());
		};

		// Node with children
		if (node.children.size() > 0) {
			for (size_t i = 0; i < node.children.size(); i++) {
				loadNode(newNode, model.nodes[node.children[i]], node.children[i], model, indexBuffer, vertexBuffer, globalscale);
			}
		}

		// Node contains mesh data
		if (node.mesh > -1)
		{
			const tinygltf::Mesh mesh = model.meshes[node.mesh];

			Mesh* newMesh = new Mesh(newNode->matrix);
			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				const tinygltf::Primitive& primitive = mesh.primitives[j];

				assert(primitive.attributes.find("POSITION") != primitive.attributes.end());
				assert(primitive.attributes.find("NORMAL") != primitive.attributes.end());
				assert(primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end());

				const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
				const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
				const tinygltf::Buffer& posBuffer = model.buffers[posView.buffer];

				const float* bufferPos = reinterpret_cast<const float*>(&posBuffer.data[posView.byteOffset + posAccessor.byteOffset]);
				const int posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);
				glm::vec3 posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
				glm::vec3 posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);

				const tinygltf::Accessor& normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
				const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
				const tinygltf::Buffer& normBuffer = model.buffers[normView.buffer];

				const float* bufferNorm = reinterpret_cast<const float*>(&normBuffer.data[normView.byteOffset + normAccessor.byteOffset]);
				const int normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);

				const tinygltf::Accessor& texCoordAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
				const tinygltf::BufferView& texCoordView = model.bufferViews[texCoordAccessor.bufferView];
				const tinygltf::Buffer& texCoordBuffer = model.buffers[texCoordView.buffer];

				const float* bufferTexCoord = reinterpret_cast<const float*>(&texCoordBuffer.data[texCoordView.byteOffset + texCoordAccessor.byteOffset]);
				const int texCoordByteStride = normAccessor.ByteStride(texCoordView) ? (texCoordAccessor.ByteStride(texCoordView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);


				std::unique_ptr<gl::VertexBuffer> vertex_buffer = std::make_unique<gl::VertexBuffer>();
				std::unique_ptr<gl::IndexBuffer> index_buffer = std::make_unique<gl::IndexBuffer>();;

				for (size_t v = 0; v < posAccessor.count; v++) 
				{
					Vertex vert{};
					vert.pos = glm::vec4(glm::make_vec3(&bufferPos[v * posByteStride]), 1.0f);
					vert.normal = glm::normalize(glm::vec3(bufferNorm ? glm::make_vec3(&bufferNorm[v * normByteStride]) : glm::vec3(0.0f)));
					vert.uv0 = bufferTexCoord ? glm::make_vec2(&bufferTexCoord[v * texCoordByteStride]) : glm::vec3(0.0f);
					int a = 0;
					//vert.uv0 = bufferTexCoordSet0 ? glm::make_vec2(&bufferTexCoordSet0[v * uv0ByteStride]) : glm::vec3(0.0f);

					vertexBuffer.push_back(vert);
				}

				vertex_buffer->Data(vertexBuffer.size(), vertexBuffer.data(), gl::Buffer::UsageMode::STATIC_DRAW);


				if (primitive.indices > -1)
				{
					const tinygltf::Accessor& accessor = model.accessors[primitive.indices > -1 ? primitive.indices : 0];
					const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

					const float* bufferIndex = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

					//index_buffer->Data(accessor.count, )
				}
			}
			
			//Mesh* newMesh = new Mesh(newNode->matrix);
			for (size_t j = 0; j < mesh.primitives.size(); j++) 
			{
				const tinygltf::Primitive& primitive = mesh.primitives[j];
				uint32_t indexStart = static_cast<uint32_t>(indexBuffer.size());
				uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
				uint32_t indexCount = 0;
				uint32_t vertexCount = 0;
				glm::vec3 posMin{};
				glm::vec3 posMax{};
				bool hasSkin = false;
				bool hasIndices = primitive.indices > -1;

				std::unique_ptr<gl::VertexBuffer> vertex_buffer;
				std::unique_ptr<gl::IndexBuffer> index_buffer;
				//std::unique_ptr<gl::VertexBuffer> normals_buffer;
				//std::unique_ptr<gl::VertexBuffer> texture_coord_buffer;

				// Vertices
				{
					const float* bufferPos = nullptr;
					const float* bufferNormals = nullptr;
					const float* bufferTexCoordSet0 = nullptr;
					const float* bufferTexCoordSet1 = nullptr;
					const void* bufferJoints = nullptr;
					const float* bufferWeights = nullptr;

					int posByteStride;
					int normByteStride;
					int uv0ByteStride;
					int uv1ByteStride;
					int jointByteStride;
					int weightByteStride;

					int jointComponentType;

					// Position attribute is required
					assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

					const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
					const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
					bufferPos = reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
					posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
					posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
					vertexCount = static_cast<uint32_t>(posAccessor.count);
					posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);

					//position_buffer = std::make_unique<gl::VertexBuffer>();
					//position_buffer->Data(posView.byteLength, bufferPos, gl::Buffer::UsageMode::STATIC_DRAW);
					//position_buffer->AttributesPattern({ gl::VertexBuffer::AttribType::POSITION });

					if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
						const tinygltf::Accessor& normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
						bufferNormals = reinterpret_cast<const float*>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
						normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC3);

						//normals_buffer = std::make_unique<gl::VertexBuffer>();
						//normals_buffer->Data(normView.byteLength, bufferNormals, gl::Buffer::UsageMode::STATIC_DRAW);
						//normals_buffer->AttributesPattern({ gl::VertexBuffer::AttribType::NORMAL });
					}

					if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
						bufferTexCoordSet0 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
						uv0ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC2);

						//texture_coord_buffer = std::make_unique<gl::VertexBuffer>();
						//texture_coord_buffer->Data(uvView.byteLength, bufferTexCoordSet0, gl::Buffer::UsageMode::STATIC_DRAW);
						//texture_coord_buffer->AttributesPattern({ gl::VertexBuffer::AttribType::UV_0 });
					}
					if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end()) {
						const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
						const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
						bufferTexCoordSet1 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
						uv1ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC2);
					}

					// Skinning
					// Joints
					if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& jointAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
						const tinygltf::BufferView& jointView = model.bufferViews[jointAccessor.bufferView];
						bufferJoints = &(model.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]);
						jointComponentType = jointAccessor.componentType;
						jointByteStride = jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) / tinygltf::GetComponentSizeInBytes(jointComponentType)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC4);
					}

					if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& weightAccessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
						const tinygltf::BufferView& weightView = model.bufferViews[weightAccessor.bufferView];
						bufferWeights = reinterpret_cast<const float*>(&(model.buffers[weightView.buffer].data[weightAccessor.byteOffset + weightView.byteOffset]));
						weightByteStride = weightAccessor.ByteStride(weightView) ? (weightAccessor.ByteStride(weightView) / sizeof(float)) : tinygltf::GetTypeSizeInBytes(TINYGLTF_TYPE_VEC4);
					}

					hasSkin = (bufferJoints && bufferWeights);

					for (size_t v = 0; v < posAccessor.count; v++) {
						Vertex vert{};
						vert.pos = glm::vec4(glm::make_vec3(&bufferPos[v * posByteStride]), 1.0f);
						vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * normByteStride]) : glm::vec3(0.0f)));
						vert.uv0 = bufferTexCoordSet0 ? glm::make_vec2(&bufferTexCoordSet0[v * uv0ByteStride]) : glm::vec3(0.0f);
						//vert.uv1 = bufferTexCoordSet1 ? glm::make_vec2(&bufferTexCoordSet1[v * uv1ByteStride]) : glm::vec3(0.0f);

						/*if (hasSkin)
						{
							switch (jointComponentType) {
							case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
								const uint16_t* buf = static_cast<const uint16_t*>(bufferJoints);
								vert.joint0 = glm::vec4(glm::make_vec4(&buf[v * jointByteStride]));
								break;
							}
							case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
								const uint8_t* buf = static_cast<const uint8_t*>(bufferJoints);
								vert.joint0 = glm::vec4(glm::make_vec4(&buf[v * jointByteStride]));
								break;
							}
							default:
								// Not supported by spec
								std::cerr << "Joint component type " << jointComponentType << " not supported!" << std::endl;
								break;
							}
						}
						else {
							vert.joint0 = glm::vec4(0.0f);
						}
						vert.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * weightByteStride]) : glm::vec4(0.0f);
						// Fix for all zero weights
						if (glm::length(vert.weight0) == 0.0f) {
							vert.weight0 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
						}*/
						vertexBuffer.push_back(vert);
					}


					vertex_buffer = std::make_unique<gl::VertexBuffer>();
					vertex_buffer->Data(sizeof(vertexBuffer), vertexBuffer.data(), gl::Buffer::UsageMode::STATIC_DRAW);
					vertex_buffer->AttributesPattern({ 
						gl::VertexBuffer::AttribType::POSITION, 
						gl::VertexBuffer::AttribType::NORMAL, 
						gl::VertexBuffer::AttribType::UV_0,
						gl::VertexBuffer::AttribType::UV_1,
						gl::VertexBuffer::AttribType::JOINT,
						gl::VertexBuffer::AttribType::WEIGHT
					});
				}
				// Indices
				if (hasIndices)
				{
					const tinygltf::Accessor& accessor = model.accessors[primitive.indices > -1 ? primitive.indices : 0];
					const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

					indexCount = static_cast<uint32_t>(accessor.count);
					const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);



					switch (accessor.componentType) {
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
						const uint32_t* buf = static_cast<const uint32_t*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++) {
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
						const uint16_t* buf = static_cast<const uint16_t*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++) {
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
						const uint8_t* buf = static_cast<const uint8_t*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++) {
							indexBuffer.push_back(buf[index] + vertexStart);
						}
						break;
					}
					default:
						std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
						return;
					}

					index_buffer = std::make_unique<gl::IndexBuffer>();
					index_buffer->Data(sizeof(indexBuffer), indexBuffer.data(), gl::Buffer::UsageMode::STATIC_DRAW);
					//vertexCount = indexBuffer.size();
				}
				std::unique_ptr<gl::VertexArray> vao = std::make_unique<gl::VertexArray>();
				vao->LinkVBO(nullptr, vertex_buffer.get(), vertexCount);
				vao->LinkEBO(index_buffer.get());

				Primitive* newPrimitive = new Primitive(vertex_buffer, index_buffer, vao, primitive.material > -1 ? materials[primitive.material] : materials.back());
				newPrimitive->primitiveMode = static_cast<gl::Primitive>(primitive.mode);
				newPrimitive->setBoundingBox(posMin, posMax);
				newMesh->primitives.push_back(newPrimitive);
			}



			for (auto p : newMesh->primitives) {
				if (p->bb.valid && !newMesh->bb.valid) {
					newMesh->bb = p->bb;
					newMesh->bb.valid = true;
				}
				newMesh->bb.min = glm::min(newMesh->bb.min, p->bb.min);
				newMesh->bb.max = glm::max(newMesh->bb.max, p->bb.max);
			}
			newNode->mesh = newMesh;

			if (parent) {
				parent->children.push_back(newNode);
			}
			else {
				nodes.push_back(newNode);
			}
		}
	}


	

	void Model::loadTextureSamplers(tinygltf::Model& gltfModel)
	{
		for (tinygltf::Sampler smpl : gltfModel.samplers) {
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

	void Model::loadTextures(tinygltf::Model& gltfModel)
	{
		for (tinygltf::Texture& tex : gltfModel.textures) {
			tinygltf::Image image = gltfModel.images[tex.source];
			gl::Texture2D::Sampler textureSampler;

			if (tex.sampler == -1) {
				// No sampler specified, use a default one
				textureSampler.magFilter = gl::Texture::FilterMode::LINEAR;
				textureSampler.minFilter = gl::Texture::FilterMode::LINEAR;
				textureSampler.wrapModeS = gl::Texture::WrapMode::REPEAT;
				textureSampler.wrapModeT = gl::Texture::WrapMode::REPEAT;
				textureSampler.generateMipmaps = true;
			}
			else {
				textureSampler = textureSamplers[tex.sampler];
			}
			Texture texture;
			texture.fromglTfImage(image, textureSampler);
			textures.push_back(std::move(texture));
		}
	}

	void Model::loadMaterials(tinygltf::Model& gltfModel)
	{
		for (tinygltf::Material& mat : gltfModel.materials) {
			Material material{};
			if (mat.values.find("baseColorTexture") != mat.values.end()) {
				material.baseColorTexture = textures[mat.values["baseColorTexture"].TextureIndex()].texture.get();
				material.texCoordSets.baseColor = mat.values["baseColorTexture"].TextureTexCoord();
			}
			if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
				material.metallicRoughnessTexture = textures[mat.values["metallicRoughnessTexture"].TextureIndex()].texture.get();
				material.texCoordSets.metallicRoughness = mat.values["metallicRoughnessTexture"].TextureTexCoord();
			}
			if (mat.values.find("roughnessFactor") != mat.values.end()) {
				material.roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
			}
			if (mat.values.find("metallicFactor") != mat.values.end()) {
				material.metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
			}
			if (mat.values.find("baseColorFactor") != mat.values.end()) {
				material.baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
			}
			if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
				material.normalTexture = textures[mat.additionalValues["normalTexture"].TextureIndex()].texture.get();
				material.texCoordSets.normal = mat.additionalValues["normalTexture"].TextureTexCoord();
			}
			if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
				material.emissiveTexture = textures[mat.additionalValues["emissiveTexture"].TextureIndex()].texture.get();
				material.texCoordSets.emissive = mat.additionalValues["emissiveTexture"].TextureTexCoord();
			}
			if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
				material.occlusionTexture = textures[mat.additionalValues["occlusionTexture"].TextureIndex()].texture.get();
				material.texCoordSets.occlusion = mat.additionalValues["occlusionTexture"].TextureTexCoord();
			}
			if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) {
				tinygltf::Parameter param = mat.additionalValues["alphaMode"];
				if (param.string_value == "BLEND") {
					material.alphaMode = Material::ALPHAMODE_BLEND;
				}
				if (param.string_value == "MASK") {
					material.alphaCutoff = 0.5f;
					material.alphaMode = Material::ALPHAMODE_MASK;
				}
			}
			if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) {
				material.alphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
			}
			if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end()) {
				material.emissiveFactor = glm::vec4(glm::make_vec3(mat.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);
				material.emissiveFactor = glm::vec4(0.0f);
			}

			// Extensions
			// @TODO: Find out if there is a nicer way of reading these properties with recent tinygltf headers
			if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end()) {
				auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
				if (ext->second.Has("specularGlossinessTexture")) {
					auto index = ext->second.Get("specularGlossinessTexture").Get("index");
					material.extension.specularGlossinessTexture = textures[index.Get<int>()].texture.get();
					auto texCoordSet = ext->second.Get("specularGlossinessTexture").Get("texCoord");
					material.texCoordSets.specularGlossiness = texCoordSet.Get<int>();
					material.pbrWorkflows.specularGlossiness = true;
				}
				if (ext->second.Has("diffuseTexture")) {
					auto index = ext->second.Get("diffuseTexture").Get("index");
					material.extension.diffuseTexture = textures[index.Get<int>()].texture.get();
				}
				if (ext->second.Has("diffuseFactor")) {
					auto factor = ext->second.Get("diffuseFactor");
					for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
						auto val = factor.Get(i);
						material.extension.diffuseFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
					}
				}
				if (ext->second.Has("specularFactor")) {
					auto factor = ext->second.Get("specularFactor");
					for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
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
		tinygltf::Model gltfModel;
		tinygltf::TinyGLTF gltfContext;
		std::string error;
		std::string warning;

		bool binary = false;
		size_t extpos = filename.rfind('.', filename.length());
		if (extpos != std::string::npos) {
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
			const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
			for (size_t i = 0; i < scene.nodes.size(); i++) {
				const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
				loadNode(nullptr, node, scene.nodes[i], gltfModel, indexBuffer, vertexBuffer, scale);
			}
			/*if (gltfModel.animations.size() > 0) {
				loadAnimations(gltfModel);
			}*/
			//loadSkins(gltfModel);

			/*for (auto node : linearNodes) {
				// Assign skins
				if (node->skinIndex > -1) {
					node->skin = skins[node->skinIndex];
				}
				// Initial pose
				if (node->mesh) {
					node->update();
				}
			}*/
		}
		else {
			// TODO: throw
			std::cerr << "Could not load gltf file: " << error << std::endl;
			return;
		}

		//extensions = gltfModel.extensionsUsed;

		//size_t vertexBufferSize = vertexBuffer.size() * sizeof(Vertex);
		//size_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);

		/*vertices.Data(vertexBufferSize, vertexBuffer.data(), gl::Buffer::UsageMode::STATIC_DRAW);
		vertices.AttributesPattern({
			gl::VertexBuffer::AttribType::POSITION, 
			gl::VertexBuffer::AttribType::NORMAL, 
			gl::VertexBuffer::AttribType::UV_0, 
			gl::VertexBuffer::AttribType::UV_1, 
			gl::VertexBuffer::AttribType::JOINT,
			gl::VertexBuffer::AttribType::WEIGHT
		});

		indices.Data(indexBufferSize, indexBuffer.data(), gl::Buffer::UsageMode::STATIC_DRAW);

		//VAO.LinkVBO()*/


		//assert(vertexBufferSize > 0);

		getSceneDimensions();
	}


	void Model::drawNode(Node* node)
	{
		if (node->mesh) {
			for (Primitive* primitive : node->mesh->primitives) 
			{
				primitive->material.baseColorTexture->Bind();
				//vkCmdDrawIndexed(commandBuffer, primitive->indexCount, 1, primitive->firstIndex, 0, 0);
				primitive->VAO->Draw(primitive->primitiveMode);
			}
		}
		for (auto& child : node->children) {
			drawNode(child);
		}
	}

	void Model::draw()
	{
		for (auto& node : nodes) {
			drawNode(node);
		}
	}

	void Model::calculateBoundingBox(Node* node, Node* parent) {
		BoundingBox parentBvh = parent ? parent->bvh : BoundingBox(dimensions.min, dimensions.max);

		if (node->mesh) {
			if (node->mesh->bb.valid) {
				node->aabb = node->mesh->bb.getAABB(node->getMatrix());
				if (node->children.size() == 0) {
					node->bvh.min = node->aabb.min;
					node->bvh.max = node->aabb.max;
					node->bvh.valid = true;
				}
			}
		}

		parentBvh.min = glm::min(parentBvh.min, node->bvh.min);
		parentBvh.max = glm::min(parentBvh.max, node->bvh.max);

		for (auto& child : node->children) {
			calculateBoundingBox(child, node);
		}
	}

	void Model::getSceneDimensions()
	{
		// Calculate binary volume hierarchy for all nodes in the scene
		for (auto node : nodes) {
			calculateBoundingBox(node, nullptr);
		}

		dimensions.min = glm::vec3(FLT_MAX);
		dimensions.max = glm::vec3(-FLT_MAX);

		for (auto node : nodes) {
			if (node->bvh.valid) {
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

	Node* Model::findNode(Node* parent, uint32_t index) {
		Node* nodeFound = nullptr;
		if (parent->index == index) {
			return parent;
		}
		for (auto& child : parent->children) {
			nodeFound = findNode(child, index);
			if (nodeFound) {
				break;
			}
		}
		return nodeFound;
	}

	Node* Model::nodeFromIndex(uint32_t index) {
		Node* nodeFound = nullptr;
		for (auto& node : nodes) {
			nodeFound = findNode(node, index);
			if (nodeFound) {
				break;
			}
		}
		return nodeFound;
	}
}