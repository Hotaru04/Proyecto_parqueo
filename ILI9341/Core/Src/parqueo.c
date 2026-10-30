/* Interfaz animada de parqueo para LCD ILI9341 320x240 (horizontal).
 * Utiliza SOLO las primitivas del controlador original: FillRect y LCD_Print.
 * El modo demo se controla con HAL_GetTick(): no bloquea con HAL_Delay().
 */
#include "parqueo.h"
#include "ili9341.h"
#include "main.h"

#define ESPACIOS       8U
#define PANTALLA_W     320
#define PANTALLA_H     240
#define ESPACIO_W      74
#define ESPACIO_H      54
#define ESPACIO_Y_SUP  70
#define ESPACIO_Y_INF  163
#define CARRIL_Y       129
#define CARRIL_H       32
#define CARRO_H        27
#define CARRO_W        23
#define INTERVALO_MS   50U
#define PAUSA_MS       1250U

/* Colores RGB565 */
#define COL_FONDO      0x0842
#define COL_CABECERA   0x10A5
#define COL_PANEL      0x18C6
#define COL_TARJETA    0x10C5
#define COL_TARJETA_OC 0x3106
#define COL_CARRIL     0x3186
#define COL_LINEA      0xCE79
#define COL_BLANCO     0xFFFF
#define COL_GRIS       0xA534
#define COL_VERDE      0x05E9
#define COL_ROJO       0xF9C8
#define COL_AMARILLO   0xFE80
#define COL_CIAN       0x4E5F
#define COL_CRISTAL    0xA6FF
#define COL_OSCURO     0x0842

/* Posicion horizontal de los 4 espacios de cada fila. */
static const int16_t espacio_x[4] = { 6, 84, 162, 240 };
/* Colores distintos para que se aprecie el movimiento de cada carro. */
static const uint16_t carro_color[8] = {
    0xFC00, 0x3DFF, 0xFE40, 0xF81F,
    0x065F, 0xFD20, 0x9C7F, 0x07F0
};

static uint8_t ocupado[ESPACIOS];
static uint8_t en_demo;
static uint8_t demo_indice;
static uint8_t demo_llenando;
static uint32_t proximo_movimiento;
static uint32_t ultimo_paso;
static uint8_t objetivo;
static int16_t auto_x;
static int16_t auto_y;

typedef enum {
    PARADO = 0,
    ENTRANDO_CARRIL,
    ENTRANDO_ESPACIO,
    SALIENDO_ESPACIO,
    SALIENDO_CARRIL
} TipoAnimacion;
static TipoAnimacion animacion;

static int16_t casilla_x(uint8_t e) { return espacio_x[e % 4U]; }
static int16_t casilla_y(uint8_t e) {
    return (e < 4U) ? ESPACIO_Y_SUP : ESPACIO_Y_INF;
}
static int16_t coche_x(uint8_t e) { return casilla_x(e) + 25; }
static int16_t coche_y(uint8_t e) { return casilla_y(e) + 24; }

/* Evita enviar a LCD coordenadas negativas (entrada/salida por los bordes). */
static void caja(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > PANTALLA_W) w = PANTALLA_W - x;
    if (y + h > PANTALLA_H) h = PANTALLA_H - y;
    if (w > 0 && h > 0)
        FillRect((unsigned int)x, (unsigned int)y,
                 (unsigned int)w, (unsigned int)h, color);
}

static void coche_vertical(int16_t x, int16_t y, uint16_t color)
{
    caja(x + 3, y,     17, 27, COL_OSCURO);  /* sombra */
    caja(x - 1, y + 5, 4,  7, COL_GRIS);    /* ruedas izquierdas */
    caja(x - 1, y + 19,4,  6, COL_GRIS);
    caja(x + 20,y + 5, 4,  7, COL_GRIS);    /* ruedas derechas */
    caja(x + 20,y + 19,4,  6, COL_GRIS);
    caja(x + 3, y + 1, 17, 25, color);       /* carroceria */
    caja(x + 5, y + 5, 13,  6, COL_CRISTAL); /* parabrisas */
    caja(x + 5, y + 17,13,  5, COL_CABECERA);
    caja(x + 5, y + 2,  4,  2, COL_BLANCO); /* luces */
    caja(x + 14,y + 2,  4,  2, COL_BLANCO);
}

static void coche_horizontal(int16_t x, int16_t y, uint16_t color)
{
    caja(x + 3,  y - 2, 7,  3, COL_OSCURO);
    caja(x + 24, y - 2, 7,  3, COL_OSCURO);
    caja(x + 3,  y + 14,7,  3, COL_OSCURO);
    caja(x + 24, y + 14,7,  3, COL_OSCURO);
    caja(x,      y,     34,14, color);
    caja(x + 8,  y + 2, 15,10, COL_CRISTAL);
    caja(x + 12, y + 2, 2, 10, COL_CABECERA);
    caja(x + 31, y + 2,  3, 3, COL_BLANCO);
    caja(x + 31, y + 9,  3, 3, COL_BLANCO);
}

