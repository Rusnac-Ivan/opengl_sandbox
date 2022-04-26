#ifndef _GL_VERTEX_BUFFER_H_
#define _GL_VERTEX_BUFFER_H_

#include <initializer_list>
#include <string>
#include <vector>
#include <GLObjects/Buffer.h>
#include <cassert>

namespace gl
{
	
	class VertexBuffer : public Buffer
	{
		public:
			enum struct AttribType
			{
				POSITION,
				NORMAL,
				UV_0,
				UV_1,
				JOINT,
				WEIGHT,
				COLOR,
				TANGENT,
				BITANGENT
			};
		
			struct Attribute
			{
				DataType dataType;
				uint16_t numOfComponents;
				uint16_t offset;
				uint16_t location;
				std::string name;
			};
		private:
			std::vector<Attribute> mAttributes;
			uint32_t mStrideSize;
			uint32_t mVertexCount;
			
		public:
			VertexBuffer();
			~VertexBuffer();
			
			uint32_t GetStrideSize() {return mStrideSize;}

			void SetVertexCount(uint32_t count) { mVertexCount = count; }
			uint32_t GetVertexCount() { return  mVertexCount; }

			void Bind();
			
			void AttributesPattern(std::initializer_list<AttribType> attributes);
			void AttributesPattern(std::vector<AttribType> attributes);
			void Data(unsigned int size, const void* data, UsageMode accessFreq);
			void SubData(unsigned short offset, unsigned short size, void* data);
			std::vector<Attribute>& GetAttributes() { return mAttributes; }
			
		private:
			Attribute MakeAttribute(AttribType type);
	};
}
#endif
