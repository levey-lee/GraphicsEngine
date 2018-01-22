#include "Precompiled.h"
#include "framework/Utilities.h"
#include "graphics/MeshManager.h"
#include "graphics/TriangleMesh.h"
#include "framework/Debug.h"
std::unordered_map<std::string /*label*/, std::shared_ptr<Graphics::Mesh> > Graphics::MeshManager::m_meshes;
namespace
{
    Math::Vector3 CreateSpherePoint(f32 theta, f32 phi, Math::Vector3 &norm)
    {
        f32 sinPhi = sinf(phi);
        norm = Math::Vector3(cosf(theta) * sinPhi, cosf(phi), sinf(theta) * sinPhi);
        // fix radius = 1 and center at (0, 0, 0)
        return norm + Math::Vector3(0, 0, 0);
    }
    
}
namespace Graphics
{
    std::vector<std::shared_ptr<Mesh>> MeshManager::GetAllMeshes()
    {
        std::vector<std::shared_ptr<Mesh>> vec;
        vec.reserve(m_meshes.size());
        for (auto& i : m_meshes)
        {
            vec.push_back(i.second);
        }
        return vec;
    }

    std::shared_ptr<TriangleMesh> MeshManager::TriangleMeshHandler::LoadObjMesh(std::string const &meshLabel, std::string const& objFileName)
    {
        std::stringstream strstr;
        strstr << ASSET_PATH << "models/" << objFileName;
        FILE* objFile = fopen(strstr.str().c_str(), "rt");

        if (!objFile)
        {
            Warning("File with name %s cannot be opend for loading.", objFileName.c_str());
            return nullptr;
        }

        // TODO(Assignment 1):implement a Wavefront OBJ loader in this file, 
        // assemble an instance of TriangleMesh using the data read in, and return it

        // NOTE: the 'input' stream above is ready to being reading the text lines
        // of OBJ files; it already is setup to correctly find the object file
        // under the assets/models directory.
        // The file is a bare minimal Obj file, contains only vertex and face with tag 'v' and 'f'
        // Be careful that the index in the file starts at 1.

#if SAMPLE_IMPLEMENTATION
        char buffer[256]; //max string length
        TriangleMesh* mesh = new TriangleMesh;

        while (!feof(objFile))
        {
            fgets(buffer, 256, objFile);

            switch (buffer[0])
            {
            case 'v':
                float x, y, z;
                sscanf(buffer, "v %f %f %f", &x, &y, &z);
                mesh->AddVertex(x, y, z);
                break;
            case 'f':
                unsigned a, b, c;
                sscanf(buffer, "f %u %u %u", &a, &b, &c);
                mesh->AddTriangle(a - 1, b - 1, c - 1);
                break;
            default:
                //probably a comment or empty line, ignore
                break;
            }
        }
        fclose(objFile);
        mesh->Preprocess();
        mesh->SetLabel(meshLabel);
        Assert(m_meshes.find(meshLabel) == m_meshes.end(), "Mesh with label \"%s\" already exists.", meshLabel.c_str());
        m_meshes.emplace(meshLabel, mesh);
        return std::static_pointer_cast<TriangleMesh>(m_meshes[meshLabel]);
#else
        //TODO(Assignment 1): Once you have the model loaded. Do the following:
        //Step 1: Call Preprocess
        //Step 2: Set label for the mesh
        //Step 3: Check if it exists in the unordered_map and place the mesh in m_meshes
        //Step 4: return the pointer

        return BuildTriangle(meshLabel);
#endif // SAMPLE_IMPLEMENTATION

    }


    std::shared_ptr<TriangleMesh> MeshManager::TriangleMeshHandler::BuildTriangle(std::string const &meshLabel)
    {
        TriangleMesh *mesh = new TriangleMesh;

        // construct vertices for a single triangle
        mesh->AddVertex(0, 0.5f, -1);
        mesh->AddVertex(-0.5f, -0.5f, -1);
        mesh->AddVertex(0.5f, -0.5f, -1);
        // stitch these vertices together in CCW order to create a triangle
        mesh->AddTriangle(0, 1, 2);

        mesh->Preprocess();
        mesh->SetLabel(meshLabel);
        Assert(m_meshes.find(meshLabel) == m_meshes.end(), "Mesh with label \"%s\" already exists.", meshLabel.c_str());
        m_meshes.emplace(meshLabel, mesh);
        return std::static_pointer_cast<TriangleMesh>(m_meshes[meshLabel]);
    }

    std::shared_ptr<TriangleMesh> MeshManager::TriangleMeshHandler::BuildSphere(std::string const &meshLabel)
    {
        using namespace Math;
        TriangleMesh *mesh = new TriangleMesh;
        f32 deltaPhi = 0.22439947525641380274733167023425f; // hardcode 12 stacks
        f32 deltaTheta = 0.52359877559829887307710723054658f; // hardcode 12 slices
        f32 phi = deltaPhi;
        //@remark: could easily optimize this by not duplicating points
        u32 vertexCount = 0;
        for (u8 y = 0; y <= 12; ++y) // stacks represents # of actual strips
        {
            f32 theta = 0.f;
            f32 nextPhi = phi + deltaPhi;
            for (u8 x = 0; x <= 12; ++x)
            {
                f32 nextTheta = theta + deltaTheta;
                Vector3 normA, normB, normC, normD;
                Vector3 a = CreateSpherePoint(theta, phi, normA);
                Vector3 b = CreateSpherePoint(nextTheta, phi, normB);
                Vector3 c = CreateSpherePoint(nextTheta, nextPhi, normC);
                Vector3 d = CreateSpherePoint(theta, nextPhi, normD);
                mesh->AddVertex(a.x, a.y, a.z);
                mesh->AddVertex(b.x, b.y, b.z);
                mesh->AddVertex(c.x, c.y, c.z);
                mesh->AddVertex(d.x, d.y, d.z);
                u32 idxA = vertexCount++, idxB = vertexCount++;
                u32 idxC = vertexCount++, idxD = vertexCount++;
                mesh->GetVertex(idxA).normal = normA.Normalized();
                mesh->GetVertex(idxB).normal = normB.Normalized();
                mesh->GetVertex(idxC).normal = normC.Normalized();
                mesh->GetVertex(idxD).normal = normD.Normalized();
                mesh->AddTriangle(idxA, idxB, idxC);
                mesh->AddTriangle(idxA, idxC, idxD);
                theta = nextTheta;
            }
            phi = nextPhi;
        }
        mesh->Preprocess();
        mesh->SetLabel(meshLabel);
        Assert(m_meshes.find(meshLabel) == m_meshes.end(), "Mesh with label \"%s\" already exists.", meshLabel.c_str());
        m_meshes.emplace(meshLabel, mesh);
        return std::static_pointer_cast<TriangleMesh>(m_meshes[meshLabel]);
    }
}