static void dibujar_espacio(uint8_t e)
{
    int16_t x = casilla_x(e);
    int16_t y = casilla_y(e);
    uint8_t esta_ocupado = ocupado[e];
    uint16_t borde = esta_ocupado ? COL_ROJO : COL_VERDE;
    char nombre[] = "P1";
    nombre[1] = (char)('1' + e);

    caja(x, y, ESPACIO_W, ESPACIO_H,
         esta_ocupado ? COL_TARJETA_OC : COL_TARJETA);
    caja(x,               y,                ESPACIO_W,2, borde);
    caja(x,               y + ESPACIO_H - 2,ESPACIO_W,2, borde);
    caja(x,               y,                2, ESPACIO_H,borde);
    caja(x + ESPACIO_W - 2,y,                2, ESPACIO_H,borde);
    LCD_Print(nombre, x + 7, y + 5, 1, COL_BLANCO,
              esta_ocupado ? COL_TARJETA_OC : COL_TARJETA);
    LCD_Print(esta_ocupado ? "OCUP" : "LIBRE", x + 28, y + 5,
              1, borde, esta_ocupado ? COL_TARJETA_OC : COL_TARJETA);
    if (esta_ocupado) {
        coche_vertical(coche_x(e), coche_y(e), carro_color[e]);
    } else {
        /* Marca de plaza vacia. */
        caja(x + 27, y + 38, 20, 2, COL_GRIS);
        caja(x + 36, y + 32, 2, 14, COL_GRIS);
    }
}

static void dibujar_carril(void)
{
    caja(0, CARRIL_Y, PANTALLA_W, CARRIL_H, COL_CARRIL);
    caja(0, CARRIL_Y, PANTALLA_W, 2, COL_LINEA);
    caja(0, CARRIL_Y + CARRIL_H - 2, PANTALLA_W, 2, COL_LINEA);
    for (int16_t x = 9; x < PANTALLA_W; x += 30)
        caja(x, CARRIL_Y + 15, 16, 2, COL_AMARILLO);
}

uint8_t Parqueo_GetLibres(void)
{
    uint8_t libres = 0;
    for (uint8_t i = 0; i < ESPACIOS; ++i)
        if (!ocupado[i]) ++libres;
    return libres;
}

uint8_t Parqueo_GetOcupado(uint8_t espacio)
{
    return (espacio >= 1U && espacio <= ESPACIOS)
           ? ocupado[espacio - 1U] : 0U;
}

static void dibujar_contadores(void)
{
    char libres[] = "0";
    char ocupados[] = "0";
    uint8_t disponibles = Parqueo_GetLibres();
    libres[0] = (char)('0' + disponibles);
    ocupados[0] = (char)('0' + ESPACIOS - disponibles);
    caja(0, 46, 320, 21, COL_PANEL);
    LCD_Print("LIBRES:", 8, 50, 1, COL_GRIS, COL_PANEL);
    LCD_Print(libres, 72, 50, 1, COL_VERDE, COL_PANEL);
    LCD_Print("OCUPADOS:", 108, 50, 1, COL_GRIS, COL_PANEL);
    LCD_Print(ocupados, 188, 50, 1, COL_ROJO, COL_PANEL);
    LCD_Print(en_demo ? "DEMO" : "REAL", 265, 50, 1, COL_CIAN, COL_PANEL);
}

static void dibujar_pie(const char *mensaje, uint8_t numero)
{
    char etiqueta[] = "P1";
    etiqueta[1] = (char)('1' + numero);
    caja(0, 221, 320, 19, COL_CABECERA);
    LCD_Print(en_demo ? "SIMULACION" : "SENSORES", 8, 224,
              1, COL_CIAN, COL_CABECERA);
    LCD_Print((char *)mensaje, 111, 224, 1, COL_BLANCO, COL_CABECERA);
    if (mensaje[0] != '\0')
        LCD_Print(etiqueta, 288, 224, 1, COL_AMARILLO, COL_CABECERA);
}

void Parqueo_SetOcupado(uint8_t espacio, uint8_t estado)
{
    if (espacio < 1U || espacio > ESPACIOS) return;
    uint8_t indice = espacio - 1U;
    estado = estado ? 1U : 0U;
    if (ocupado[indice] == estado) return; /* evita parpadeos */
    ocupado[indice] = estado;
    dibujar_espacio(indice);
    dibujar_contadores();
    if (!en_demo) dibujar_pie("ESTADO ACTUALIZADO", indice);
}

