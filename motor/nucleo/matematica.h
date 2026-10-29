#pragma once
// Matemática mínima del motor. Unidades: metros, segundos. Eje Y hacia arriba.
#include <cmath>

namespace helios {

struct Vec3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;

    float& operator[](int i) { return i == 0 ? x : (i == 1 ? y : z); }
    float operator[](int i) const { return i == 0 ? x : (i == 1 ? y : z); }

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float k) const { return {x * k, y * k, z * k}; }
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
};

inline float producto_punto(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline float longitud(const Vec3& v) { return std::sqrt(producto_punto(v, v)); }

}  // namespace helios
