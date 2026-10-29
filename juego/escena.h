#pragma once
// Escena de la v0.2: geometría, puertas, interruptores y meta.
#include <vector>

#include "juego/jugador.h"

namespace helios {

struct Puerta {
    int indice_solido = 0;       // posición dentro de mundo.solidos
    Caja cerrada;                // caja cuando está cerrada
    Vec3 desplazamiento;         // cuánto se mueve al abrirse por completo
    float apertura = 0.0f;       // 0 = cerrada, 1 = abierta
    bool abierta = false;        // estado deseado
    float duracion = 1.2f;       // segundos para abrir o cerrar
};

struct Interruptor {
    Caja caja;
    int puerta = 0;              // índice en Escena::puertas
    bool activado = false;
};

struct Escena {
    MundoColision mundo;
    std::vector<Vec3> colores;   // un color por sólido (mismo orden que mundo.solidos)
    std::vector<Puerta> puertas;
    std::vector<Interruptor> interruptores;
    Caja zona_meta;
    Vec3 inicio;

    int agregar_solido(const Caja& caja, const Vec3& color);
};

Escena crear_escena();

// Mueve las puertas. Una puerta que se está cerrando se detiene si va a aplastar al jugador.
void actualizar_puertas(Escena& escena, const Jugador& jugador, const ConfigMovimiento& config, float dt);

Vec3 posicion_ojos(const Jugador& jugador, const ConfigMovimiento& config);
Vec3 direccion_mirada(const Jugador& jugador);

// Índice del interruptor al que apunta el jugador (a la vista y dentro del alcance), o -1.
int interruptor_apuntado(const Escena& escena, const Jugador& jugador, const ConfigMovimiento& config,
                         float alcance);

// Acciona el interruptor apuntado. Devuelve true si accionó alguno.
bool interactuar(Escena& escena, const Jugador& jugador, const ConfigMovimiento& config, float alcance);

bool jugador_en_meta(const Escena& escena, const Jugador& jugador, const ConfigMovimiento& config);

}  // namespace helios