void Parqueo_Init(uint8_t modo_demo)
{
    en_demo = modo_demo ? 1U : 0U;
    demo_indice = 0;
    demo_llenando = 1;
    animacion = PARADO;
    ultimo_paso = HAL_GetTick();
    proximo_movimiento = ultimo_paso + PAUSA_MS;
    for (uint8_t i = 0; i < ESPACIOS; i++) ocupado[i] = 0;

    LCD_Clear(COL_FONDO);
    caja(0, 0, 320, 46, COL_CABECERA);
    caja(0, 43, 320, 3, COL_CIAN);
    LCD_Print("CONTROL DE PARQUEO", 16, 13, 2, COL_BLANCO, COL_CABECERA);
    dibujar_contadores();
    for (uint8_t i = 0; i < ESPACIOS; i++) dibujar_espacio(i);
    dibujar_carril();
    dibujar_pie("", 0);
}

static void siguiente_espacio(void)
{
    if (demo_llenando) {
        if (demo_indice == ESPACIOS - 1U) {
            demo_llenando = 0U;
        } else {
            ++demo_indice;
        }
    } else {
        if (demo_indice == 0U) {
            demo_llenando = 1U;
        } else {
            --demo_indice;
        }
    }
}

static void iniciar_movimiento(uint32_t ahora)
{
    objetivo = demo_indice;
    ultimo_paso = ahora;
    if (demo_llenando) {
        auto_x = -34;
        animacion = ENTRANDO_CARRIL;
        dibujar_pie("ENTRANDO", objetivo);
    } else {
        auto_y = coche_y(objetivo);
        /* Al abandonar su plaza ya queda disponible. */
        Parqueo_SetOcupado(objetivo + 1U, 0U);
        animacion = SALIENDO_ESPACIO;
        dibujar_pie("SALIENDO", objetivo);
    }
}

static void fin_movimiento(uint32_t ahora)
{
    animacion = PARADO;
    dibujar_carril();
    dibujar_pie("", 0);
    siguiente_espacio();
    proximo_movimiento = ahora + PAUSA_MS;
}

void Parqueo_Update(void)
{
    if (!en_demo) return;
    uint32_t ahora = HAL_GetTick();
    if (animacion == PARADO) {
        if ((int32_t)(ahora - proximo_movimiento) >= 0)
            iniciar_movimiento(ahora);
        return;
    }
    if ((uint32_t)(ahora - ultimo_paso) < INTERVALO_MS) return;
    ultimo_paso = ahora;

    int16_t destino_x = casilla_x(objetivo) + (ESPACIO_W - 34) / 2;
    int16_t destino_y = coche_y(objetivo);

    switch (animacion) {
    case ENTRANDO_CARRIL:
        auto_x += 7;
        if (auto_x >= destino_x) auto_x = destino_x;
        dibujar_carril();
        coche_horizontal(auto_x, CARRIL_Y + 9, carro_color[objetivo]);
        if (auto_x == destino_x) {
            animacion = ENTRANDO_ESPACIO;
            auto_y = CARRIL_Y + 3;
        }
        break;

    case ENTRANDO_ESPACIO:
        if (auto_y < destino_y) {
            auto_y += 4;
            if (auto_y > destino_y) auto_y = destino_y;
        } else if (auto_y > destino_y) {
            auto_y -= 4;
            if (auto_y < destino_y) auto_y = destino_y;
        }
        dibujar_espacio(objetivo);
        dibujar_carril();
        coche_vertical(coche_x(objetivo), auto_y, carro_color[objetivo]);
        if (auto_y == destino_y) {
            Parqueo_SetOcupado(objetivo + 1U, 1U);
            fin_movimiento(ahora);
        }
        break;

    case SALIENDO_ESPACIO:
        if (auto_y < CARRIL_Y + 3) {
            auto_y += 4;
            if (auto_y > CARRIL_Y + 3) auto_y = CARRIL_Y + 3;
        } else if (auto_y > CARRIL_Y + 3) {
            auto_y -= 4;
            if (auto_y < CARRIL_Y + 3) auto_y = CARRIL_Y + 3;
        }
        dibujar_espacio(objetivo);
        dibujar_carril();
        coche_vertical(coche_x(objetivo), auto_y, carro_color[objetivo]);
        if (auto_y == CARRIL_Y + 3) {
            animacion = SALIENDO_CARRIL;
            auto_x = destino_x;
        }
        break;

    case SALIENDO_CARRIL:
        auto_x += 7;
        dibujar_carril();
        coche_horizontal(auto_x, CARRIL_Y + 9, carro_color[objetivo]);
        if (auto_x >= PANTALLA_W) fin_movimiento(ahora);
        break;

    default:
        animacion = PARADO;
        break;
    }
}
