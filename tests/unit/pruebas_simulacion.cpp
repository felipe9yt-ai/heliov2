// Pruebas automáticas de la simulación del jugador. No usan gráficos ni SDL.
#include <cmath>
#include <cstdio>
#include <cstring>

#include "juego/escena.h"
#include "juego/habitacion.h"
#include "juego/jugador.h"

using namespace helios;

static int g_fallos = 0;
#define VERIFICAR(cond, mensaje)                                              \
    do {                                                                      \
        if (!(cond)) { std::printf("  FALLO: %s (linea %d)\n", mensaje, __LINE__); ++g_fallos; } \
    } while (0)

static const float DT = 1.0f / 60.0f;

static Jugador jugador_en(float x, float y, float z) {
    Jugador j;
    j.posicion = {x, y, z};
    return j;
}

static void correr(Jugador& j, const EntradaJugador& e, int pasos, const MundoColision& m,
                   const ConfigMovimiento& c) {
    for (int i = 0; i < pasos; ++i) simular_jugador(j, e, m, c, DT);
}

static void prueba_cae_y_se_apoya() {
    std::printf("cae_y_se_apoya\n");
    auto m = crear_habitacion_prueba(); ConfigMovimiento c;
    Jugador j = jugador_en(0, 2.0f, 4);  // cabeza a 3.8 m, bajo el techo (4 m)
    correr(j, {}, 180, m, c);
    VERIFICAR(std::fabs(j.posicion.y) < 1e-4f, "debe quedar apoyado en y=0");
    VERIFICAR(j.en_suelo, "debe estar en el suelo");
}

static void prueba_no_atraviesa_suelo_desde_altura() {
    std::printf("no_atraviesa_suelo_desde_altura\n");
    auto m = crear_habitacion_prueba(); ConfigMovimiento c;
    Jugador j = jugador_en(0, 2.2f, 4);  // cabeza a 4.0 m, pegada al techo
    correr(j, {}, 240, m, c);
    VERIFICAR(j.posicion.y > -1e-4f, "no debe caer por debajo del suelo");
}

static void prueba_no_atraviesa_pared() {
    std::printf("no_atraviesa_pared\n");
    auto m = crear_habitacion_prueba(); ConfigMovimiento c;
    Jugador j = jugador_en(-1.5f, 0, 4);
    EntradaJugador e; e.adelante = 1.0f;  // hacia -Z, hay caja y pared
    correr(j, e, 600, m, c);
    VERIFICAR(j.posicion.z - c.radio >= -6.0f - 1e-4f, "no debe pasar la pared norte");
    // Hacia +X contra la columna y la pared este
    Jugador k = jugador_en(0, 0, 0); k.yaw = -3.14159265f / 2.0f;  // mira hacia +X
    correr(k, e, 600, m, c);
    VERIFICAR(k.posicion.x + c.radio <= 6.0f + 1e-4f, "no debe pasar la pared este");
}

static void prueba_no_atraviesa_caja() {
    std::printf("no_atraviesa_caja\n");
    auto m = crear_habitacion_prueba(); ConfigMovimiento c;
    Jugador j = jugador_en(1.75f, 0, 2);  // justo frente a la caja (x 1..2.5, z -4..-2.5)
    EntradaJugador e; e.adelante = 1.0f;
    correr(j, e, 300, m, c);
    VERIFICAR(j.posicion.z - c.radio >= -2.5f - 1e-4f, "no debe entrar en la caja");
}

static void prueba_salto() {
    std::printf("salto_altura\n");
    auto m = crear_habitacion_prueba(); ConfigMovimiento c;
    Jugador j = jugador_en(0, 0, 4);
    correr(j, {}, 30, m, c);
    EntradaJugador e; e.salto = true;
    float maxima = 0.0f;
    simular_jugador(j, e, m, c, DT);
    for (int i = 0; i < 120; ++i) {
        simular_jugador(j, {}, m, c, DT);
        if (j.posicion.y > maxima) maxima = j.posicion.y;
    }
    const float ideal = c.velocidad_salto * c.velocidad_salto / (2.0f * c.gravedad);
    VERIFICAR(maxima > ideal * 0.85f && maxima < ideal * 1.05f, "altura de salto cercana a v^2/2g");
    VERIFICAR(j.en_suelo, "debe volver a aterrizar");
}

