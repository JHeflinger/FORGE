#include "VertexArray.h"
#include <glad/glad.h>

static GLenum ConvertShaderDataType(ShaderDataTypes type) {
	switch (type) {
		case ShaderDataTypes::FLOAT:   return GL_FLOAT;
		case ShaderDataTypes::FLOAT2:  return GL_FLOAT;
		case ShaderDataTypes::FLOAT3:  return GL_FLOAT;
		case ShaderDataTypes::FLOAT4:  return GL_FLOAT;
		case ShaderDataTypes::MAT3:    return GL_FLOAT;
		case ShaderDataTypes::MAT4:    return GL_FLOAT;
		case ShaderDataTypes::INT:     return GL_INT;
		case ShaderDataTypes::INT2:    return GL_INT;
		case ShaderDataTypes::INT3:    return GL_INT;
		case ShaderDataTypes::INT4:    return GL_INT;
		case ShaderDataTypes::BOOL:    return GL_BOOL;
		default: ASSERT(false, "Unknown shader data type!");
	}
	return 0;
}

VertexArray::VertexArray() {
	glCreateVertexArrays(1, &m_RendererID);
}

VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &m_RendererID);
}

void VertexArray::Bind() const {
	glBindVertexArray(m_RendererID);
}

void VertexArray::Unbind() const {
	glBindVertexArray(0);
}

void VertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) {
	ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex buffer has no layout!");
	glBindVertexArray(m_RendererID);
	vertexBuffer->Bind();
	const auto& layout = vertexBuffer->GetLayout();
	for (const auto& element : layout) {
		switch (element.Type) {
			case ShaderDataTypes::FLOAT:
			case ShaderDataTypes::FLOAT2:
			case ShaderDataTypes::FLOAT3:
			case ShaderDataTypes::FLOAT4: {
				glEnableVertexAttribArray(m_VertexBufferIndex);
				glVertexAttribPointer(m_VertexBufferIndex,
									  element.GetComponentCount(),
									  ConvertShaderDataType(element.Type),
									  element.Normalized ? GL_TRUE : GL_FALSE,
									  layout.GetStride(),
									  reinterpret_cast<const void*>(element.Offset));
				m_VertexBufferIndex++;
				break;
			}
			case ShaderDataTypes::INT:
			case ShaderDataTypes::INT2:
			case ShaderDataTypes::INT3:
			case ShaderDataTypes::INT4:
			case ShaderDataTypes::BOOL: {
				glEnableVertexAttribArray(m_VertexBufferIndex);
				glVertexAttribIPointer(m_VertexBufferIndex,
									   element.GetComponentCount(),
									   ConvertShaderDataType(element.Type),
									   layout.GetStride(),
									   reinterpret_cast<const void*>(element.Offset));
				m_VertexBufferIndex++;
				break;
			}
			case ShaderDataTypes::MAT3:
			case ShaderDataTypes::MAT4: {
				uint8_t count = element.GetComponentCount();
				for (uint8_t i = 0; i < count; i++) {
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribPointer(m_VertexBufferIndex,
										  count,
										  ConvertShaderDataType(element.Type),
										  element.Normalized ? GL_TRUE : GL_FALSE,
										  layout.GetStride(),
										  (const void*)(element.Offset + sizeof(float) * count * i));
										  glVertexAttribDivisor(m_VertexBufferIndex, 1);
					m_VertexBufferIndex++;
				}
				break;
			}
			default: ASSERT(false, "Unknown shader type!");
		}
	}
	m_VertexBuffers.push_back(vertexBuffer);
}

void VertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) {
	glBindVertexArray(m_RendererID);
	indexBuffer->Bind();
	m_IndexBuffer = indexBuffer;
}
