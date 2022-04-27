#ifndef _SCENE_H_
#define _SCENE_H_

#include <glm/glm.hpp>
#include<glm/gtc/quaternion.hpp>
#include <tiny_gltf.h>
#include <memory>
#include <GLObjects/Texture.h>
#include <GLObjects/VertexBuffer.h>
#include <GLObjects/IndexBuffer.h>
#include <GLObjects/VertexArray.h>
#include <GLObjects/Program.h>

namespace Scene
{
	struct Node;

	struct BoundingBox
	{
		glm::vec3 min;
		glm::vec3 max;
		bool valid = false;
		BoundingBox();
		BoundingBox(glm::vec3 min, glm::vec3 max);
		BoundingBox getAABB(glm::mat4 m);
	};

	struct Texture
	{
		std::unique_ptr<gl::Texture2D> texture;

		Texture() {}

		Texture(Texture& other) : texture(std::move(other.texture)) {}
		Texture(Texture&& other) : texture(std::move(other.texture)) {}
		Texture& operator=(Texture& other)
		{
			texture = std::move(other.texture);
		}

		//Texture(const Texture& other) = delete;
		// Load a texture from a glTF image (stored as vector of chars loaded via stb_image) and generate a full mip chaing for it
		void fromglTfImage(tinygltf::Image& gltfimage, gl::Texture2D::Sampler sampler);
	};

	struct Material 
	{
		enum AlphaMode { ALPHAMODE_OPAQUE, ALPHAMODE_MASK, ALPHAMODE_BLEND };
		AlphaMode alphaMode = ALPHAMODE_OPAQUE;
		float alphaCutoff = 1.0f;
		float metallicFactor = 1.0f;
		float roughnessFactor = 1.0f;
		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		glm::vec4 emissiveFactor = glm::vec4(1.0f);
		gl::Texture2D* baseColorTexture;
		gl::Texture2D* metallicRoughnessTexture;
		gl::Texture2D* normalTexture;
		gl::Texture2D* occlusionTexture;
		gl::Texture2D* emissiveTexture;
		struct TexCoordSets {
			uint8_t baseColor = 0;
			uint8_t metallicRoughness = 0;
			uint8_t specularGlossiness = 0;
			uint8_t normal = 0;
			uint8_t occlusion = 0;
			uint8_t emissive = 0;
		} texCoordSets;
		struct Extension {
			gl::Texture2D* specularGlossinessTexture;
			gl::Texture2D* diffuseTexture;
			glm::vec4 diffuseFactor = glm::vec4(1.0f);
			glm::vec3 specularFactor = glm::vec3(0.0f);
		} extension;
		struct PbrWorkflows {
			bool metallicRoughness = true;
			bool specularGlossiness = false;
		} pbrWorkflows;
	};

	struct Primitive {

		std::unique_ptr<gl::VertexBuffer> vertices;
		std::unique_ptr<gl::IndexBuffer> indices;
		std::unique_ptr<gl::VertexArray> VAO;

		gl::Primitive primitiveMode;

		Material& material;
		bool hasIndices;
		BoundingBox bb;
		Primitive(std::unique_ptr<gl::VertexBuffer>& vertex_buffer, std::unique_ptr<gl::IndexBuffer>& index_buffer, std::unique_ptr<gl::VertexArray>& vao, Material& material);
		void setBoundingBox(glm::vec3 min, glm::vec3 max);
	};

	struct Mesh {
		std::vector<Primitive*> primitives;
		BoundingBox bb;
		BoundingBox aabb;
		
		Mesh(glm::mat4 matrix);
		~Mesh();
		void setBoundingBox(glm::vec3 min, glm::vec3 max);
	};

	/*struct Skin {
		std::string name;
		Node* skeletonRoot = nullptr;
		std::vector<glm::mat4> inverseBindMatrices;
		std::vector<Node*> joints;
	};*/

	struct Node {
		Node* parent;
		uint32_t index;
		std::vector<Node*> children;
		glm::mat4 matrix;
		std::string name;
		Mesh* mesh;
		//Skin* skin;
		int32_t skinIndex = -1;
		glm::vec3 translation{};
		glm::vec3 scale{ 1.0f };
		glm::quat rotation{};
		BoundingBox bvh;
		BoundingBox aabb;
		glm::mat4 localMatrix();
		glm::mat4 getMatrix();
		void update();
		~Node();
	};

	struct Model {
		struct Vertex {
			glm::vec3 pos;
			glm::vec3 normal;
			glm::vec2 uv0;
			//glm::vec2 uv1;
			//glm::vec4 joint0;
			//glm::vec4 weight0;
		};

		/*struct Vertices {
			VkBuffer buffer = VK_NULL_HANDLE;
			VkDeviceMemory memory;
		} vertices;*/
		//gl::VertexBuffer vertices;
		/*struct Indices {
			int count;
			VkBuffer buffer = VK_NULL_HANDLE;
			VkDeviceMemory memory;
		} indices;*/
		//gl::IndexBuffer indices;

		//gl::VertexArray VAO;

		glm::mat4 aabb;

		std::vector<Node*> nodes;
		//std::vector<Node*> linearNodes;

		//std::vector<Skin*> skins;

		std::vector<Texture> textures;
		std::vector<gl::Texture2D::Sampler> textureSamplers;
		std::vector<Material> materials;
		//std::vector<Animation> animations;

		struct Dimensions {
			glm::vec3 min = glm::vec3(FLT_MAX);
			glm::vec3 max = glm::vec3(-FLT_MAX);
		} dimensions;

		void destroy();
		void loadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer, float globalscale);
		//void loadSkins(tinygltf::Model& gltfModel);
		void loadTextures(tinygltf::Model& gltfModel);
		//VkSamplerAddressMode getVkWrapMode(int32_t wrapMode);
		//VkFilter getVkFilterMode(int32_t filterMode);
		void loadTextureSamplers(tinygltf::Model& gltfModel);
		void loadMaterials(tinygltf::Model& gltfModel);
		//void loadAnimations(tinygltf::Model& gltfModel);
		void loadFromFile(std::string filename, float scale = 1.0f);
		void drawNode(gl::Program* program, Node* node);
		void draw(gl::Program* program);
		void calculateBoundingBox(Node* node, Node* parent);
		void getSceneDimensions();
		//void updateAnimation(uint32_t index, float time);
		Node* findNode(Node* parent, uint32_t index);
		Node* nodeFromIndex(uint32_t index);
	};


}

#endif