static void prueba_velocidad_maxima_y_diagonal() {
    std::printf("velocidad_maxima_y_diagonal\n");
    auto m = crear_habitacion_prueba(); ConfigMovimiento c;
    Jugador a = jugador_en(0, 0, 4);
    EntradaJugador e; e.adelante = 1.0f;
    correr(a, e, 30, m, c);  // 0.5 s, sin llegar a la pared
    float va = std::sqrt(a.velocidad.x * a.velocidad.x + a.velocidad.z * a.velocidad.z);
    VERIFICAR(std::fabs(va - c.velocidad_maxima) < c.velocidad_maxima * 0.01f, "alcanza la velocidad máxima");
    Jugador b = jugador_en(0, 0, 4);
    EntradaJugador d; d.adelante = 1.0f; d.lateral = 1.0f;
    correr(b, d, 30, m, c);
    float vb = std::sqrt(b.velocidad.x * b.velocidad.x + b.velocidad.z * b.velocidad.z);
    VERIFICAR(vb <= c.velocidad_maxima * 1.01f, "la diagonal no supera la velocidad máxima");
}

static void prueba_friccion_frena() {
    std::printf("friccion_frena\n");
    auto m = crear_habitacion_prueba(); ConfigMovimiento c;
    Jugador j = jugador_en(0, 0, 4);
    EntradaJugador e; e.adelante = 1.0f;
    correr(j, e, 30, m, c);
    correr(j, {}, 120, m, c);
    VERIFICAR(std::fabs(j.velocidad.x) + std::fabs(j.velocidad.z) < 1e-3f, "debe frenar por completo");
}

static void prueba_determinismo() {
    std::printf("determinismo\n");
    auto m = crear_habitacion_prueba(); ConfigMovimiento c;
    Jugador a = jugador_en(0, 0, 4), b = jugador_en(0, 0, 4);
    for (int i = 0; i < 600; ++i) {
        EntradaJugador e;
        e.adelante = (i % 90 < 60) ? 1.0f : 0.0f;
        e.lateral = (i % 70 < 20) ? -1.0f : 0.0f;
        e.salto = (i % 100 == 0);
        a.yaw = b.yaw = 0.01f * i;
        simular_jugador(a, e, m, c, DT);
        simular_jugador(b, e, m, c, DT);
    }
    VERIFICAR(std::memcmp(&a, &b, sizeof(Jugador)) == 0, "dos corridas idénticas dan el mismo resultado");
}


// ---------------------------------------------------------------- v0.2

static MundoColision mundo_con_escalones() {
    MundoColision m;
    m.solidos.push_back({{-5, -1, -8}, {5, 0, 8}});            // suelo
    for (int i = 0; i < 4; ++i)                                   // 4 escalones de 0.25 m
        m.solidos.push_back({{-2, 0, -1.0f - i}, {2, 0.25f * (i + 1), -static_cast<float>(i)}});
    m.solidos.push_back({{-2, 0, -5}, {2, 1.0f, -4}});            // plataforma
    m.solidos.push_back({{-3, -1, -6}, {3, 3, -5}});              // pared al fondo
    return m;
}

static void prueba_sube_escalones() {
    std::printf("sube_escalones\n");
    auto m = mundo_con_escalones(); ConfigMovimiento c;
    Jugador j = jugador_en(0, 0, 2);
    EntradaJugador e; e.adelante = 1.0f;
    correr(j, e, 240, m, c);
    VERIFICAR(std::fabs(j.posicion.y - 1.0f) < 1e-3f, "debe quedar sobre la plataforma (y=1.0)");
    VERIFICAR(j.en_suelo, "debe estar apoyado");
}

static void prueba_obstaculo_alto_no_se_sube() {
    std::printf("obstaculo_alto_no_se_sube\n");
    MundoColision m;
    m.solidos.push_back({{-5, -1, -8}, {5, 0, 8}});
    m.solidos.push_back({{-2, 0, -1}, {2, 0.8f, 0}});  // 0.8 m > altura_escalon
    ConfigMovimiento c;
    Jugador j = jugador_en(0, 0, 2);
    EntradaJugador e; e.adelante = 1.0f;
    correr(j, e, 180, m, c);
    VERIFICAR(j.posicion.y < 0.01f, "no debe trepar un obstáculo de 0.8 m caminando");
    VERIFICAR(j.posicion.z - c.radio >= -1e-4f + 0.0f, "debe quedar frenado frente al obstáculo");
}

