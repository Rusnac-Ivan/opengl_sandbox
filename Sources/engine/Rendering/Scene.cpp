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
	Primitive::Primitive(std::vector<std::unique_ptr<gl::VertexBuffer>>& vertex_buffer, std::unique_ptr<gl::IndexBuffer>& index_buffer, std::unique_ptr<gl::VertexArray>& vao, Material& material) :
		indices(std::move(index_buffer)),
		VAO(std::move(vao)),
		material(material) 
	{
		for (auto& vbo : vertex_buffer)
			vertices.push_back(std::move(vbo));
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
			//std::map<uint32_t, Scene::Buffer> buffers;
			std::map<uint32_t, std::unique_ptr<gl::VertexBuffer>> vertexBuffers;
			std::map<uint32_t, std::unique_ptr<gl::IndexBuffer>> indexBuffers;

			for (size_t i = 0; i < model.bufferViews.size(); ++i)
			{
				const tinygltf::BufferView& bufferView = model.bufferViews[i];
				if (bufferView.target == 0) {  // TODO impl drawarrays
					std::cout << "WARN: bufferView.target is zero" << std::endl;
					continue;  // Unsupported bufferView.
							   /*
								 From spec2.0 readme:
								 https://github.com/KhronosGroup/glTF/tree/master/specification/2.0
										  ... drawArrays function should be used with a count equal to
								 the count            property of any of the accessors referenced by the
								 attributes            property            (they are all equal for a given
								 primitive).
							   */
				}

				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
				std::cout << "bufferview.target " << bufferView.target << std::endl;

				//GLuint vbo;
				//glGenBuffers(1, &vbo);

				if (bufferView.target == static_cast<int>(gl::Buffer::Type::ARRAY))
				{
					vertexBuffers[i] = std::make_unique<gl::VertexBuffer>();
					vertexBuffers[i]->Data(bufferView.byteLength, &buffer.data.at(0) + bufferView.byteOffset, gl::Buffer::UsageMode::STATIC_DRAW);
				}

				if (bufferView.target == static_cast<int>(gl::Buffer::Type::ELEMENT_ARRAY))
				{
					indexBuffers[i] = std::make_unique<gl::IndexBuffer>();
					indexBuffers[i]->Data(bufferView.byteLength, &buffer.data.at(0) + bufferView.byteOffset, gl::Buffer::UsageMode::STATIC_DRAW);

				}
				//glBindBuffer(bufferView.target, vbo);

				std::cout << "buffer.data.size = " << buffer.data.size()
					<< ", bufferview.byteOffset = " << bufferView.byteOffset
					<< std::endl;

				//glBufferData(bufferView.target, bufferView.byteLength, &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
			}

			for (size_t i = 0; i < mesh.primitives.size(); ++i)
			{
				tinygltf::Primitive primitive = mesh.primitives[i];
				tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

				std::map<uint32_t, gl::VertexBuffer::AttribType> attribTypes;
				for (auto& attrib : primitive.attributes)
				{
					tinygltf::Accessor accessor = model.accessors[attrib.second];
					int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
					//glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

					int size = 1;
					if (accessor.type != TINYGLTF_TYPE_SCALAR) {
						size = accessor.type;
					}

					int vaa = -1;
					if (attrib.first.compare("POSITION") == 0)
					{
						vaa = 0;
						attribTypes[accessor.bufferView] = gl::VertexBuffer::AttribType::POSITION;
					}
					if (attrib.first.compare("NORMAL") == 0)
					{
						vaa = 1;
						attribTypes[accessor.bufferView] = gl::VertexBuffer::AttribType::NORMAL;
					}
					if (attrib.first.compare("TEXCOORD_0") == 0)
					{
						vaa = 2;
						attribTypes[accessor.bufferView] = gl::VertexBuffer::AttribType::UV_0;
					}
					if (attrib.first.compare("TEXCOORD_1") == 0)
					{
						vaa = 3;
						attribTypes[accessor.bufferView] = gl::VertexBuffer::AttribType::UV_1;
					}
					if (attrib.first.compare("JOINTS_0") == 0)
					{
						vaa = 4;
						attribTypes[accessor.bufferView] = gl::VertexBuffer::AttribType::JOINT;
					}
					if (attrib.first.compare("WEIGHTS_0") == 0)
					{
						vaa = 4;
						attribTypes[accessor.bufferView] = gl::VertexBuffer::AttribType::WEIGHT;
					}
					if (vaa > -1) {
						//glEnableVertexAttribArray(vaa);
						//glVertexAttribPointer(vaa, size, accessor.componentType,
							//accessor.normalized ? GL_TRUE : GL_FALSE,
							//byteStride, BUFFER_OFFSET(accessor.byteOffset));
						std::cout << "vaa : " << attrib.first << std::endl;
					}
					else
						std::cout << "vaa missing: " << attrib.first << std::endl;
				}

				for (auto& vertex_buff : vertexBuffers)
				{
					std::vector<gl::VertexBuffer::AttribType> buffer_attrib_list;
					for (auto& arg_type : attribTypes)
					{
						buffer_attrib_list.push_back(attribTypes[vertex_buff.first]);
					}

					vertex_buff.second->AttributesPattern(buffer_attrib_list);
				}

				std::unique_ptr<gl::VertexArray> vao = std::make_unique<gl::VertexArray>();
				std::vector<std::unique_ptr<gl::VertexBuffer>> vertex_buffers;
				for (auto& vertex_buff : vertexBuffers)
				{
					vao->LinkVBO(nullptr, vertex_buff.second.get(), indexAccessor.count);
					vertex_buffers.push_back(std::move(vertex_buff.second));
				}

				if (indexBuffers.size() > 0)
				{
					assert(indexBuffers.size() == 1);
					vao->LinkEBO(indexBuffers.begin()->second.get());
				}
				const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];

				glm::vec3 posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
				glm::vec3 posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);

				Primitive* newPrimitive = new Primitive(vertex_buffers, indexBuffers.begin()->second, vao, primitive.material > -1 ? materials[primitive.material] : materials.back());
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