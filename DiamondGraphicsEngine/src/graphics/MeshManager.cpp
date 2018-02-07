#include "Precompiled.h"
#include "framework/Utilities.h"
#include "graphics/MeshManager.h"
#include "graphics/TriangleMesh.h"
#include "framework/Debug.h"
std::unordered_map<std::string /*label*/, std::shared_ptr<Graphics::Mesh> > Graphics::MeshManager::m_meshes;
std::shared_mutex Graphics::MeshManager::m_meshListMutex;
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
    std::shared_ptr<TriangleMesh> MeshManager::TriangleMeshHandler::GetMesh(std::string const& label) const
    {
        return std::static_pointer_cast<TriangleMesh>(m_meshes.at(label));
    }

    std::shared_ptr<Mesh> MeshManager::GetMesh(std::string const& label) const
    {
        return m_meshes.at(label);
    }

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

    std::shared_ptr<TriangleMesh> MeshManager::TriangleMeshHandler::LoadObjMesh(std::string const &meshLabel, std::string const& objFileName, DefaultUvType defaultUvType)
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
        mesh->Preprocess(defaultUvType);
        mesh->SetLabel(meshLabel);
        m_meshListMutex.lock();
        Assert(m_meshes.find(meshLabel) == m_meshes.end(), "Mesh with label \"%s\" already exists.", meshLabel.c_str());
        m_meshes.emplace(meshLabel, mesh);
#if VERBOSE
        std::cout << "A thread finished loading model with name:\"" + meshLabel + "\"\n";
#endif // VERBOSE

        m_meshListMutex.unlock();

        return std::static_pointer_cast<TriangleMesh>(m_meshes.at(meshLabel));
#else
        //TODO(Assignment 1): Once you have the model loaded. Do the following:
        //Step 1: Call Preprocess
        //Step 2: Set label for the mesh
        //Step 3: Check if it exists in the unordered_map and place the mesh in m_meshes
        //Step 4: return the pointer

        return BuildTriangle(meshLabel);
