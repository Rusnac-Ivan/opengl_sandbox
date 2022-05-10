#include "Platform.h"

unsigned short DataSize(const DataType& dataType)
{
	switch (dataType)
	{
	case DataType::BYTE:
		return sizeof(char);
	case DataType::UNSIGNED_BYTE:
		return sizeof(unsigned char);
	case DataType::SHORT:
		return sizeof(short);
	case DataType::UNSIGNED_SHORT:
		return sizeof(unsigned short);
	case DataType::INT:
		return sizeof(int);
	case DataType::UNSIGNED_INT:
		return sizeof(unsigned int);
	case DataType::FLOAT:
		return sizeof(float);
	case DataType::DOUBLE:
		return sizeof(double);
	default:
	{
		assert(false && "Invalid Data Type!");
		return -1;
	}
	}
}