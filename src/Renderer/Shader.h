#pragma once
#include "Core/Safety.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader {
public:
	Shader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
	Shader(const std::string& filepath);
	Shader(const std::string& content, const std::string& name);
	~Shader();
	void Bind() const;
	void Unbind() const;
	const std::string& GetName() const { return m_Name; }
public:
	void SetInt(const std::string& name, int value);
	void SetIntArray(const std::string& name, int* values, uint32_t count);
	void SetMat4(const std::string& name, const glm::mat4& value);
	void SetFloat(const std::string& name, float value);
	void SetFloat3(const std::string& name, const glm::vec3& value);
	void SetFloat4(const std::string& name, const glm::vec4& value);
public:
	void UploadUniformInt(const std::string& name, int value);
	void UploadUniformIntArray(const std::string& name, int* values, uint32_t count);
	void UploadUniformFloat(const std::string& name, float value);
	void UploadUniformFloat2(const std::string& name, const glm::vec2& values);
	void UploadUniformFloat3(const std::string& name, const glm::vec3& values);
	void UploadUniformFloat4(const std::string& name, const glm::vec4& values);
	void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
	void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
private:
	std::string ReadFile(const std::string& filepath);
	std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
	void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
private:
	uint32_t m_RendererID;
	std::string m_Name;
};

class ShaderLibrary {
public:
	void Add(const Ref<Shader>& shader);
	void Add(const std::string& name, const Ref<Shader>& shader);
	Ref<Shader> Load(const std::string& filepath);
	Ref<Shader> Load(const std::string& name, const std::string& filepath);
	Ref<Shader> Get(const std::string& name);
	bool Exists(const std::string& name) const;
private:
	std::unordered_map<std::string, Ref<Shader>> m_Shaders;
};
