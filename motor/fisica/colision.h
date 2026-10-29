#pragma once
// Colisión contra geometría estática hecha de cajas alineadas a los ejes (AABB).
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>
#include "motor/nucleo/matematica.h"

namespace helios {

struct Caja {
    Vec3 minimo;
    Vec3 maximo;
};

struct MundoColision {
    std::vector<Caja> solidos;
};

// Superposición estricta: dos cajas que solo se tocan NO se consideran superpuestas.
inline bool cajas_se_superponen(const Caja& a, const Caja& b) {
    return a.minimo.x < b.maximo.x && a.maximo.x > b.minimo.x &&
           a.minimo.y < b.maximo.y && a.maximo.y > b.minimo.y &&
           a.minimo.z < b.maximo.z && a.maximo.z > b.minimo.z;
}

// Rayo contra caja (método de las "losas"). Devuelve true si el rayo la alcanza
// y deja en `t_entrada` la distancia (en unidades de `direccion`) al primer contacto.
inline bool rayo_contra_caja(const Vec3& origen, const Vec3& direccion, const Caja& caja,
                             float& t_entrada) {
    float t_min = 0.0f;
    float t_max = std::numeric_limits<float>::max();
    for (int eje = 0; eje < 3; ++eje) {
        if (std::fabs(direccion[eje]) < 1e-8f) {
            if (origen[eje] < caja.minimo[eje] || origen[eje] > caja.maximo[eje]) return false;
        } else {
            float t1 = (caja.minimo[eje] - origen[eje]) / direccion[eje];
            float t2 = (caja.maximo[eje] - origen[eje]) / direccion[eje];
            if (t1 > t2) std::swap(t1, t2);
            t_min = std::max(t_min, t1);
            t_max = std::min(t_max, t2);
            if (t_min > t_max) return false;
        }
    }
    t_entrada = t_min;
    return true;
}

}  // namespace helios
