#pragma once
#include "Core/Log.h"

#define ENABLE_ASSERTS //TODO: remove this for the fully optimized build

#ifdef ENABLE_ASSERTS
	#define ASSERT(x, ...) { if (!(x)) {FATAL("Assertion Failed: {0}", __VA_ARGS__); exit(1); }}
#else
	#define ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

template<typename T>
using Scope = std::unique_ptr<T>;
template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args) {
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
}
