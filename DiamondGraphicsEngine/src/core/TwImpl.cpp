#include "Precompiled.h"
#include "core/TwImpl.h"
#include "framework/Debug.h"
#include "core/Object.h"
#include "math/Vector4.h"
#include "math/Quaternion.h"
#include "graphics/Texture.h"
#include "graphics/Materials.h"
#include "graphics/GraphicsEngine.h"
#include "framework/Application.h"
#include "graphics/FramebufferManager.h"
#include "graphics/Framebuffer.h"

namespace
{
    ///////////////////////////////////////////////////////////////////////////////////////
    //          Component Bar
    ///////////////////////////////////////////////////////////////////////////////////////
    std::vector<Object*> editorObjects;
    Graphics::GraphicsEngine* graphics;
    size_t objectSize = 0;
    size_t currentEditorObjId = 0;
    Object* currentObj = nullptr;
    TwBar* componentBar = nullptr;
    std::string componentBarName;

    void TW_CALL SetObjectComponent(const void *value, void *clientData);
    void TW_CALL GetObjectComponent(void *value, void *clientData);

    void ResetObject(int objEditId)
    {
        TwRemoveAllVars(componentBar);
        if (editorObjects.empty() == false)
        {
            std::string objectNames;
            for (auto& i : editorObjects)
            {
                objectNames += "ID:[" + std::to_string(i->GetHandle().GetId());
                objectNames += "] Name:";
                objectNames += i->GetName();
                objectNames += ",";
            }
            TwType objectNameTypes = TwDefineEnumFromString(nullptr, objectNames.c_str());
            TwAddVarCB(componentBar, nullptr, objectNameTypes, SetObjectComponent, GetObjectComponent, &currentEditorObjId, "label='Editor Object'");

        }

        if (objEditId < 0 || (objEditId >= static_cast<int>(editorObjects.size())))
        {
            return;
        }
        currentObj = editorObjects.at(objEditId);
        currentObj->Reflect(componentBar, componentBarName, graphics);
    }
    void TW_CALL SetObjectComponent(const void *value, void *clientData)
    {
        *static_cast<size_t *>(clientData) = *static_cast<const size_t *>(value);
        ResetObject(*static_cast<const size_t *>(value));
    }
    void TW_CALL GetObjectComponent(void *value, void *clientData)
    {
        *static_cast<size_t *>(value) = *static_cast<size_t *>(clientData);
    }


    ///////////////////////////////////////////////////////////////////////////////////////
    //          Resource Bar
    ///////////////////////////////////////////////////////////////////////////////////////

    std::vector<std::shared_ptr<Graphics::Texture> > textures;
    size_t textureEnumIntVal = 0;
    std::vector<std::shared_ptr<Graphics::Material> > materialsCache;
    size_t materialEnumIntVal = 0;
    TwBar* resourceBar = nullptr;
    std::shared_ptr<Graphics::Material> currentMaterial;

    void ResetResource();

    void ResetMaterial()
    {
        if (currentMaterial != nullptr)
        {
            currentMaterial->Reflect(resourceBar, currentMaterial->GetMaterialName(), graphics);
        }
    }
    
    void TW_CALL SetMaterial(const void *value, void *clientData)
    {
        size_t enumIndex = *static_cast<const size_t *>(value);
        *static_cast<size_t *>(clientData) = enumIndex;
        currentMaterial = materialsCache[enumIndex];
        ResetResource();
        ResetMaterial();
    }
    void TW_CALL GetMaterial(void *value, void *clientData)
    {
        *static_cast<size_t *>(value) = *static_cast<size_t *>(clientData);
    }
    void TW_CALL SetBackgroundColor(const void *value, void *)
    {
        if (graphics)
        {
            graphics->SetBackgroundColor(*static_cast<const Graphics::Color *>(value));
        }
    }
    void TW_CALL GetBackgroundColor(void *value, void *)
    {
        if (graphics)
        {
            *static_cast<Graphics::Color *>(value) = graphics->GetBackgroundColor();
        }
    }
    void TW_CALL OutputDeferredBuffers(void *)
    {
#if DEFERRED_SHADING_TEST
        std::shared_ptr<Graphics::FramebufferManager> fboManager = graphics->GetFrameBufferManager();
        for (u8 i = 0; i < 4; i++)
        {
            std::shared_ptr<Graphics::Texture> renderedTexture = fboManager->GetFramebuffer(Graphics::FramebufferType::DeferredGBuffer)->GetFboColorAttachment(i);
            renderedTexture->Bind(0);
            renderedTexture->DownloadContents();
            renderedTexture->Unbind();
            renderedTexture->SavePNG(renderedTexture, std::string("Buffer") + std::to_string(i) + ".png");
        }
#endif //DEFERRED_SHADING_TEST
    }

