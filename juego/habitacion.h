#pragma once
// Habitación de prueba de la v0.1 (interior de 12 x 4 x 12 m, con una caja y una columna).
#include "motor/fisica/colision.h"

namespace helios {

inline MundoColision crear_habitacion_prueba() {
    MundoColision m;
    m.solidos = {
        {{-7, -1, -7}, {7, 0, 7}},      // 0 suelo
        {{-7, 4, -7}, {7, 5, 7}},       // 1 techo
        {{-7, -1, -7}, {7, 5, -6}},     // 2 pared norte
        {{-7, -1, 6}, {7, 5, 7}},       // 3 pared sur
        {{-7, -1, -7}, {-6, 5, 7}},     // 4 pared oeste
        {{6, -1, -7}, {7, 5, 7}},       // 5 pared este
        {{1, 0, -4}, {2.5f, 1.5f, -2.5f}},  // 6 caja
        {{-4, 0, 2}, {-3, 4, 3}},       // 7 columna
    };
    return m;
}

}  // namespace helios
