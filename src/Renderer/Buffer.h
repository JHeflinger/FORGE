#pragma once
#include "Core/Safety.h"

enum class ShaderDataTypes {
	NONE = 0,
	FLOAT, FLOAT2, FLOAT3, FLOAT4,
	MAT3, MAT4,
	INT, INT2, INT3, INT4,
	BOOL
};

static uint32_t ShaderDataTypeSize(ShaderDataTypes type) {
	switch (type) {
		case ShaderDataTypes::FLOAT:     return 4;
		case ShaderDataTypes::FLOAT2:    return 4 * 2;
		case ShaderDataTypes::FLOAT3:    return 4 * 3;
		case ShaderDataTypes::FLOAT4:    return 4 * 4;
		case ShaderDataTypes::MAT3:      return 3 * 3 * 4;
		case ShaderDataTypes::MAT4:      return 4 * 4 * 4;
		case ShaderDataTypes::INT:       return 4;
		case ShaderDataTypes::INT2:      return 4 * 2;
		case ShaderDataTypes::INT3:      return 4 * 3;
		case ShaderDataTypes::INT4:      return 4 * 4;
		case ShaderDataTypes::BOOL:      return 1;
		default: ASSERT(false, "Unknown shader datatype!");
		return 0;
	}
	return 0;
}

struct BufferElement {
	std::string Name;
	ShaderDataTypes Type;
	uint32_t Size;
	uint32_t Offset;
	bool Normalized;
	BufferElement() {}
	BufferElement(ShaderDataTypes type, const std::string name, bool normalized = false) 
		: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized) {}
	virtual ~BufferElement() = default;
	uint32_t GetComponentCount() const {
		switch (Type) {
			case ShaderDataTypes::FLOAT:     return 1;
			case ShaderDataTypes::FLOAT2:    return 2;
			case ShaderDataTypes::FLOAT3:    return 3;
			case ShaderDataTypes::FLOAT4:    return 4;
			case ShaderDataTypes::MAT3:      return 3 * 3;
			case ShaderDataTypes::MAT4:      return 4 * 4;
			case ShaderDataTypes::INT:       return 1;
			case ShaderDataTypes::INT2:      return 2;
			case ShaderDataTypes::INT3:      return 3;
			case ShaderDataTypes::INT4:      return 4;
			case ShaderDataTypes::BOOL:      return 1;
			default: ASSERT(false, "Unknown shader datatype!");
			return 0;
		}
		return 0;
	}
};

class BufferLayout {
public:
	BufferLayout() {}
	BufferLayout(const std::initializer_list<BufferElement>& elements)
		: m_Elements(elements) {
		CalculateOffsetsAndStride();
	}
	virtual ~BufferLayout() = default;
	inline uint32_t GetStride() const { return m_Stride; }
	inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }
	std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
	std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
	std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
	std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
private:
	void CalculateOffsetsAndStride() {
		uint32_t offset = 0;
		m_Stride = 0;
		for (auto& element : m_Elements) {
			element.Offset = offset;
			offset += element.Size;
			m_Stride += element.Size;
		}
	}
private:
	std::vector<BufferElement> m_Elements;
	uint32_t m_Stride = 0;
};

class VertexBuffer {
public:
	VertexBuffer(uint32_t size);
	VertexBuffer(float* vertices, uint32_t size);
	~VertexBuffer();
	void Bind() const;
	void Unbind() const;
	const BufferLayout& GetLayout() const { return m_Layout; }
	void SetLayout(const BufferLayout& layout) { m_Layout = layout; }
	void SetData(const void* data, uint32_t size);
private:
	uint32_t m_RendererID;
	BufferLayout m_Layout;
};

class IndexBuffer {
public:
	IndexBuffer(uint32_t* indices, uint32_t count);
	~IndexBuffer();
	void Bind() const;
	void Unbind() const;
	uint32_t GetCount() const { return m_Count; }
private:
	uint32_t m_RendererID;
	uint32_t m_Count;
};
