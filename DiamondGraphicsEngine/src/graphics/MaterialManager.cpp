#include "Precompiled.h"
#include "graphics/MaterialManager.h"
#include "framework/Debug.h"
#include "graphics/Materials.h"
#include "graphics/GraphicsEngine.h"
#include "graphics/TextureManager.h"
#include "graphics/Texture.h"

namespace Graphics
{
    void MaterialManager::LoadMaterials(std::string const& fileName, std::shared_ptr<GraphicsEngine> g)
    {
        std::stringstream strstr;
        strstr << ASSET_PATH << "materials/" << fileName;
        //TODO(Assignemtn 1): Load all materials in "basic.mtl" and store them in m_materials
        //The material file name is the file name under assets\materials\ directory
        //once you load them in, make sure you call function empalceMaterial after a material
        //is loaded and setup, so it will be stored in the container.


        //TODO(Assignemtn 1): Remove everything below, in this function. Since material will be loaded, not hardcoded
        ////////////////////////////////////////////////////////////////////////////
        //      Texture
        //BTR80A
        std::shared_ptr<Material> btr80AMat = std::shared_ptr<Material>(new Material);
        btr80AMat->SetAmbientColor(Color(0.1f, 0.1f, 0.1f));
        btr80AMat->SetIlluminationModel(2);
        btr80AMat->SetDiffuseColor(Color(1, 1, 1));
        btr80AMat->SetSpecularExponent(10.0f);
        btr80AMat->SetEmissiverColor(Color(0, 0, 0));
        emplaceMaterial("BTR80A", btr80AMat);
               


        //skybox
        //textures.at(skyboxTex)->Build();
        std::shared_ptr<Material> skyboxMat = std::shared_ptr<Material>(new Material);
        skyboxMat->SetDiffuseColor(Color(1, 1, 1));
        skyboxMat->SetIfReceiveLight(false);
        skyboxMat->SetAmbientColor(Color(0.1f, 0.1f, 0.1f));
        skyboxMat->SetSpecularColor(Color(0,0,0));
        emplaceMaterial("Skydome", skyboxMat);

        //plane
        std::shared_ptr<Material> planeMat = std::make_shared<Material>(*btr80AMat);
        planeMat->SetSpecularExponent(10.0f);
        emplaceMaterial("Plane", planeMat);

        //golf
        std::shared_ptr<Material> golfMat = std::make_shared<Material>(*btr80AMat);
        golfMat->SetSpecularExponent(50.0f);
        emplaceMaterial("Golf", golfMat);

        //Teapot
        std::shared_ptr<Material> teapotMat = std::make_shared<Material>(*btr80AMat);
        emplaceMaterial("Teapot", teapotMat);
        
        //Sponge
        std::shared_ptr<Material> spongeMat = std::make_shared<Material>(*btr80AMat);
        emplaceMaterial("Sponge", spongeMat);

        //ReversedSphere
        std::shared_ptr<Material> reversedSphereMat = std::make_shared<Material>(*btr80AMat);
        emplaceMaterial("ReversedSphere", reversedSphereMat);

        //Sphere
        std::shared_ptr<Material> sphereMat = std::make_shared<Material>(*btr80AMat);
        emplaceMaterial("Sphere", sphereMat);

        //Lucy
        std::shared_ptr<Material> Lucy = std::make_shared<Material>(*btr80AMat);
        emplaceMaterial("Lucy", Lucy);


        std::shared_ptr<TextureManager> textureManager = g->GetTextureManager();

        std::string wareHouseHDR = "Warehouse-with-lights.jpg";
        std::string groundHDR = "GroudHDR.jpg";
        std::string grassGroudHDR = "hdrmaps_com_free_060_Bg.jpg";
        std::string metalRoof = "metal_roof_diff_512x512.jpg";
        std::string btrDiffuse = "BTR80A/DF_4k_btr80a02.jpg";
        std::string btrNormalMap = "BTR80A/btr_base_n.jpg";
        std::map<std::string, std::shared_ptr<Texture> > & textures = textureManager->GetTextureMap();
        textureManager->LoadTextureMultiThreadRealTime(
        {
            { wareHouseHDR },
            { groundHDR },
            { grassGroudHDR },
            { metalRoof },
            { btrDiffuse },
            { btrNormalMap },
        });
        //btr80A
        btr80AMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, textures.at(btrDiffuse));
        //btr80AMat->AssignNormalMapTexture(TextureType::NormalMapTexture_0, textures.at(btrNormalMap));

        //skybox
        skyboxMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, textures.at(grassGroudHDR));

        //teapot
        teapotMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, textures.at(grassGroudHDR));

        //sphere
        sphereMat->AssignDiffuseTexture(TextureType::DiffuseTexture_0, textures.at(wareHouseHDR));

        //plane
        //planeMesh->AssignDiffuseTexture(TextureType::DiffuseTexture_0, textures.at(metalRoof));

    }

    std::shared_ptr<Material> MaterialManager::AddMaterial(std::shared_ptr<Material> material)
    {
        m_materials[material->GetMaterialName()] = material;
        return material;
    }

    std::shared_ptr<Material> MaterialManager::GetMaterial(std::string const& matName)
    {
#ifdef _DEBUG
        if (m_materials.find(matName) == m_materials.end())
        {
            Warning("Material not found with name \"%s\".", matName.c_str());
            return nullptr;
        }
#endif // DEBUG
        return m_materials[matName];
    }


    std::vector<std::shared_ptr<Material>> MaterialManager::GetAllMaterials() const
    {
        std::vector<std::shared_ptr<Material>> materials;
        for (auto& i : m_materials)
        {
            materials.push_back(i.second);
        }
        return materials;
    }

    void MaterialManager::emplaceMaterial(std::string const& matName, std::shared_ptr<Material> material)
    {
        material->SetMaterialName(matName);
        m_materials.emplace(matName, material);
    }
}