    void ResetResource()
    {
        TwRemoveAllVars(resourceBar);
        TwAddButton(resourceBar, nullptr, nullptr, nullptr, "label='Rendering'");        
        TwAddButton(resourceBar, nullptr, OutputDeferredBuffers, nullptr, "label='Output Buffer Textures'");
        TwType deferredRenderType = TwDefineEnumFromString(nullptr, "Combined, Diffuse Color, World Position, World Normal, Specular Color, Depth, Shadow Map");
        TwAddVarRW(resourceBar, nullptr, deferredRenderType, &graphics->DebugRenderUniform.OutputIndex, "label='Deferred Output'");
        TwAddVarRW(resourceBar, nullptr, TW_TYPE_BOOL32, &graphics->DebugRenderUniform.EnableBlur, "label='Enable Blur'");
        TwAddVarRW(resourceBar, nullptr, TW_TYPE_INT32, &graphics->DebugRenderUniform.BlurStrength, "label='Blur Strength'");

        TwAddVarCB(resourceBar, nullptr, TW_TYPE_COLOR3F, SetBackgroundColor, GetBackgroundColor, nullptr, "label='Background Color'");
        TwAddSeparator(resourceBar, nullptr, nullptr);
        TwAddButton(resourceBar, nullptr, nullptr, nullptr, "label='Textures'");
        if (textures.empty() == false)
        {
            std::string textureNames;
            for (auto& i : textures)
            {
                textureNames += i->GetTextureName();
                textureNames += ",";
            }
            TwType textureNameTypes = TwDefineEnumFromString(nullptr, textureNames.c_str());
            TwAddVarRW(resourceBar, nullptr, textureNameTypes, &textureEnumIntVal, "label='Registered Textures'");
        }
        TwAddSeparator(resourceBar, nullptr, nullptr);
        TwAddButton(resourceBar, nullptr, nullptr, nullptr, "label='Materials'");
        if (materialsCache.empty() == false)
        {
            std::string materialNames;
            for (auto& i : materialsCache)
            {
                materialNames += i->GetMaterialName();
                materialNames += ",";
            }
            TwType materialNameTypes = TwDefineEnumFromString(nullptr, materialNames.c_str());
            TwAddVarCB(resourceBar, nullptr, materialNameTypes,SetMaterial, GetMaterial, &materialEnumIntVal, "label='Registered Materials'");
        }
    }
}

TwBar* TwEditor::CreateComponentEditor(std::string const& name, std::vector<Object*> objs,
    std::shared_ptr<Graphics::GraphicsEngine> g , int width, int height)
{
    Assert(componentBar==nullptr, "Component Editor is already created.");
    InitComponentEditor();
    graphics = &*g;
    componentBarName = name;
    editorObjects = objs;
    componentBar = TwNewBar(name.c_str());
    TwDefine(("'" + componentBarName + "'" + " size='" + std::to_string(width) + " " + std::to_string(height) + "'").c_str());
    TwDefine(("'" + componentBarName + "'" + " position='0 0'").c_str());
    objectSize = objs.size();
    std::string defStr = "'" + name + "' refresh=0.01";
    TwDefine(defStr.c_str());
    ResetObject(-1);
    return componentBar;
}

TwBar* TwEditor::CreateResourceEditor(std::string const& name, std::vector<std::shared_ptr<Graphics::Texture>> tex,
    std::vector<std::shared_ptr<Graphics::Material>> mat, std::shared_ptr<Graphics::GraphicsEngine> g, int width, int height)
{
    Assert(resourceBar==nullptr, "Resource Editor is already created.");
    graphics = &*g;
    resourceBar = TwNewBar(name.c_str());
    materialsCache = mat;
    textures = tex;
    TwDefine(("'"+ name+"' " + " size='" + std::to_string(width) + " " + std::to_string(height) + "'").c_str());
    int posx = static_cast<int>(Application::GetInstance().GetWindowWidth()) - width;
    TwDefine(("'"+ name+"' " + " position='" + std::to_string(posx) + " 0'").c_str());

    ResetResource();
    ResetMaterial();
    return resourceBar;
}

void TwEditor::SetSelection(Object* obj)
{
    if (obj == nullptr)
    {
        ResetObject(-1);
        return;
    }
    size_t index = 0;
    for (auto& i : editorObjects)
    {
        if (i == obj)
        {
            ResetObject(index);
            break;
        }
        index++;
    }
}

void TwEditor::SetQuaternionFromAxisAngle(const float* axis, float angle, float* quat)
{
    const float sina2 = static_cast<float>(sin(0.5f * angle));
    const float norm = static_cast<float>(sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]));
    quat[0] = sina2 * axis[0] / norm;
    quat[1] = sina2 * axis[1] / norm;
    quat[2] = sina2 * axis[2] / norm;
    quat[3] = static_cast<float>(cos(0.5f * angle));
}

