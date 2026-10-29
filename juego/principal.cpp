// Helios Engine v0.2: escenario con salas, puertas, interruptores y escalera.
// Render: OpenGL de función fija (1.x) a propósito: no necesita cargador de
// extensiones y es lo más fiable para un prototipo. Los shaders llegan después.
#include <SDL.h>
#include <SDL_opengl.h>

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "juego/escena.h"
#include "juego/jugador.h"

using namespace helios;

namespace {

constexpr float PASO_FIJO = 1.0f / 60.0f;  // la simulación avanza siempre a 60 Hz
constexpr float SENSIBILIDAD = 0.0022f;    // radianes por píxel de ratón
constexpr float ALCANCE_INTERACCION = 3.0f;
constexpr float PI = 3.14159265f;

struct LuzPuntual {
    float x, y, z;
    float r, g, b;
};
const LuzPuntual LUCES[] = {
    {0.0f, 3.4f, 0.0f, 1.00f, 0.94f, 0.82f},    // sala A
    {0.0f, 2.7f, -11.0f, 0.80f, 0.92f, 1.00f},  // pasillo
    {0.0f, 4.4f, -21.0f, 0.92f, 1.00f, 0.90f},  // sala B
};
constexpr int CANTIDAD_LUCES = 3;

// Dibuja una cara rectangular dividida en celdas de ~1 m. La iluminación de función fija
// se calcula por vértice, así que las caras grandes necesitan vértices intermedios.
// Las celdas alternan un 7% de brillo para dar sensación de baldosas y de profundidad.
void dibujar_cara(const Vec3& origen, const Vec3& u, const Vec3& v, const Vec3& normal, const Vec3& color) {
    const int nu = std::max(1, static_cast<int>(std::ceil(longitud(u))));
    const int nv = std::max(1, static_cast<int>(std::ceil(longitud(v))));
    glNormal3f(normal.x, normal.y, normal.z);
    glBegin(GL_QUADS);
    for (int i = 0; i < nu; ++i) {
        for (int j = 0; j < nv; ++j) {
            const float a0 = static_cast<float>(i) / nu, a1 = static_cast<float>(i + 1) / nu;
            const float b0 = static_cast<float>(j) / nv, b1 = static_cast<float>(j + 1) / nv;
            const float k = ((i + j) & 1) ? 0.93f : 1.0f;
            glColor3f(color.x * k, color.y * k, color.z * k);
            const Vec3 p00 = origen + u * a0 + v * b0, p10 = origen + u * a1 + v * b0;
            const Vec3 p11 = origen + u * a1 + v * b1, p01 = origen + u * a0 + v * b1;
            glVertex3f(p00.x, p00.y, p00.z);
            glVertex3f(p10.x, p10.y, p10.z);
            glVertex3f(p11.x, p11.y, p11.z);
            glVertex3f(p01.x, p01.y, p01.z);
        }
    }
    glEnd();
}

void dibujar_caja(const Caja& c, const Vec3& color) {
    const Vec3 a = c.minimo, b = c.maximo;
    const float dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
    dibujar_cara({a.x, a.y, b.z}, {dx, 0, 0}, {0, dy, 0}, {0, 0, 1}, color);
    dibujar_cara({a.x, a.y, a.z}, {dx, 0, 0}, {0, dy, 0}, {0, 0, -1}, color);
    dibujar_cara({b.x, a.y, a.z}, {0, 0, dz}, {0, dy, 0}, {1, 0, 0}, color);
    dibujar_cara({a.x, a.y, a.z}, {0, 0, dz}, {0, dy, 0}, {-1, 0, 0}, color);
    dibujar_cara({a.x, b.y, a.z}, {dx, 0, 0}, {0, 0, dz}, {0, 1, 0}, color);
    dibujar_cara({a.x, a.y, a.z}, {dx, 0, 0}, {0, 0, dz}, {0, -1, 0}, color);
}

void configurar_proyeccion(int ancho, int alto) {
    const float cerca = 0.05f, lejos = 100.0f, fov = 75.0f * PI / 180.0f;
    const float arriba = cerca * std::tan(fov * 0.5f);
    const float derecha = arriba * (static_cast<float>(ancho) / static_cast<float>(std::max(alto, 1)));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-derecha, derecha, -arriba, arriba, cerca, lejos);
    glMatrixMode(GL_MODELVIEW);
}

