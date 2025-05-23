#pragma once
#include "yaml-cpp/yaml.h"
#include "glm/glm.hpp"

template<>
struct YAML::convert<glm::vec2> {
	static Node encode(const glm::vec2& rhs) {
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		return node;
	}

	static bool decode(const Node& node, glm::vec2& rhs) {
		if (!node.IsSequence() || node.size() != 2)
			return false;
		rhs.x = node[0].as<float>();
		rhs.y = node[1].as<float>();
		return true;
	}
};

template<>
struct YAML::convert<glm::vec3> {
	static Node encode(const glm::vec3& rhs) {
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		node.push_back(rhs.z);
		return node;
	}

	static bool decode(const Node& node, glm::vec3& rhs) {
		if (!node.IsSequence() || node.size() != 3)
			return false;
		rhs.x = node[0].as<float>();
		rhs.y = node[1].as<float>();
		rhs.z = node[2].as<float>();
		return true;
	}
};

template<>
struct YAML::convert<glm::vec4> {
	static Node encode(const glm::vec4& rhs) {
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		node.push_back(rhs.z);
		node.push_back(rhs.w);
		return node;
	}

	static bool decode(const Node& node, glm::vec4& rhs) {
		if (!node.IsSequence() || node.size() != 4)
			return false;
		rhs.x = node[0].as<float>();
		rhs.y = node[1].as<float>();
		rhs.z = node[2].as<float>();
		rhs.w = node[3].as<float>();
		return true;
	}
};

#ifdef __linux__
[[maybe_unused]]
#endif
static YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v) {
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

#ifdef __linux__
[[maybe_unused]]
#endif
static YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v) {
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

#ifdef __linux__
[[maybe_unused]]
#endif
static YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v) {
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
	return out;
}

template<>
struct YAML::convert<glm::dvec2> {
	static Node encode(const glm::dvec2& rhs) {
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		return node;
	}

	static bool decode(const Node& node, glm::dvec2& rhs) {
		if (!node.IsSequence() || node.size() != 2)
			return false;
		rhs.x = node[0].as<double>();
		rhs.y = node[1].as<double>();
		return true;
	}
};

template<>
struct YAML::convert<glm::dvec3> {
	static Node encode(const glm::dvec3& rhs) {
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		node.push_back(rhs.z);
		return node;
	}

	static bool decode(const Node& node, glm::dvec3& rhs) {
		if (!node.IsSequence() || node.size() != 3)
			return false;
		rhs.x = node[0].as<double>();
		rhs.y = node[1].as<double>();
		rhs.z = node[2].as<double>();
		return true;
	}
};

template<>
struct YAML::convert<glm::dvec4> {
	static Node encode(const glm::dvec4& rhs) {
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		node.push_back(rhs.z);
		node.push_back(rhs.w);
		return node;
	}

	static bool decode(const Node& node, glm::dvec4& rhs) {
		if (!node.IsSequence() || node.size() != 4)
			return false;
		rhs.x = node[0].as<double>();
		rhs.y = node[1].as<double>();
		rhs.z = node[2].as<double>();
		rhs.w = node[3].as<double>();
		return true;
	}
};

#ifdef __linux__
[[maybe_unused]]
#endif
static YAML::Emitter& operator<<(YAML::Emitter& out, const glm::dvec2& v) {
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

#ifdef __linux__
[[maybe_unused]]
#endif
static YAML::Emitter& operator<<(YAML::Emitter& out, const glm::dvec3& v) {
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

#ifdef __linux__
[[maybe_unused]]
#endif
static YAML::Emitter& operator<<(YAML::Emitter& out, const glm::dvec4& v) {
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
	return out;
}
