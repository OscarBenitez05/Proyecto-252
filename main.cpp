#include <raylib.h>
#include <stdlib.h>
#include <time.h>

// Estructura para controlar a cada topo
struct Topo {
    Vector2 posicion;
    bool activo;
    float tiempoVisible;
    float temporizador;
    int tipo; // 0 = Normal, 1 = Casco (2 golpes), 2 = Bomba
    int vidas;
};

int main(void) {
    // Configuración de la pantalla
    const int pantallaAncho = 1080;
    const int pantallaAlto = 720;
    InitWindow(pantallaAncho, pantallaAlto, "Proyecto 252 - Juego de Topos");

    // Inicializar el sistema de audio
    InitAudioDevice();

    // Cargar Texturas (Deben estar en la carpeta assets/textures/)
    Texture2D fondo = LoadTexture("assets/textures/fondo.png");
    Texture2D spriteTopoNormal = LoadTexture("assets/textures/topo_normal.png");
    Texture2D spriteTopoCasco = LoadTexture("assets/textures/topo_casco.png");
    Texture2D spriteTopoBomba = LoadTexture("assets/textures/topo_bomba.png");
    Texture2D mazo = LoadTexture("assets/textures/mazo.png");

    // Cargar Sonidos (Deben estar en la carpeta assets/sounds/)
    Sound sonidoGolpe = LoadSound("assets/sounds/golpe.wav");
    Sound sonidoExplosion = LoadSound("assets/sounds/explosion.wav");

    srand(time(NULL));

    // Configuración del tablero (3 madrigueras fijas de ejemplo)
    Topo topos[3];
    Vector2 posicionesMadrigueras[3] = {
        { 200, 450 },
        { 540, 450 },
        { 880, 450 }
    };

    for (int i = 0; i < 3; i++) {
        topos[i].posicion = posicionesMadrigueras[i];
        topos[i].activo = false;
        topos[i].temporizador = 0.0f;
        topos[i].tiempoVisible = 1.5f; // Segundos en pantalla
        topos[i].tipo = 0;
        topos[i].vidas = 1;
    }

    // Variables de control del juego
    int puntuacion = 0;
    int vidasJugador = 3;
    float tiempoJuego = 60.0f; // 1 minuto de partida
    float temporizadorSpawn = 0.0f;
    bool gameOver = false;

    SetTargetFPS(60);

    // Bucle principal del juego
    while (!WindowShouldClose()) {
        // --- ACTUALIZACIÓN DE LÓGICA ---
        if (!gameOver) {
            float deltaTime = GetFrameTime();
            tiempoJuego -= deltaTime;

            if (tiempoJuego <= 0 || vidasJugador <= 0) {
                gameOver = true;
            }

            // Temporizador para hacer aparecer un topo al azar
            temporizadorSpawn += deltaTime;
            if (temporizadorSpawn >= 1.2f) { // Intenta sacar un topo cada 1.2 segundos
                int indiceAzar = rand() % 3;
                if (!topos[indiceAzar].activo) {
                    topos[indiceAzar].activo = true;
                    topos[indiceAzar].temporizador = 0.0f;
                    
                    // Decidir tipo de topo de forma aleatoria
                    int probabilidad = rand() % 100;
                    if (probabilidad < 60) {
                        topos[indiceAzar].tipo = 0; // Normal
                        topos[indiceAzar].vidas = 1;
                    } else if (probabilidad < 85) {
                        topos[indiceAzar].tipo = 1; // Casco
                        topos[indiceAzar].vidas = 2;
                    } else {
                        topos[indiceAzar].tipo = 2; // Bomba
                        topos[indiceAzar].vidas = 1;
                    }
                }
                temporizadorSpawn = 0.0f;
            }

            // Controlar el comportamiento de los topos activos
            for (int i = 0; i < 3; i++) {
                if (topos[i].activo) {
                    topos[i].temporizador += deltaTime;
                    
                    // Si el jugador no lo golpea y se acaba su tiempo
                    if (topos[i].temporizador >= topos[i].tiempoVisible) {
                        topos[i].activo = false;
                        if (topos[i].tipo != 2) { // Si se escapa uno interactivo, resta vida
                            vidasJugador--;
                        }
                    }

                    // DETECTAR CLIC / GOLPE
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        Vector2 mousePos = GetMousePosition();
                        // Crear un área de colisión simple alrededor del topo
                        Rectangle rectTopo = { topos[i].posicion.x - 60, topos[i].posicion.y - 100, 120, 120 };
                        
                        if (CheckCollisionPointRec(mousePos, rectTopo)) {
                            topos[i].vidas--;
                            PlaySound(sonidoGolpe);

                            if (topos[i].vidas <= 0) {
                                topos[i].activo = false;
                                
                                // Asignar puntajes según tipo
                                if (topos[i].tipo == 0) puntuacion += 10;
                                else if (topos[i].tipo == 1) puntuacion += 25;
                                else if (topos[i].tipo == 2) {
                                    puntuacion -= 15;
                                    PlaySound(sonidoExplosion);
                                    vidasJugador--; // La bomba quita vida extra
                                }
                            }
                        }
                    }
                }
            }
        } else {
            // Reiniciar juego en Game Over con la tecla R
            if (IsKeyPressed(KEY_R)) {
                puntuacion = 0;
                vidasJugador = 3;
                tiempoJuego = 60.0f;
                gameOver = false;
                for (int i = 0; i < 3; i++) topos[i].activo = false;
            }
        }

        // --- RENDERIZADO (DIBUJO EN PANTALLA) ---
        BeginDrawing();
            ClearBackground(RAYWHITE);

            // 1. Dibujar Fondo de jardín
            DrawTexture(fondo, 0, 0, WHITE);

            // 2. Dibujar Topos activos
            for (int i = 0; i < 3; i++) {
                if (topos[i].activo) {
                    Texture2D spriteActual = spriteTopoNormal;
                    if (topos[i].tipo == 1) spriteActual = spriteTopoCasco;
                    if (topos[i].tipo == 2) spriteActual = spriteTopoBomba;

                    // Centrar el sprite en la posición de la madriguera
                    DrawTexture(spriteActual, topos[i].posicion.x - (spriteActual.width / 2), topos[i].posicion.y - spriteActual.height, WHITE);
                }
            }

            // 3. Interfaz de Usuario (UI)
            DrawText(TextFormat("PUNTOS: %04d", puntuacion), 20, 20, 30, GREEN);
            DrawText(TextFormat("TIEMPO: %02.0f", tiempoJuego), 450, 20, 30, BLACK);
            DrawText(TextFormat("VIDAS: %d", vidasJugador), 900, 20, 30, RED);

            // Pantalla de Game Over
            if (gameOver) {
                DrawRectangle(0, 0, pantallaAncho, pantallaAlto, Fade(BLACK, 0.8f));
                DrawText("¡FIN DEL JUEGO!", pantallaAncho/2 - 180, pantallaAlto/2 - 50, 40, RED);
                DrawText(TextFormat("Puntuación Final: %d", puntuacion), pantallaAncho/2 - 130, pantallaAlto/2 + 10, 25, WHITE);
                DrawText("Presiona 'R' para volver a jugar", pantallaAncho/2 - 190, pantallaAlto/2 + 60, 20, LIGHTGRAY);
            }

            // 4. Dibujar el Mazo en la posición del puntero del ratón
            Vector2 mousePos = GetMousePosition();
            DrawTexture(mazo, mousePos.x - 30, mousePos.y - 30, WHITE);

        EndDrawing();
    }

    // Descargar recursos de la memoria antes de cerrar
    UnloadTexture(fondo);
    UnloadTexture(spriteTopoNormal);
    UnloadTexture(spriteTopoCasco);
    UnloadTexture(spriteTopoBomba);
    UnloadTexture(mazo);
    UnloadSound(sonidoGolpe);
    UnloadSound(sonidoExplosion);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}