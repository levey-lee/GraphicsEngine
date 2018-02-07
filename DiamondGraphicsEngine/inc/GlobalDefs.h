#pragma once

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

// This is the path used to locate assets (shaders and models) by the
// ShaderProgram and MeshLoader loaders. This path is dictated by the premake
// script and is always kept relative to the VS project directory.
#ifndef ASSET_PATH
#define ASSET_PATH "../"
#endif


#define REGISTER_EDITOR_COMPONENT(name) std::string GetComponentTypeName() override { return #name; }
#define SAMPLE_IMPLEMENTATION 1
#define ASSIGNMENT_1_HIDE_SHADER 0
#define VERBOSE 1
#define DEFERRED_SHADING_TEST 0
#define UNUSED_VAR(x) static_cast<void>(x);

#if VERBOSE
//#define DEBUG_PRINT_DATA_FLOW std::cout<<"DEBUG_PRINT_DATA_FLOW_FUNCTION: " << __FUNCTION__ <<std::endl;
#define DEBUG_PRINT_DATA_FLOW //leave this empty to call nothing
#else
#define DEBUG_PRINT_DATA_FLOW //leave this empty to call nothing
#endif