static MundoColision mundo_con_bloque_bajo() {
    MundoColision m;
    m.solidos.push_back({{-5, -1, -8}, {5, 0, 8}});
    m.solidos.push_back({{-2, 0, -1}, {2, 0.8f, 0}});  // bloque de 0.8 m, profundidad 1 m
    return m;
}

static void prueba_salto_sobre_obstaculo() {
    std::printf("salto_sobre_obstaculo\n");
    auto m = mundo_con_bloque_bajo(); ConfigMovimiento c;
    // Desde parado, pegado al bloque: el control aéreo es limitado pero alcanza para subir.
    Jugador j = jugador_en(0, 0, 0.65f);
    correr(j, {}, 10, m, c);
    EntradaJugador e; e.adelante = 1.0f; e.salto = true;
    bool estuvo_arriba = false;
    for (int i = 0; i < 60; ++i) {
        simular_jugador(j, e, m, c, DT);
        if (j.en_suelo && std::fabs(j.posicion.y - 0.8f) < 1e-3f) estuvo_arriba = true;
        e.salto = false;
    }
    VERIFICAR(estuvo_arriba, "saltando junto al bloque debe quedar apoyado encima (y=0.8)");
}

static void prueba_salto_con_carrera_pasa_por_encima() {
    std::printf("salto_con_carrera\n");
    auto m = mundo_con_bloque_bajo(); ConfigMovimiento c;
    Jugador j = jugador_en(0, 0, 4.0f);
    EntradaJugador e; e.adelante = 1.0f;
    correr(j, e, 30, m, c);          // toma carrera
    e.salto = true;
    correr(j, e, 1, m, c);
    e.salto = false;
    correr(j, e, 120, m, c);
    VERIFICAR(j.posicion.z + c.radio < -1.0f, "con carrera debe pasar por encima y quedar del otro lado");
    VERIFICAR(j.posicion.y < 0.01f, "y terminar en el suelo");
}

static void prueba_puerta_cerrada_bloquea_y_abierta_deja_pasar() {
    std::printf("puerta_bloquea_y_abre\n");
    Escena esc = crear_escena(); ConfigMovimiento c;
    Jugador j = jugador_en(0, 0, -3);
    EntradaJugador e; e.adelante = 1.0f;
    for (int i = 0; i < 240; ++i) {
        simular_jugador(j, e, esc.mundo, c, DT);
        actualizar_puertas(esc, j, c, DT);
    }
    VERIFICAR(j.posicion.z - c.radio >= -6.0f - 1e-3f, "con la puerta cerrada no se puede pasar");

    esc.puertas[0].abierta = true;
    for (int i = 0; i < 90; ++i) actualizar_puertas(esc, j, c, DT);
    VERIFICAR(esc.puertas[0].apertura >= 0.999f, "la puerta debe abrirse por completo");
    for (int i = 0; i < 240; ++i) {
        simular_jugador(j, e, esc.mundo, c, DT);
        actualizar_puertas(esc, j, c, DT);
    }
    VERIFICAR(j.posicion.z < -9.0f, "con la puerta abierta se debe poder entrar al pasillo");
}

static void prueba_puerta_no_aplasta() {
    std::printf("puerta_no_aplasta\n");
    Escena esc = crear_escena(); ConfigMovimiento c;
    esc.puertas[0].abierta = true;
    Jugador j = jugador_en(0, 0, -6.5f);
    for (int i = 0; i < 90; ++i) actualizar_puertas(esc, j, c, DT);
    esc.puertas[0].abierta = false;
    for (int i = 0; i < 240; ++i) actualizar_puertas(esc, j, c, DT);
    const Caja puerta = esc.mundo.solidos[static_cast<std::size_t>(esc.puertas[0].indice_solido)];
    VERIFICAR(!cajas_se_superponen(caja_del_jugador(j, c), puerta), "la puerta no debe superponerse al jugador");
    VERIFICAR(esc.puertas[0].apertura > 0.5f, "la puerta debe quedar detenida, sin cerrarse del todo");
}