void dibujar_mira(int ancho, int alto, bool resaltada) {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, ancho, alto, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    const float cx = ancho * 0.5f, cy = alto * 0.5f;
    const float largo = resaltada ? 12.0f : 8.0f;
    if (resaltada) glColor3f(0.2f, 1.0f, 0.3f); else glColor3f(1, 1, 1);
    glBegin(GL_LINES);
    glVertex2f(cx - largo, cy); glVertex2f(cx + largo, cy);
    glVertex2f(cx, cy - largo); glVertex2f(cx, cy + largo);
    glEnd();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

}  // namespace

int main(int, char**) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "[error] No se pudo inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_Window* ventana = SDL_CreateWindow("Helios Engine v0.2", SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED, 1280, 720,
                                           SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!ventana) {
        std::fprintf(stderr, "[error] No se pudo crear la ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_GLContext contexto = SDL_GL_CreateContext(ventana);
    if (!contexto) {
        std::fprintf(stderr, "[error] No se pudo crear el contexto OpenGL: %s\n", SDL_GetError());
        SDL_DestroyWindow(ventana);
        SDL_Quit();
        return 1;
    }
    SDL_GL_SetSwapInterval(1);  // sincronización vertical
    SDL_SetRelativeMouseMode(SDL_TRUE);
    std::printf("[info] OpenGL: %s | %s\n", reinterpret_cast<const char*>(glGetString(GL_VERSION)),
                reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    Escena escena = crear_escena();
    const ConfigMovimiento config;
    Jugador jugador;
    jugador.posicion = escena.inicio;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    const GLfloat ambiente_global[] = {0.22f, 0.22f, 0.25f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambiente_global);
    for (int i = 0; i < CANTIDAD_LUCES; ++i) {
        const GLenum id = GL_LIGHT0 + i;
        const GLfloat difusa[] = {LUCES[i].r, LUCES[i].g, LUCES[i].b, 1.0f};
        const GLfloat ambiente[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glEnable(id);
        glLightfv(id, GL_DIFFUSE, difusa);
        glLightfv(id, GL_AMBIENT, ambiente);
        glLightf(id, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(id, GL_LINEAR_ATTENUATION, 0.08f);
        glLightf(id, GL_QUADRATIC_ATTENUATION, 0.02f);
    }
    glClearColor(0.05f, 0.06f, 0.08f, 1.0f);

    bool ejecutando = true;
    bool nivel_completado = false;
    float acumulador = 0.0f;
    Uint64 anterior = SDL_GetPerformanceCounter();
    float tiempo_fps = 0.0f;
    int cuadros = 0;
    bool salto_pendiente = false;

    while (ejecutando) {
        SDL_Event evento;
        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_QUIT) ejecutando = false;
            if (evento.type == SDL_KEYDOWN && !evento.key.repeat) {
                if (evento.key.keysym.sym == SDLK_ESCAPE) ejecutando = false;
                if (evento.key.keysym.sym == SDLK_SPACE) salto_pendiente = true;
                if (evento.key.keysym.sym == SDLK_e) interactuar(escena, jugador, config, ALCANCE_INTERACCION);
                if (evento.key.keysym.sym == SDLK_r) {  // reiniciar nivel
                    escena = crear_escena();
                    jugador = Jugador();
                    jugador.posicion = escena.inicio;
                    nivel_completado = false;
                }
            }
            if (evento.type == SDL_MOUSEMOTION) {
                jugador.yaw -= evento.motion.xrel * SENSIBILIDAD;
                jugador.pitch -= evento.motion.yrel * SENSIBILIDAD;
                jugador.pitch = std::clamp(jugador.pitch, -1.55f, 1.55f);
            }
        }

        const Uint64 ahora = SDL_GetPerformanceCounter();
        float delta = static_cast<float>(ahora - anterior) / static_cast<float>(SDL_GetPerformanceFrequency());
        anterior = ahora;
        delta = std::min(delta, 0.25f);  // evita la "espiral de la muerte" tras una pausa larga
        acumulador += delta;

        const Uint8* teclas = SDL_GetKeyboardState(nullptr);
        EntradaJugador entrada;
        entrada.adelante = (teclas[SDL_SCANCODE_W] ? 1.0f : 0.0f) - (teclas[SDL_SCANCODE_S] ? 1.0f : 0.0f);
        entrada.lateral = (teclas[SDL_SCANCODE_D] ? 1.0f : 0.0f) - (teclas[SDL_SCANCODE_A] ? 1.0f : 0.0f);

        while (acumulador >= PASO_FIJO) {
            entrada.salto = salto_pendiente || teclas[SDL_SCANCODE_SPACE];
            salto_pendiente = false;
            simular_jugador(jugador, entrada, escena.mundo, config, PASO_FIJO);
            actualizar_puertas(escena, jugador, config, PASO_FIJO);
            acumulador -= PASO_FIJO;
        }
        if (!nivel_completado && jugador_en_meta(escena, jugador, config)) {
            nivel_completado = true;
            std::printf("[info] Nivel completado\n");
        }

        int ancho = 0, alto = 0;
        SDL_GL_GetDrawableSize(ventana, &ancho, &alto);
        glViewport(0, 0, ancho, alto);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        configurar_proyeccion(ancho, alto);
        glLoadIdentity();
        glRotatef(-jugador.pitch * 180.0f / PI, 1, 0, 0);
        glRotatef(-jugador.yaw * 180.0f / PI, 0, 1, 0);
        glTranslatef(-jugador.posicion.x, -(jugador.posicion.y + config.altura_ojos), -jugador.posicion.z);
        for (int i = 0; i < CANTIDAD_LUCES; ++i) {
            const GLfloat posicion[] = {LUCES[i].x, LUCES[i].y, LUCES[i].z, 1.0f};
            glLightfv(GL_LIGHT0 + i, GL_POSITION, posicion);
        }

        for (std::size_t i = 0; i < escena.mundo.solidos.size(); ++i)
            dibujar_caja(escena.mundo.solidos[i], escena.colores[i]);
        for (const Interruptor& s : escena.interruptores)
            dibujar_caja(s.caja, s.activado ? Vec3{0.2f, 0.9f, 0.3f} : Vec3{0.95f, 0.8f, 0.1f});
        dibujar_caja({{escena.zona_meta.minimo.x, 0.0f, escena.zona_meta.minimo.z},
                      {escena.zona_meta.maximo.x, 0.05f, escena.zona_meta.maximo.z}},
                     nivel_completado ? Vec3{0.3f, 1.0f, 0.5f} : Vec3{0.15f, 0.7f, 0.3f});

        const bool apuntando = interruptor_apuntado(escena, jugador, config, ALCANCE_INTERACCION) >= 0;
        dibujar_mira(ancho, alto, apuntando);
        SDL_GL_SwapWindow(ventana);

        ++cuadros;
        tiempo_fps += delta;
        if (tiempo_fps >= 1.0f) {
            char titulo[192];
            std::snprintf(titulo, sizeof(titulo), "Helios Engine v0.2 | %d FPS | pos %.1f %.1f %.1f%s",
                          cuadros, jugador.posicion.x, jugador.posicion.y, jugador.posicion.z,
                          nivel_completado ? " | NIVEL COMPLETADO (R reinicia)" : "");
            SDL_SetWindowTitle(ventana, titulo);
            cuadros = 0;
            tiempo_fps = 0.0f;
        }
    }

    SDL_GL_DeleteContext(contexto);
    SDL_DestroyWindow(ventana);
    SDL_Quit();
    return 0;
}
