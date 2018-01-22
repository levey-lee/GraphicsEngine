#pragma once

namespace Graphics {
    class GraphicsEngine;
    class Material;
    class Texture;
}

namespace Math {
    struct Quaternion;
}

class Object;

/*******************************************************
 * @brief Functor for TwType, use AntTweakBar enums
 *******************************************************/
struct TW_TYPE_POINT
{
    TW_TYPE_POINT(unsigned dimension, float step = 0.01f,
        std::string const& name1 = "X",
        std::string const& name2 = "Y",
        std::string const& name3 = "Z",
        std::string const& name4 = "W");
    operator TwType() const;
private:
    int m_dimension;
    float m_step;
    std::string m_name1;
    std::string m_name2;
    std::string m_name3;
    std::string m_name4;
};

/*******************************************************
* @brief A static class handles all the AntTweakBars.
*******************************************************/
class TwEditor
{
public:
    static void InitComponentEditor();

    static TwBar* CreateComponentEditor(std::string const& name, std::vector<Object*> objs,
        std::shared_ptr<Graphics::GraphicsEngine> g,
        int width = 300,
        int height = 500);

    static TwBar* CreateResourceEditor(std::string const& name, 
        std::vector<std::shared_ptr<Graphics::Texture> > tex,
        std::vector<std::shared_ptr<Graphics::Material> >mat,
        std::shared_ptr<Graphics::GraphicsEngine> g,
        int width = 300,
        int height = 400);

    /*******************************************************
     * @brief Set the editor to selected object.
     * @param obj Selected object. Can be nullptr for not selecting anything.
     *******************************************************/
    static void SetSelection(Object* obj);

    static void SetQuaternionFromAxisAngle(const float *axis, float angle, float *quat);
    static void ConvertQuaternionToMatrix(const float *quat, float *mat);
    static void MultiplyQuaternions(const float *q1, const float *q2, float *qout);
    static Math::Quaternion EulertoQuaternion(float pitch, float roll, float yaw);
    static void QuaternionToEulerianAngle(Math::Quaternion const& q, float& roll, float& pitch, float& yaw);
private:
    static void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString);

};

