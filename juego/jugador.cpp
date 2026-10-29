#include "juego/jugador.h"

#include <algorithm>

namespace helios {

Caja caja_del_jugador(const Jugador& j, const ConfigMovimiento& c) {
    return {{j.posicion.x - c.radio, j.posicion.y, j.posicion.z - c.radio},
            {j.posicion.x + c.radio, j.posicion.y + c.altura, j.posicion.z + c.radio}};
}

namespace {

// Mueve al jugador a lo largo de UN eje y lo expulsa de los sólidos que atraviese.
// Devuelve true si hubo colisión.
bool mover_en_eje(Jugador& j, int eje, float delta, const ConfigMovimiento& c,
                  const MundoColision& mundo) {
    if (delta == 0.0f) return false;
    j.posicion[eje] += delta;
    const float extension_superior = (eje == 1) ? c.altura : c.radio;
    const float extension_inferior = (eje == 1) ? 0.0f : c.radio;
    bool choco = false;
    for (const Caja& solido : mundo.solidos) {
        if (!cajas_se_superponen(caja_del_jugador(j, c), solido)) continue;
        choco = true;
        if (delta > 0.0f) j.posicion[eje] = solido.minimo[eje] - extension_superior;
        else              j.posicion[eje] = solido.maximo[eje] + extension_inferior;
    }
    if (choco) j.velocidad[eje] = 0.0f;
    return choco;
}

void aplicar_friccion(Vec3& vel, const ConfigMovimiento& c, float dt) {
    const float rapidez = std::sqrt(vel.x * vel.x + vel.z * vel.z);
    if (rapidez < 1e-6f) { vel.x = vel.z = 0.0f; return; }
    const float control = std::max(rapidez, c.velocidad_frenado);
    const float nueva = std::max(rapidez - control * c.friccion * dt, 0.0f);
    const float factor = nueva / rapidez;
    vel.x *= factor;
    vel.z *= factor;
}

// Aceleración tipo Quake: solo suma velocidad en la dirección deseada hasta el tope.
void acelerar(Vec3& vel, const Vec3& direccion, float rapidez_deseada, float aceleracion, float dt) {
    const float actual = vel.x * direccion.x + vel.z * direccion.z;
    const float falta = rapidez_deseada - actual;
    if (falta <= 0.0f) return;
    const float incremento = std::min(aceleracion * rapidez_deseada * dt, falta);
    vel.x += incremento * direccion.x;
    vel.z += incremento * direccion.z;
}

// Si el movimiento horizontal quedó bloqueado, prueba "subir un escalón":
// elevar al jugador, avanzar, y volver a bajar. Solo se acepta si avanza más
// que el movimiento normal y aterriza sobre algo más alto que el punto de partida.
void intentar_subir_escalon(Jugador& j, const Jugador& antes, const ConfigMovimiento& c,
                            const MundoColision& mundo, float dt) {
    Jugador intento = antes;
    intento.velocidad.y = 0.0f;
    if (mover_en_eje(intento, 1, c.altura_escalon, c, mundo)) return;  // techo demasiado bajo
    mover_en_eje(intento, 0, antes.velocidad.x * dt, c, mundo);
    mover_en_eje(intento, 2, antes.velocidad.z * dt, c, mundo);
    if (!mover_en_eje(intento, 1, -c.altura_escalon, c, mundo)) return;  // no aterrizó sobre nada
    if (intento.posicion.y <= antes.posicion.y + 1e-3f) return;           // no era un escalón

    auto distancia2 = [&](const Jugador& a) {
        const float dx = a.posicion.x - antes.posicion.x;
        const float dz = a.posicion.z - antes.posicion.z;
        return dx * dx + dz * dz;
    };
    if (distancia2(intento) > distancia2(j) + 1e-5f) {
        j.posicion = intento.posicion;
        j.velocidad.x = intento.velocidad.x;
        j.velocidad.z = intento.velocidad.z;
    }
}

}  // namespace

void simular_jugador(Jugador& j, const EntradaJugador& entrada, const MundoColision& mundo,
                     const ConfigMovimiento& c, float dt) {
    // Dirección deseada en el plano horizontal (yaw 0 mira hacia -Z).
    const Vec3 adelante{-std::sin(j.yaw), 0.0f, -std::cos(j.yaw)};
    const Vec3 derecha{std::cos(j.yaw), 0.0f, -std::sin(j.yaw)};
    Vec3 deseada = adelante * entrada.adelante + derecha * entrada.lateral;
    const float largo = longitud(deseada);
    if (largo > 1.0f) deseada = deseada * (1.0f / largo);  // la diagonal no es más rápida
    const float rapidez_deseada = longitud(deseada) * c.velocidad_maxima;
    Vec3 direccion = deseada;
    if (largo > 1e-6f) direccion = deseada * (1.0f / longitud(deseada));

    if (j.en_suelo) {
        if (entrada.salto) {
            j.velocidad.y = c.velocidad_salto;
            j.en_suelo = false;
        } else {
            aplicar_friccion(j.velocidad, c, dt);
        }
    }
    if (rapidez_deseada > 0.0f) {
        if (j.en_suelo) acelerar(j.velocidad, direccion, rapidez_deseada, c.aceleracion_suelo, dt);
        else acelerar(j.velocidad, direccion, std::min(rapidez_deseada, c.limite_velocidad_aire),
                      c.aceleracion_aire, dt);
    }

    j.velocidad.y -= c.gravedad * dt;

    const bool estaba_en_suelo = j.en_suelo;
    j.en_suelo = false;
    const Jugador antes = j;
    const bool choco_x = mover_en_eje(j, 0, j.velocidad.x * dt, c, mundo);
    const bool choco_z = mover_en_eje(j, 2, j.velocidad.z * dt, c, mundo);
    if ((choco_x || choco_z) && estaba_en_suelo) intentar_subir_escalon(j, antes, c, mundo, dt);
    const bool bajando = j.velocidad.y < 0.0f;
    const bool choco_y = mover_en_eje(j, 1, j.velocidad.y * dt, c, mundo);
    if (choco_y && bajando) j.en_suelo = true;
}

}  // namespace helios
