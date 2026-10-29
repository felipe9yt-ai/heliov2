# Decisiones

- 2026-10-29: OpenGL de función fija en v0.1 (sin cargador de extensiones). Pasar a shaders (GL 3.3 + GLAD) en v0.2+.
- 2026-10-29: SDL2 enlazado de forma estática y runtime de MSVC estático: el .exe no necesita DLL.
- 2026-10-29: simulación a paso fijo de 60 Hz; todos los parámetros de movimiento son DECISIÓN PROPIA, sin calibrar.
- 2026-10-29: la lógica de simulación no depende de SDL ni de OpenGL, para poder probarla automáticamente.
- 2026-10-29 (v0.2): subir escalones caminando = elevar, avanzar y bajar; se acepta solo si avanza más que el movimiento normal y aterriza más alto. Altura máxima 0.45 m (DECISIÓN PROPIA).
- 2026-10-29 (v0.2): el control aéreo limita la velocidad deseada a 0.7 m/s (estilo Quake): desde parado no se salta lejos, con carrera sí.
- 2026-10-29 (v0.2): puertas = un sólido que se desplaza hacia arriba; no se cierran si van a aplastar al jugador.
- 2026-10-29 (v0.2): caras subdivididas cada ~1 m porque la luz de función fija se calcula por vértice.
