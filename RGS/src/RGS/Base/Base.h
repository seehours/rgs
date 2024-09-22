#pragma once

#ifdef RGS_BUILD_RELEASE
	#define ASSERT(x, ...)
	#define BREAKIF(x)
#endif

#ifdef RGS_BUILD_DEBUG
	#define LOG(...) 
	#define ASSERT(x, ...) { if(!(x)) { LOG(__VA_ARGS__); __debugbreak(); } }
	#define BREAKIF(x) {if(x){__debugbreak();}}
#endif

#define UNUSED(x) ((void)(x))
