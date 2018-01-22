#pragma once
#include "graphics/Mesh.h"


namespace Graphics
{
    
    class LineMesh
        : public Mesh
    {
    public:
        //TODO(Assignment 1): Finish LineMesh for DebugDraw.
        //Once you're done with the class, you need to render this and there
        //are basically two ways to do so.

        // Method 1:  You can just attach this mesh to an object's Renderer Component
        // since it supports multiple mesh rendering, then you should be able to render it or 
        //see it appearing in the editor.

        // Method 2: Make a shaded component call it DebugDrawer or something, and in the 
        //components' SetShaderParam function, you render the mesh.


    };


}
