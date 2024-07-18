#pragma once
#define LOG(...) 
#define ASSERT(x, ...){ if(!(x)) { LOG(__VA_ARGS__); __debugbreak(); } }