void TwEditor::ConvertQuaternionToMatrix(const float* quat, float* mat)
{
    float yy2 = 2.0f * quat[1] * quat[1];
    float xy2 = 2.0f * quat[0] * quat[1];
    float xz2 = 2.0f * quat[0] * quat[2];
    float yz2 = 2.0f * quat[1] * quat[2];
    float zz2 = 2.0f * quat[2] * quat[2];
    float wz2 = 2.0f * quat[3] * quat[2];
    float wy2 = 2.0f * quat[3] * quat[1];
    float wx2 = 2.0f * quat[3] * quat[0];
    float xx2 = 2.0f * quat[0] * quat[0];
    mat[0 * 4 + 0] = -yy2 - zz2 + 1.0f;
    mat[0 * 4 + 1] = xy2 + wz2;
    mat[0 * 4 + 2] = xz2 - wy2;
    mat[0 * 4 + 3] = 0;
    mat[1 * 4 + 0] = xy2 - wz2;
    mat[1 * 4 + 1] = -xx2 - zz2 + 1.0f;
    mat[1 * 4 + 2] = yz2 + wx2;
    mat[1 * 4 + 3] = 0;
    mat[2 * 4 + 0] = xz2 + wy2;
    mat[2 * 4 + 1] = yz2 - wx2;
    mat[2 * 4 + 2] = -xx2 - yy2 + 1.0f;
    mat[2 * 4 + 3] = 0;
    mat[3 * 4 + 0] = mat[3 * 4 + 1] = mat[3 * 4 + 2] = 0;
    mat[3 * 4 + 3] = 1;
}

void TwEditor::MultiplyQuaternions(const float* q1, const float* q2, float* qout)
{
    float qr[4];
    qr[0] = q1[3] * q2[0] + q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1];
    qr[1] = q1[3] * q2[1] + q1[1] * q2[3] + q1[2] * q2[0] - q1[0] * q2[2];
    qr[2] = q1[3] * q2[2] + q1[2] * q2[3] + q1[0] * q2[1] - q1[1] * q2[0];
    qr[3] = q1[3] * q2[3] - (q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2]);
    qout[0] = qr[0]; qout[1] = qr[1]; qout[2] = qr[2]; qout[3] = qr[3];
}

Math::Quaternion TwEditor::EulertoQuaternion(float pitch, float roll, float yaw)
{
    Math::Quaternion q;
    // Abbreviations for the various angular functions
    float cy = Math::Cos(yaw * 0.5f);
    float sy = Math::Sin(yaw * 0.5f);
    float cr = Math::Cos(roll * 0.5f);
    float sr = Math::Sin(roll * 0.5f);
    float cp = Math::Cos(pitch * 0.5f);
    float sp = Math::Sin(pitch * 0.5f);

    q.w = cy * cr * cp + sy * sr * sp;
    q.x = cy * sr * cp - sy * cr * sp;
    q.y = cy * cr * sp + sy * sr * cp;
    q.z = sy * cr * cp - cy * sr * sp;
    return q;
}

void TwEditor::QuaternionToEulerianAngle(Math::Quaternion const& q, float& roll, float& pitch, float& yaw)
{// roll (x-axis rotation)
    float sinr = +2.0f * (q.w * q.x + q.y * q.z);
    float cosr = +1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    roll = atan2(sinr, cosr);

    // pitch (y-axis rotation)
    float sinp = +2.0f * (q.w * q.y - q.z * q.x);
    if (fabs(sinp) >= 1)
        pitch = copysign(Math::c_Pi / 2.0f, sinp); // use 90 degrees if out of range
    else
        pitch = asin(sinp);

    // yaw (z-axis rotation)
    float siny = +2.0f * (q.w * q.z + q.x * q.y);
    float cosy = +1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    yaw = atan2(siny, cosy);
}


TW_TYPE_POINT::TW_TYPE_POINT(unsigned dimension, float step,
    std::string const& name1,
    std::string const& name2,
    std::string const& name3,
    std::string const& name4)
{
    m_dimension = dimension;
    m_name1 = name1;
    m_name2 = name2;
    m_name3 = name3;
    m_name4 = name4;
    m_step = step > 0 ? step : 0.01f;
    WarnIf(step < 0,
        "TW_TYPE_POINT() - invalid float step provided, using default.");
}
TW_TYPE_POINT::operator TwType() const
{
    Assert(m_dimension <= 4,
        "EditorHandler::operator TwType() - provided dimension is invalid.");
    std::string step("Step=");
    step.append(std::to_string(m_step));

    TwStructMember pointMembers[] = {
        { m_name1.c_str(), TW_TYPE_FLOAT, offsetof(Math::Vector4, x),step.c_str() },
        { m_name2.c_str(), TW_TYPE_FLOAT, offsetof(Math::Vector4, y),step.c_str() },
        { m_name3.c_str(), TW_TYPE_FLOAT, offsetof(Math::Vector4, z),step.c_str() },
        { m_name4.c_str(), TW_TYPE_FLOAT, offsetof(Math::Vector4, w),step.c_str() }, };

    return TwDefineStruct(nullptr, pointMembers, m_dimension,
        sizeof(Math::Vector4), nullptr, nullptr);
}

void TwEditor::InitComponentEditor()
{
    componentBarName.clear();
    editorObjects.clear();
    objectSize = 0;
    currentEditorObjId = 0;
    currentObj = nullptr;
    componentBar = nullptr;
    TwCopyStdStringToClientFunc(CopyStdStringToClient);
}
void TwEditor::CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString)
{
    destinationClientString = sourceLibraryString;
}
