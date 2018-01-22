///////////////////////////////////////////////////////////////////////////////
///
/// \file Vector4.cpp
/// Implementation of the Vector 4 structure.
/// 
/// Authors: Joshua Davis, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.h"
#include "math/Vector4.h"
#define ErrorIf 
#define Error
namespace Math
{
  Vector4 const Vector4::c_Origin(0.f, 0.f, 0.f, 1.f);
  const Vector4 Vector4::c_Zero(0.0f, 0.0f, 0.0f, 0.0f);
  const Vector4 Vector4::c_XAxis(1.0f, 0.0f, 0.0f, 0.0f);
  const Vector4 Vector4::c_YAxis(0.0f, 1.0f, 0.0f, 0.0f);
  const Vector4 Vector4::c_ZAxis(0.0f, 0.0f, 1.0f, 0.0f);
  const Vector4 Vector4::c_WAxis(0.0f, 0.0f, 0.0f, 1.0f);

  float* Vector4::ToFloats()
  {
    return (float*)this;
  }

  float const* Vector4::ToFloats() const
  {
    return (float const*)this;
  }

  Vector4::Vector4(float x_, float y_, float z_, float w_)
  {
    x = x_;
    y = y_;
    z = z_;
    w = w_;
  }

  Vector4::Vector4(ConstRealPointer data)
  {
    array[0] = data[0];
    array[1] = data[1];
    array[2] = data[2];
    array[3] = data[3];
  }

  Vector4::Vector4(float xyzw)
  {
    x = y = z = w = xyzw;
  }

  float& Vector4::operator[](unsigned index)
  {
    ErrorIf(index > 3, "Math::Vector4 - Subscript out of range.");
    return array[index];
  }

  float Vector4::operator[](unsigned index) const
  {
    ErrorIf(index > 3, "Math::Vector4 - Subscript out of range.");
    return array[index];
  }


  ////////// Unary Operators /////////////////////////////////////////////////////

  Vector4 Vector4::operator-() const
  {
    return Vector4(-x, -y, -z, -w);
  }


  ////////// Binary Assignment Operators (reals) /////////////////////////////////

  void Vector4::operator*=(float rhs)
  {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    w *= rhs;
  }

  void Vector4::operator/=(float rhs)
  {
    ErrorIf(Math::IsZero(rhs), "Math::Vector4 - Division by zero.");
    x /= rhs;
    y /= rhs;
    z /= rhs;
    w /= rhs;
  }


  ////////// Binary Operators (reals) ////////////////////////////////////////////

  Vector4 Vector4::operator*(float rhs) const
  {
    return Vector4(x * rhs, y * rhs, z * rhs, w * rhs);
  }

  Vector4 Vector4::operator/(float rhs) const
  {
    ErrorIf(Math::IsZero(rhs), "Math::Vector4 - Division by zero.");
    return Vector4(x / rhs, y / rhs, z / rhs, w / rhs);
  }


  ////////// Binary Assignment Operator (Vectors) ////////////////////////////////

  void Vector4::operator+=(Vec4Param rhs)
  {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
  }

  void Vector4::operator-=(Vec4Param rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
  }

  void Vector4::operator*=(Vec4Param rhs)
  {
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    w *= rhs.w;
  }

  void Vector4::operator/=(Vec4Param rhs)
  {
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    w /= rhs.w;
  }

  ////////// Binary Operators (Vectors) //////////////////////////////////////////

  Vector4 Vector4::operator+(Vec4Param rhs) const
  {
    return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
  }

  Vector4 Vector4::operator-(Vec4Param rhs) const
  {
    return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
  }


  ///////// Binary Vector Comparisons ////////////////////////////////////////////

  bool Vector4::operator==(Vec4Param rhs) const
  {
    return Equal(x, rhs.x) &&
      Equal(y, rhs.y) &&
      Equal(z, rhs.z) &&
      Equal(w, rhs.w);
  }

  bool Vector4::operator!=(Vec4Param rhs) const
  {
    return !(*this == rhs);
  }

  void Vector4::Set(float x_, float y_, float z_, float w_)
  {
    x = x_;
    y = y_;
    z = z_;
    w = w_;
  }

  void Vector4::Splat(float xyzw)
  {
    x = y = z = w = xyzw;
  }

  //Do a component-wise scaling of this vector with the given vector.
  void Vector4::ScaleByVector(Vec4Param rhs)
  {
    *this *= rhs;
  }