static void prueba_interruptor() {
    std::printf("interruptor\n");
    Escena esc = crear_escena(); ConfigMovimiento c;
    Jugador j = jugador_en(-2.8f, 0, -3.0f);  // mira hacia -Z, frente al interruptor 0
    VERIFICAR(interruptor_apuntado(esc, j, c, 3.5f) == 0, "debe apuntar al interruptor 0");
    VERIFICAR(interruptor_apuntado(esc, j, c, 1.0f) == -1, "fuera de alcance no debe detectarlo");
    j.yaw = 3.14159265f;  // mirando hacia +Z, de espaldas
    VERIFICAR(interruptor_apuntado(esc, j, c, 3.5f) == -1, "de espaldas no debe detectarlo");
    j.yaw = 0.0f;
    VERIFICAR(interactuar(esc, j, c, 3.5f), "debe accionarse");
    VERIFICAR(esc.puertas[0].abierta, "la puerta A debe quedar pedida como abierta");
    for (int i = 0; i < 90; ++i) actualizar_puertas(esc, j, c, DT);
    VERIFICAR(esc.mundo.solidos[static_cast<std::size_t>(esc.puertas[0].indice_solido)].minimo.y > 2.5f,
              "el sólido de la puerta debe haberse elevado");
    // Una pared entre medio tapa el interruptor
    Jugador k = jugador_en(-2.8f, 0, 0.0f);
    k.posicion.x = -2.8f;
    esc.mundo.solidos.push_back({{-4, 0, -4}, {-1, 3, -3.5f}});
    VERIFICAR(interruptor_apuntado(esc, k, c, 8.0f) == -1, "un sólido interpuesto lo debe tapar");
}

static void prueba_escena_valida() {
    std::printf("escena_valida\n");
    Escena esc = crear_escena(); ConfigMovimiento c;
    for (auto& p : esc.puertas) {
        p.abierta = true;
        p.apertura = 1.0f;
        esc.mundo.solidos[static_cast<std::size_t>(p.indice_solido)] = {
            {p.cerrada.minimo + p.desplazamiento}, {p.cerrada.maximo + p.desplazamiento}};
    }
    const Vec3 puntos[] = {esc.inicio, {0, 0, -10}, {0, 0, -17}, {5.5f, 1.8f, -24}, {0, 0, -29}, {0, 0, -4}};
    for (const Vec3& p : puntos) {
        Jugador j = jugador_en(p.x, p.y, p.z);
        bool libre = true;
        for (const Caja& s : esc.mundo.solidos)
            if (cajas_se_superponen(caja_del_jugador(j, c), s)) libre = false;
        VERIFICAR(libre, "el punto de la escena debe estar libre de sólidos");
    }
    Jugador j = jugador_en(0, 0, -30);
    VERIFICAR(jugador_en_meta(esc, j, c), "en la sala final se llega a la meta");
    Jugador k = jugador_en(0, 0, 4);
    VERIFICAR(!jugador_en_meta(esc, k, c), "al inicio no se está en la meta");
}

static void prueba_sube_escalera_de_la_escena() {
    std::printf("sube_escalera_de_la_escena\n");
    Escena esc = crear_escena(); ConfigMovimiento c;
    Jugador j = jugador_en(5.5f, 0, -15.5f);  // en la sala B, frente a la escalera, mirando a -Z
    EntradaJugador e; e.adelante = 1.0f;
    correr(j, e, 400, esc.mundo, c);
    VERIFICAR(std::fabs(j.posicion.y - 1.8f) < 1e-3f, "debe quedar sobre la plataforma (y=1.8)");
    VERIFICAR(j.en_suelo, "debe estar apoyado en la plataforma");
    Jugador desde_plataforma = jugador_en(5.5f, 1.8f, -24.5f);
    VERIFICAR(interruptor_apuntado(esc, desde_plataforma, c, 3.0f) == 1, "desde la plataforma se ve el interruptor 1");
}

int main() {
    prueba_cae_y_se_apoya();
    prueba_no_atraviesa_suelo_desde_altura();
    prueba_no_atraviesa_pared();
    prueba_no_atraviesa_caja();
    prueba_salto();
    prueba_velocidad_maxima_y_diagonal();
    prueba_friccion_frena();
    prueba_determinismo();
    prueba_sube_escalones();
    prueba_obstaculo_alto_no_se_sube();
    prueba_salto_sobre_obstaculo();
    prueba_salto_con_carrera_pasa_por_encima();
    prueba_puerta_cerrada_bloquea_y_abierta_deja_pasar();
    prueba_puerta_no_aplasta();
    prueba_interruptor();
    prueba_escena_valida();
    prueba_sube_escalera_de_la_escena();
    if (g_fallos == 0) std::printf("\nTODAS LAS PRUEBAS PASARON\n");
    else std::printf("\n%d verificaciones fallaron\n", g_fallos);
    return g_fallos == 0 ? 0 : 1;
}
