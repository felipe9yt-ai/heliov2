#include "juego/escena.h"

#include <algorithm>
#include <cmath>

namespace helios {

int Escena::agregar_solido(const Caja& caja, const Vec3& color) {
    mundo.solidos.push_back(caja);
    colores.push_back(color);
    return static_cast<int>(mundo.solidos.size()) - 1;
}

Escena crear_escena() {
    Escena e;
    const Vec3 c_suelo{0.40f, 0.40f, 0.43f}, c_techo{0.55f, 0.55f, 0.58f};
    const Vec3 c_pared{0.72f, 0.68f, 0.60f}, c_pasillo{0.55f, 0.62f, 0.68f};
    const Vec3 c_sala_b{0.62f, 0.68f, 0.58f}, c_puerta{0.75f, 0.30f, 0.22f};
    const Vec3 c_caja{0.60f, 0.40f, 0.20f}, c_columna{0.30f, 0.45f, 0.65f};
    const Vec3 c_escalon{0.50f, 0.50f, 0.56f}, c_plataforma{0.45f, 0.55f, 0.78f};
    const Vec3 c_salida{0.40f, 0.55f, 0.45f};

    // Suelo continuo de todo el nivel.
    e.agregar_solido({{-9, -1, -33}, {9, 0, 7}}, c_suelo);

    // --- Sala A (inicio): interior x[-6,6] z[-6,6] y[0,4] ---
    e.agregar_solido({{-7, 4, -7}, {7, 5, 7}}, c_techo);
    e.agregar_solido({{-7, -1, -7}, {-6, 5, 7}}, c_pared);
    e.agregar_solido({{6, -1, -7}, {7, 5, 7}}, c_pared);
    e.agregar_solido({{-7, -1, 6}, {7, 5, 7}}, c_pared);
    e.agregar_solido({{-7, -1, -7}, {-1.5f, 5, -6}}, c_pared);
    e.agregar_solido({{1.5f, -1, -7}, {7, 5, -6}}, c_pared);
    e.agregar_solido({{-1.5f, 2.6f, -7}, {1.5f, 5, -6}}, c_pared);
    e.agregar_solido({{2.5f, 0, -4}, {4, 1.5f, -2.5f}}, c_caja);
    e.agregar_solido({{-5, 0, -4}, {-3.5f, 0.8f, -2.5f}}, c_caja);  // bajo: se sube saltando
    e.agregar_solido({{-4, 0, 2}, {-3, 4, 3}}, c_columna);

    Puerta puerta_a;
    puerta_a.cerrada = {{-1.5f, 0, -7}, {1.5f, 2.6f, -6}};
    puerta_a.desplazamiento = {0, 2.6f, 0};
    puerta_a.indice_solido = e.agregar_solido(puerta_a.cerrada, c_puerta);
    e.puertas.push_back(puerta_a);

    // --- Pasillo: interior x[-1.5,1.5] z[-15,-7] y[0,3] ---
    e.agregar_solido({{-2.5f, -1, -15}, {-1.5f, 4, -7}}, c_pasillo);
    e.agregar_solido({{1.5f, -1, -15}, {2.5f, 4, -7}}, c_pasillo);
    e.agregar_solido({{-2.5f, 3, -15}, {2.5f, 4, -7}}, c_pasillo);

    // --- Sala B: interior x[-8,8] z[-26,-15] y[0,5] ---
    e.agregar_solido({{-9, 5, -27}, {9, 6, -14}}, c_techo);
    e.agregar_solido({{-9, -1, -27}, {-8, 6, -14}}, c_sala_b);
    e.agregar_solido({{8, -1, -27}, {9, 6, -14}}, c_sala_b);
    e.agregar_solido({{-9, -1, -15}, {-1.5f, 6, -14}}, c_sala_b);
    e.agregar_solido({{1.5f, -1, -15}, {9, 6, -14}}, c_sala_b);
    e.agregar_solido({{-1.5f, 3, -15}, {1.5f, 6, -14}}, c_sala_b);
    e.agregar_solido({{-9, -1, -27}, {-1.5f, 6, -26}}, c_sala_b);
    e.agregar_solido({{1.5f, -1, -27}, {9, 6, -26}}, c_sala_b);
    e.agregar_solido({{-1.5f, 2.6f, -27}, {1.5f, 6, -26}}, c_sala_b);
    e.agregar_solido({{-6, 0, -19}, {-4.5f, 1.4f, -17.5f}}, c_caja);
    e.agregar_solido({{-1, 0, -21}, {0, 5, -20}}, c_columna);

    // Escalera de 6 escalones de 0.30 m hacia una plataforma de 1.80 m.
    for (int i = 0; i < 6; ++i) {
        const float z_cerca = -16.0f - 0.8f * static_cast<float>(i);
        e.agregar_solido({{4, 0, z_cerca - 0.8f}, {7, 0.3f * static_cast<float>(i + 1), z_cerca}}, c_escalon);
    }
    e.agregar_solido({{4, 0, -26}, {7, 1.8f, -20.8f}}, c_plataforma);

    Puerta puerta_d;
    puerta_d.cerrada = {{-1.5f, 0, -27}, {1.5f, 2.6f, -26}};
    puerta_d.desplazamiento = {0, 2.6f, 0};
    puerta_d.indice_solido = e.agregar_solido(puerta_d.cerrada, c_puerta);
    e.puertas.push_back(puerta_d);

    // --- Sala de salida: interior x[-1.5,1.5] z[-31,-27] y[0,4] ---
    e.agregar_solido({{-2.5f, -1, -32}, {-1.5f, 5, -27}}, c_salida);
    e.agregar_solido({{1.5f, -1, -32}, {2.5f, 5, -27}}, c_salida);
    e.agregar_solido({{-2.5f, -1, -32}, {2.5f, 5, -31}}, c_salida);
    e.agregar_solido({{-2.5f, 4, -32}, {2.5f, 5, -27}}, c_salida);

    // Interruptores: el 0 abre la puerta A (en la sala A), el 1 abre la puerta D (en la plataforma).
    e.interruptores.push_back({{{-3.0f, 1.3f, -6.0f}, {-2.6f, 1.9f, -5.9f}}, 0, false});
    e.interruptores.push_back({{{5.2f, 3.1f, -26.0f}, {5.8f, 3.7f, -25.9f}}, 1, false});

    e.zona_meta = {{-1, 0, -31}, {1, 2, -29}};
    e.inicio = {0.0f, 0.0f, 4.0f};
    return e;
}

void actualizar_puertas(Escena& escena, const Jugador& jugador, const ConfigMovimiento& config, float dt) {
    const Caja caja_jugador = caja_del_jugador(jugador, config);
    for (Puerta& p : escena.puertas) {
        const float objetivo = p.abierta ? 1.0f : 0.0f;
        const float paso = dt / p.duracion;
        float nueva = p.apertura;
        if (nueva < objetivo) nueva = std::min(nueva + paso, objetivo);
        else if (nueva > objetivo) nueva = std::max(nueva - paso, objetivo);

        Caja candidata = p.cerrada;
        candidata.minimo += p.desplazamiento * nueva;
        candidata.maximo += p.desplazamiento * nueva;
        if (nueva < p.apertura && cajas_se_superponen(caja_jugador, candidata)) continue;  // no aplastar

        p.apertura = nueva;
        escena.mundo.solidos[static_cast<std::size_t>(p.indice_solido)] = candidata;
    }
}

Vec3 posicion_ojos(const Jugador& j, const ConfigMovimiento& c) {
    return {j.posicion.x, j.posicion.y + c.altura_ojos, j.posicion.z};
}

Vec3 direccion_mirada(const Jugador& j) {
    const float cp = std::cos(j.pitch);
    return {-std::sin(j.yaw) * cp, std::sin(j.pitch), -std::cos(j.yaw) * cp};
}

int interruptor_apuntado(const Escena& escena, const Jugador& jugador, const ConfigMovimiento& config,
                         float alcance) {
    const Vec3 origen = posicion_ojos(jugador, config);
    const Vec3 dir = direccion_mirada(jugador);
    int mejor = -1;
    float mejor_t = alcance;
    for (std::size_t i = 0; i < escena.interruptores.size(); ++i) {
        float t = 0.0f;
        if (!rayo_contra_caja(origen, dir, escena.interruptores[i].caja, t) || t > mejor_t) continue;
        bool tapado = false;
        for (const Caja& solido : escena.mundo.solidos) {
            float ts = 0.0f;
            if (rayo_contra_caja(origen, dir, solido, ts) && ts < t - 1e-4f) { tapado = true; break; }
        }
        if (!tapado) { mejor = static_cast<int>(i); mejor_t = t; }
    }
    return mejor;
}

bool interactuar(Escena& escena, const Jugador& jugador, const ConfigMovimiento& config, float alcance) {
    const int i = interruptor_apuntado(escena, jugador, config, alcance);
    if (i < 0) return false;
    Interruptor& s = escena.interruptores[static_cast<std::size_t>(i)];
    s.activado = !s.activado;
    escena.puertas[static_cast<std::size_t>(s.puerta)].abierta = s.activado;
    return true;
}

bool jugador_en_meta(const Escena& escena, const Jugador& jugador, const ConfigMovimiento& config) {
    return cajas_se_superponen(caja_del_jugador(jugador, config), escena.zona_meta);
}

}  // namespace helios