  Vector4 Vector4::ScaledByVector(Vec4Param rhs) const
  {
    return *this * rhs;
  }

  Vector4 Vector4::operator*(Vec4Param rhs) const
  {
    return Vector4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w);
  }

  Vector4 Vector4::operator/(Vec4Param rhs) const
  {
    ErrorIf(rhs.x == 0.0f || rhs.y == 0.0f ||
      rhs.z == 0.0f || rhs.w == 0.0f,
      "Vector4 - Division by Zero.");
    return Vector4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w);
  }


  void Vector4::ZeroOut()
  {
    array[0] = 0.0f;
    array[1] = 0.0f;
    array[2] = 0.0f;
    array[3] = 0.0f;
  }

  void Vector4::AddScaledVector(Vec4Param vector, float scalar)
  {
    x += vector.x * scalar;
    y += vector.y * scalar;
    z += vector.z * scalar;
    w += vector.w * scalar;
  }

  float Vector4::Dot(Vec4Param rhs) const
  {
    return (x * rhs.x) + (y * rhs.y) + (z * rhs.z) + (w * rhs.w);
  }

  float Vector4::Length() const
  {
    return Sqrt(LengthSq());
  }

  float Vector4::LengthSq() const
  {
    return Dot(*this);
  }

  Vector4 Vector4::Normalized() const
  {
    Vector4 ret = *this;
    ret /= Length();
    return ret;
  }

  float Vector4::Normalize()
  {
    float length = Length();
    *this /= length;
    return length;
  }

  float Vector4::AttemptNormalize()
  {
    float lengthSq = LengthSq();
    if (Math::IsZero(lengthSq) == false)
    {
      lengthSq = Sqrt(lengthSq);
      *this /= lengthSq;
    }
    return lengthSq;
  }

  Vec4Ref Vector4::Negate()
  {
    (*this) *= -1.0f;
    return *this;
  }

  bool Vector4::Valid() const
  {
    return IsValid(x) && IsValid(y) && IsValid(z) && IsValid(w);
  }

  Vector4 operator*(float lhs, Vec4Param rhs)
  {
    return rhs * lhs;
  }

  float Dot(Vec4Param lhs, Vec4Param rhs)
  {
    return lhs.Dot(rhs);
  }

  float Length(Vec4Param vect)
  {
    return vect.Length();
  }

  float LengthSq(Vec4Param vect)
  {
    return vect.LengthSq();
  }

  Vector4 Normalized(Vec4Param vect)
  {
    return vect.Normalized();
  }

  float Normalize(Vec4Ptr vect)
  {
    ErrorIf(vect == nullptr, "Vector4 - Null pointer passed for vector.");
    return vect->Normalize();
  }

  float AttemptNormalize(Vec4Ptr vect)
  {
    ErrorIf(vect == nullptr, "Vector4 - Null pointer passed for vector.");
    return vect->AttemptNormalize();
  }

  void Negate(Vec4Ptr vec)
  {
    ErrorIf(vec == nullptr, "Vector4 - Null pointer passed for vector.");
    *vec *= -1.0f;
  }

  Vector4 Negated(Vec4Param vec)
  {
    return Vector4(-vec.x, -vec.y, -vec.z, -vec.w);
  }

  Vector4 Abs(Vec4Param vec)
  {
    return Vector4(Math::Abs(vec.x), Math::Abs(vec.y),
      Math::Abs(vec.z), Math::Abs(vec.w));
  }

  Vector4 Min(Vec4Param lhs, Vec4Param rhs)
  {
    return Vector4(Math::Min(lhs.x, rhs.x),
      Math::Min(lhs.y, rhs.y),
      Math::Min(lhs.z, rhs.z),
      Math::Min(lhs.w, rhs.w));
  }

  Vector4 Max(Vec4Param lhs, Vec4Param rhs)
  {
    return Vector4(Math::Max(lhs.x, rhs.x),
      Math::Max(lhs.y, rhs.y),
      Math::Max(lhs.z, rhs.z),
      Math::Max(lhs.w, rhs.w));
  }

  Vector4 Lerp(Vec4Param start, Vec4Param end, float tValue)
  {
    return Vector4(start[0] + tValue * (end[0] - start[0]),
      start[1] + tValue * (end[1] - start[1]),
      start[2] + tValue * (end[2] - start[2]),
      start[3] + tValue * (end[3] - start[3]));
  }

}// namespace Math