#endif // SAMPLE_IMPLEMENTATION
    }

    std::shared_ptr<TriangleMesh> MeshManager::TriangleMeshHandler::LoadObjMeshWithUvNormal(
        std::string const& meshLabel, std::string const& objFileName)
   {
#if VERBOSE
        printf("Loading OBJ file %s... with uv and normal.\n", objFileName.c_str());
#endif // VERBOSE
        std::stringstream strstr;
        strstr << ASSET_PATH << "models/" << objFileName;
        FILE * file = fopen(strstr.str().c_str(), "r");
        if (file == nullptr) 
        {
            printf("Impossible to open the file ! Are you in the right path ? \n");
            return nullptr;
        }
        
        TriangleMesh* mesh = new TriangleMesh;

        std::vector<std::tuple<TriangleMesh::TriangleFace, TriangleMesh::TriangleFace, TriangleMesh::TriangleFace> >faces;

        std::vector<Math::Vector3> verts;
        std::vector<Math::Vector2> uvs;
        std::vector<Math::Vector3> normals;

        std::vector<TriangleMesh::Vertex> vertices;

        while (1) 
        {
            char lineHeader[128];
            // read the first word of the line
            int res = fscanf(file, "%s", lineHeader);
            if (res == EOF)
                break; // EOF = End Of File. Quit the loop.
                       // else : parse lineHeader

            if (strcmp(lineHeader, "v") == 0) 
            {
                Math::Vec3 vertex;
                fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
                verts.push_back(vertex);
            }
            else if (strcmp(lineHeader, "vt") == 0)
            {
                Math::Vec2 uv;
                fscanf(file, "%f %f\n", &uv.x, &uv.y);
                uvs.push_back(uv);
            }
            else if (strcmp(lineHeader, "vn") == 0)
            {
                Math::Vec3 normal;
                fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
                normals.push_back(normal);
            }
            else if (strcmp(lineHeader, "f") == 0)
            {
                std::string vertex1, vertex2, vertex3;
                unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
                int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
                faces.push_back(
                {
                    { vertexIndex[0] - 1, vertexIndex[1] - 1, vertexIndex[2] - 1 },
                    { uvIndex[0] - 1, uvIndex[1] - 1, uvIndex[2] - 1 },
                    { normalIndex[0] - 1, normalIndex[1] - 1, normalIndex[2] - 1 }
                });

                if (matches != 9)
                {
                    printf("File can't be read by our simple parser :-( Try exporting with other options\n");
                    fclose(file);
                    return nullptr;
                }
            }
            else
            {
                // Probably a comment, eat up the rest of the line
                char stupidBuffer[1000];
                fgets(stupidBuffer, 1000, file);
            }
        }
        fclose(file);

        vertices.resize(verts.size());


        // For each vertex of each triangle
        for (auto& i : faces)
        {
            u32& v0 = std::get<0>(i).a;
            u32& v1 = std::get<0>(i).b;
            u32& v2 = std::get<0>(i).c;
            
            u32& vt0 = std::get<1>(i).a;
            u32& vt1 = std::get<1>(i).b;
            u32& vt2 = std::get<1>(i).c;
            
            u32& vn0 = std::get<2>(i).a;
            u32& vn1 = std::get<2>(i).b;
            u32& vn2 = std::get<2>(i).c;


            vertices[v0].position = verts[v0];
            vertices[v1].position = verts[v1];
            vertices[v2].position = verts[v2];

            vertices[v0].uv = uvs[vt0];
            vertices[v1].uv = uvs[vt1];
            vertices[v2].uv = uvs[vt2];

            vertices[v0].normal = normals[vn0];
            vertices[v1].normal = normals[vn1];
            vertices[v2].normal = normals[vn2];

            mesh->AddTriangle(v0,v1,v2);
        }

        mesh->m_vertices = vertices;


        mesh->centerMesh();
        mesh->normalizeVertices();
        mesh->CalculateBoundingSphere();
        mesh->CalcTanBitan();
        mesh->SetLabel(meshLabel);
        Assert(m_meshes.find(meshLabel) == m_meshes.end(), "Mesh with label \"%s\" already exists.", meshLabel.c_str());
        m_meshListMutex.lock();
        m_meshes.emplace(meshLabel, mesh);
        m_meshListMutex.unlock();
        return std::static_pointer_cast<TriangleMesh>(m_meshes.at(meshLabel));
    }

    void MeshManager::TriangleMeshHandler::LoadAndBuildObjMeshMultiThread(
        const std::vector<std::tuple<std::string/*meshLabel*/, std::string/*objFile*/, DefaultUvType> >& meshList)
    {
        size_t threadNum = meshList.size();
        std::vector<std::thread> threads(threadNum);


        for (size_t i = 0; i < threadNum; ++i)
        {
            threads[i] = std::thread(&TriangleMeshHandler::LoadObjMesh,this,
                std::get<0>(meshList[i]), std::get<1>(meshList[i]), std::get<2>(meshList[i]));
            
        }

        for (size_t i = 0; i < threadNum; ++i)
        {
            threads[i].join();
        }

        for (auto& i : meshList)
        {
            m_meshes.at(std::get<0>(i))->Build();
        }
    }


    std::shared_ptr<TriangleMesh> MeshManager::TriangleMeshHandler::BuildTriangle(std::string const &meshLabel, DefaultUvType defaultUvType)
    {
        TriangleMesh *mesh = new TriangleMesh;

        // construct vertices for a single triangle
        mesh->AddVertex(0, 0.5f, -1);
        mesh->AddVertex(-0.5f, -0.5f, -1);
        mesh->AddVertex(0.5f, -0.5f, -1);
        // stitch these vertices together in CCW order to create a triangle
        mesh->AddTriangle(0, 1, 2);

        mesh->Preprocess(defaultUvType);
        mesh->SetLabel(meshLabel);
        Assert(m_meshes.find(meshLabel) == m_meshes.end(), "Mesh with label \"%s\" already exists.", meshLabel.c_str());
        m_meshListMutex.lock();
        m_meshes.emplace(meshLabel, mesh);
        m_meshListMutex.unlock();
        return std::static_pointer_cast<TriangleMesh>(m_meshes[meshLabel]);
    }

    std::shared_ptr<TriangleMesh> MeshManager::TriangleMeshHandler::BuildFullScreenQuad(std::string const& meshLabel)
    {
        TriangleMesh *mesh = new TriangleMesh;


        mesh->AddVertex(-1.0f, -1.0f, 0.0f);//0 - lower left
        mesh->AddVertex( 1.0f, -1.0f, 0.0f);//1 - lower right
        mesh->AddVertex(-1.0f,  1.0f, 0.0f);//2 - top left
        mesh->AddVertex( 1.0f,  1.0f, 0.0f);//3 - top right

        mesh->AddTriangle(0, 1, 2);
        mesh->AddTriangle(2, 1, 3);

        mesh->SetLabel(meshLabel);
        Assert(m_meshes.find(meshLabel) == m_meshes.end(), "Mesh with label \"%s\" already exists.", meshLabel.c_str());
        m_meshListMutex.lock();
        m_meshes.emplace(meshLabel, mesh);
        m_meshListMutex.unlock();
        return std::static_pointer_cast<TriangleMesh>(m_meshes[meshLabel]);
    }

    std::shared_ptr<TriangleMesh> MeshManager::TriangleMeshHandler::BuildSphere(std::string const &meshLabel, DefaultUvType defaultUvType)
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
        mesh->Preprocess(defaultUvType);
        mesh->SetLabel(meshLabel);
        Assert(m_meshes.find(meshLabel) == m_meshes.end(), "Mesh with label \"%s\" already exists.", meshLabel.c_str());
        m_meshListMutex.lock();
        m_meshes.emplace(meshLabel, mesh);
        m_meshListMutex.unlock();
        return std::static_pointer_cast<TriangleMesh>(m_meshes[meshLabel]);
    }
}
