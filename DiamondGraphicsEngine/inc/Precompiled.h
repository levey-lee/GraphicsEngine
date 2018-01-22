#pragma once

/* Precompiled header. */

#pragma warning(disable : 4996)

#ifndef STD_DEFINE
#define STB_DEFINE   
#endif // !STD_DEFINE

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif // !STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif // STB_IMAGE_WRITE_IMPLEMENTATION

#define UNUSED_VAR(x) static_cast<void>(x);
//#define DEBUG_PRINT_DATA_FLOW std::cout<<"DEBUG_PRINT_DATA_FLOW_FUNCTION: " << __FUNCTION__ <<std::endl;
#define DEBUG_PRINT_DATA_FLOW //leave this empty to call nothing

#define REGISTER_EDITOR_COMPONENT(name) std::string GetComponentTypeName() override { return #name; }

#define SAMPLE_IMPLEMENTATION 1
#define ASSIGNMENT_1_HIDE_SHADER 0


// This is the path used to locate assets (shaders and models) by the
// ShaderProgram and MeshLoader loaders. This path is dictated by the premake
// script and is always kept relative to the VS project directory.
#ifndef ASSET_PATH
#define ASSET_PATH "../"
#endif

#include <iostream>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <cassert>

#include <string>
#include <vector>
#include <array>
#include <list>
#include <forward_list>
#include <set>
#include <queue>
#include <stack>
#include <deque>
#include <bitset>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iterator>
#include <memory>
#include <mutex>
#include <numeric>
#include <random>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <exception>
#include <typeindex>

#include <cassert>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

#include "GLEW/glew.h"
#include "FreeGLUT/freeglut.h"
#include "AntTweakBar/AntTweakBar.h"



using ObjectId = long long int;

