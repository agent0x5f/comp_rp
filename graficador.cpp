#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include "graficador.h"
#include "algoritmo.h"
#include <algorithm>

MyGraphCanvas::MyGraphCanvas(wxWindow* parent, wxPoint pos, wxSize size)
    : wxPanel(parent, wxID_ANY, pos, size, wxBORDER_SUNKEN) 
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &MyGraphCanvas::OnPaint, this);
}

// Función de dibujado
void MyGraphCanvas::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();

    wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
    if (!gc) return;

    int w, h;
    GetClientSize(&w, &h);
    int margin = 50;
    int rightMargin = 150;

    double anchoGrafico = w - margin - rightMargin;
    double altoGrafico = h - 2.0 * margin;

    // --- PALETA DE COLORES ---
    std::vector<wxColour> paleta = {
        wxColour(150, 150, 150), // Grupo 0 Gris
        wxColour(215, 50, 50),   // Grupo 1 Rojo
        wxColour(50, 120, 215),  // Grupo 2 Azul
        wxColour(50, 200, 50),   // Grupo 3 Verde
        wxColour(255, 128, 0),   // Grupo 4 Naranja
        wxColour(128, 0, 128),    // Grupo 5 Morado
        wxColour(255, 105, 180), // Grupo 6: Rosa
        wxColour(0, 255, 255),   // Grupo 7: Cyan
        wxColour(255, 255, 0),   // Grupo 8: Amarillo
        wxColour(128, 0, 255),   // Grupo 9: Violeta
        wxColour(0, 255, 127),   // Grupo 10: Verde Neón
        wxColour(255, 255, 255), // Grupo 11: Blanco
        wxColour(0, 0, 0)        // Grupo 12: Negro
        };

    // --- ENCONTRAR MÁXIMOS Y MÍNIMOS REALES ---
    double minXReal = 0.0, maxXReal = 0.0;
    double minYReal = 0.0, maxYReal = 0.0;

    if (!Algoritmo::matrizDatos.empty()) {
        minXReal = maxXReal = Algoritmo::matrizDatos[0][0];
        minYReal = maxYReal = Algoritmo::matrizDatos[0][1];

        for (const auto& fila : Algoritmo::matrizDatos) {
            if (fila.size() >= 2) {
                if (fila[0] < minXReal) minXReal = fila[0];
                if (fila[0] > maxXReal) maxXReal = fila[0];
                if (fila[1] < minYReal) minYReal = fila[1];
                if (fila[1] > maxYReal) maxYReal = fila[1];
            }
        }
    }

    // Forzamos a que el (0,0) siempre esté dentro del área visible
    // para que podamos ver los ejes principales.
    if (minXReal > 0) minXReal = 0;
    if (maxXReal < 0) maxXReal = 0;
    if (minYReal > 0) minYReal = 0;
    if (maxYReal < 0) maxYReal = 0;

    // --- CÁLCULO DE LA CUADRÍCULA Y ESCALA ---
    int divisiones = 10;

    // Agregamos un 10% de padding al rango total
    double rangoXReal = maxXReal - minXReal;
    double rangoYReal = maxYReal - minYReal;
    if (rangoXReal == 0) rangoXReal = 10; // Evitar divisiones por cero
    if (rangoYReal == 0) rangoYReal = 10;

    double limitMinX = minXReal - (rangoXReal * 0.1);
    double limitMaxX = maxXReal + (rangoXReal * 0.1);
    double limitMinY = minYReal - (rangoYReal * 0.1);
    double limitMaxY = maxYReal + (rangoYReal * 0.1);

    // Calculamos el paso exacto en enteros
    int pasoXMat = std::max(1, (int)std::ceil((limitMaxX - limitMinX) / divisiones));
    int pasoYMat = std::max(1, (int)std::ceil((limitMaxY - limitMinY) / divisiones));

    // Ajustamos los límites matemáticos para que sean múltiplos exactos del paso
    int minX = std::floor(limitMinX / pasoXMat) * pasoXMat;
    int maxX = minX + (pasoXMat * divisiones);
    int minY = std::floor(limitMinY / pasoYMat) * pasoYMat;
    int maxY = minY + (pasoYMat * divisiones);

    // Factores de escala
    double escalaX = anchoGrafico / (maxX - minX);
    double escalaY = altoGrafico / (maxY - minY);

    // --- FUNCIONES LAMBDA DE ESCALAMIENTO ---
    // Estas funciones convierten cualquier valor matemático (X,Y) a píxeles de pantalla
    auto toScreenX = [&](double x) { return margin + (x - minX) * escalaX; };
    auto toScreenY = [&](double y) { return (h - margin) - (y - minY) * escalaY; };

    // DIBUJAR CUADRÍCULA Y TEXTOS ---
    wxFont fuente = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    fuente.SetPointSize(12);
    gc->SetFont(fuente, wxColour(80, 80, 80));
    gc->SetPen(wxPen(wxColour(220, 220, 220), 1)); // Gris claro

    for (int i = 0; i <= divisiones; ++i) {
        double tw, th;

        // Líneas Verticales
        int actualXMat = minX + i * pasoXMat;
        double actualXPantalla = toScreenX(actualXMat);
        gc->StrokeLine(actualXPantalla, margin, actualXPantalla, h - margin);
        wxString textoX = wxString::Format("%d", actualXMat);
        gc->GetTextExtent(textoX, &tw, &th);
        gc->DrawText(textoX, actualXPantalla - (tw / 2.0), h - margin + 8);

        // Líneas Horizontales
        int actualYMat = minY + i * pasoYMat;
        double actualYPantalla = toScreenY(actualYMat);
        gc->StrokeLine(margin, actualYPantalla, w - rightMargin, actualYPantalla);
        wxString textoY = wxString::Format("%d", actualYMat);
        gc->GetTextExtent(textoY, &tw, &th);
        gc->DrawText(textoY, margin - tw - 8, actualYPantalla - (th / 2.0));
    }

    // --- DIBUJAR EJES PRINCIPALES (0,0) ---
    // Encontramos dónde cayó exactamente el (0,0) en la pantalla
    double origenXPantalla = toScreenX(0);
    double origenYPantalla = toScreenY(0);

    gc->SetPen(wxPen(wxColour(50, 50, 50), 2)); // Gris oscuro, más grueso

    // Eje Y (Línea vertical en X=0)
    if (origenXPantalla >= margin && origenXPantalla <= w - rightMargin) {
        gc->StrokeLine(origenXPantalla, margin, origenXPantalla, h - margin);
    }
    // Eje X (Línea horizontal en Y=0)
    if (origenYPantalla >= margin && origenYPantalla <= h - margin) {
        gc->StrokeLine(margin, origenYPantalla, w - rightMargin, origenYPantalla);
    }

    // --- DIBUJAR LOS PUNTOS ---
    if (!Algoritmo::matrizDatos.empty()) {
        gc->SetPen(*wxTRANSPARENT_PEN);
        double radioPunto = 5.0;

        for (size_t i = 0; i < Algoritmo::matrizDatos.size(); ++i) {
            if (Algoritmo::matrizDatos[i].size() >= 2) {
                double xMat = Algoritmo::matrizDatos[i][0];
                double yMat = Algoritmo::matrizDatos[i][1];

                int clase = -1;
                if (i < Algoritmo::listaIndices.size()) {
                    clase = Algoritmo::listaIndices[i];
                }

                wxColour colorGrupo = (clase == -1) ? paleta[0] : paleta[(clase + 1) % paleta.size()];
                gc->SetBrush(wxBrush(colorGrupo));

                // Usamos las lambdas para ubicar el punto exacto
                gc->DrawEllipse(toScreenX(xMat) - radioPunto, toScreenY(yMat) - radioPunto,
                                radioPunto * 2, radioPunto * 2);
            }
        }
    }

    // --- DIBUJAR LA LEYENDA ---
    int maxClaseEncontrada = -1;
    for (int c : Algoritmo::listaIndices) {
        if (c > maxClaseEncontrada) maxClaseEncontrada = c;
    }

    int leyendaX = w - rightMargin + 20;
    int leyendaY = margin;
    gc->SetFont(fuente, *wxBLACK);
    gc->DrawText("Grupos:", leyendaX, leyendaY);
    leyendaY += 25;

    // Pintamos Grupo 0 (No asignados / Base)
    gc->SetBrush(wxBrush(paleta[0]));
    gc->DrawEllipse(leyendaX, leyendaY, 12, 12);
    gc->DrawText("Sin asignar", leyendaX + 20, leyendaY - 1);
    leyendaY += 22;

    // Pintamos los demás grupos
    for (int i = 0; i <= maxClaseEncontrada; ++i) {
        if (i == -1) continue; // Ya dibujamos el base arriba

        wxColour colorGrupo = paleta[(i + 1) % paleta.size()];
        gc->SetBrush(wxBrush(colorGrupo));
        gc->DrawEllipse(leyendaX, leyendaY, 12, 12);

        wxString nombreGrupo = wxString::Format("Grupo %d", i + 1); // Mostramos Grupo 1, Grupo 2...
        gc->DrawText(nombreGrupo, leyendaX + 20, leyendaY - 1);

        leyendaY += 22;
    }

    delete gc;
}