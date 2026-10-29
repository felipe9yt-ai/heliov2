#pragma once
// Controlador cinemático del jugador en primera persona.
// Todos los valores de ConfigMovimiento son DECISIÓN PROPIA: no están medidos
// contra Half-Life y se calibrarán más adelante con pruebas de observación.
#include "motor/fisica/colision.h"
#include "motor/nucleo/matematica.h"

namespace helios {

struct ConfigMovimiento {
    float velocidad_maxima = 6.0f;       // m/s en suelo
    float aceleracion_suelo = 10.0f;
    float aceleracion_aire = 10.0f;
    float limite_velocidad_aire = 0.7f;  // tope de la "velocidad deseada" en el aire (m/s)
    float friccion = 6.0f;
    float velocidad_frenado = 2.0f;      // por debajo de esto la fricción usa este valor
    float gravedad = 20.0f;              // m/s^2
    float velocidad_salto = 7.5f;        // m/s
    float radio = 0.4f;                  // mitad del ancho de la caja de colisión
    float altura = 1.8f;
    float altura_ojos = 1.6f;
    float altura_escalon = 0.45f;        // desnivel máximo que se sube caminando
};

struct Jugador {
    Vec3 posicion;    // posición de los PIES
    Vec3 velocidad;
    float yaw = 0.0f;    // radianes; 0 mira hacia -Z
    float pitch = 0.0f;
    bool en_suelo = false;
};

struct EntradaJugador {
    float adelante = 0.0f;  // -1..1
    float lateral = 0.0f;   // -1..1 (positivo = derecha)
    bool salto = false;
};

// Caja de colisión del jugador (los pies están en la cara inferior).
Caja caja_del_jugador(const Jugador& jugador, const ConfigMovimiento& config);

// Avanza la simulación un paso fijo de `dt` segundos.
void simular_jugador(Jugador& jugador, const EntradaJugador& entrada,
                     const MundoColision& mundo, const ConfigMovimiento& config, float dt);

}  // namespace helios
