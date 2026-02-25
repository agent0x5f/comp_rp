#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include "graficador.h"
#include "algoritmo.h"
#include <algorithm>
#include <cmath>

MyGraphCanvas::MyGraphCanvas(wxWindow* parent, wxPoint pos, wxSize size)
    : wxPanel(parent, wxID_ANY, pos, size, wxBORDER_SUNKEN) 
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &MyGraphCanvas::OnPaint, this);
}

// Método que llamarán tus nuevos botones
void MyGraphCanvas::SetModo3D(bool activar3D) {
    modo3D = activar3D;
    Refresh(); // Fuerza a wxWidgets a repintar el canvas inmediatamente
}

void MyGraphCanvas::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();

    wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
    if (!gc) return;

    int w, h;
    GetClientSize(&w, &h);

    // Switch de renderizado
    if (modo3D) {
        Dibujar3D(gc, w, h);
    } else {
        Dibujar2D(gc, w, h);
    }

    delete gc;
}

void MyGraphCanvas::Dibujar2D(wxGraphicsContext* gc, int w, int h) {
    int margin = 50;
    int rightMargin = 150;

    double anchoGrafico = w - margin - rightMargin;
    double altoGrafico = h - 2.0 * margin;

    // --- PALETA DE COLORES ---
    std::vector<wxColour> paleta = {
        wxColour(150, 150, 150), wxColour(215, 50, 50), wxColour(50, 120, 215),
        wxColour(50, 200, 50), wxColour(255, 128, 0), wxColour(128, 0, 128),
        wxColour(255, 105, 180), wxColour(0, 255, 255), wxColour(255, 255, 0),
        wxColour(128, 0, 255), wxColour(0, 255, 127)
    };

    // --- ENCONTRAR MÁXIMOS Y MÍNIMOS REALES ---
    double minXReal = 0.0, maxXReal = 0.0;
    double minYReal = 0.0, maxYReal = 0.0;

    if (!Algoritmo::matrizDatos.empty()) {
        // Aseguramos que haya al menos 1 dimensión para X
        if (Algoritmo::matrizDatos[0].size() > 0) {
            minXReal = maxXReal = Algoritmo::matrizDatos[0][0];
        }
        // Aseguramos que haya al menos 2 dimensiones para Y
        if (Algoritmo::matrizDatos[0].size() > 1) {
            minYReal = maxYReal = Algoritmo::matrizDatos[0][1];
        }

        for (const auto& fila : Algoritmo::matrizDatos) {
            if (fila.size() > 0) {
                if (fila[0] < minXReal) minXReal = fila[0];
                if (fila[0] > maxXReal) maxXReal = fila[0];
            }
            if (fila.size() > 1) {
                if (fila[1] < minYReal) minYReal = fila[1];
                if (fila[1] > maxYReal) maxYReal = fila[1];
            }
        }
    }

    if (minXReal > 0) minXReal = 0;
    if (maxXReal < 0) maxXReal = 0;
    if (minYReal > 0) minYReal = 0;
    if (maxYReal < 0) maxYReal = 0;

    int divisiones = 10;
    double rangoXReal = maxXReal - minXReal;
    double rangoYReal = maxYReal - minYReal;
    if (rangoXReal == 0) rangoXReal = 10; 
    if (rangoYReal == 0) rangoYReal = 10;

    double limitMinX = minXReal - (rangoXReal * 0.1);
    double limitMaxX = maxXReal + (rangoXReal * 0.1);
    double limitMinY = minYReal - (rangoYReal * 0.1);
    double limitMaxY = maxYReal + (rangoYReal * 0.1);

    int pasoXMat = std::max(1, (int)std::ceil((limitMaxX - limitMinX) / divisiones));
    int pasoYMat = std::max(1, (int)std::ceil((limitMaxY - limitMinY) / divisiones));

    int minX = std::floor(limitMinX / pasoXMat) * pasoXMat;
    int maxX = minX + (pasoXMat * divisiones);
    int minY = std::floor(limitMinY / pasoYMat) * pasoYMat;
    int maxY = minY + (pasoYMat * divisiones);

    double escalaX = anchoGrafico / (maxX - minX);
    double escalaY = altoGrafico / (maxY - minY);

    auto toScreenX = [&](double x) { return margin + (x - minX) * escalaX; };
    auto toScreenY = [&](double y) { return (h - margin) - (y - minY) * escalaY; };

    wxFont fuente = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    fuente.SetPointSize(12);
    gc->SetFont(fuente, wxColour(80, 80, 8);
}

void MyGraphCanvas::Dibujar3D(wxGraphicsContext* gc, int w, int h) {
    int margin = 50;
    int rightMargin = 150;
    
    // Centro del área de dibujo
    double centroX = (w - rightMargin) / 2.0;
    double centroY = h / 2.0 + 50; // Desplazado un poco abajo para centrar visualmente

    std::vector<wxColour> paleta = {
        wxColour(150, 150, 150), wxColour(215, 50, 50), wxColour(50, 120, 215),
        wxColour(50, 200, 50), wxColour(255, 128, 0), wxColour(128, 0, 128),
        wxColour(255, 105, 180), wxColour(0, 255, 255), wxColour(255, 255, 0),
        wxColour(128, 0, 255), wxColour(0, 255, 127)
    };

    // 1. Encontrar máximos en X, Y, Z para la escala
    double maxVal = 1.0; 
    if (!Algoritmo::matrizDatos.empty()) {
        for (const auto& fila : Algoritmo::matrizDatos) {
            if (fila.size() > 0 && fila[0] > maxVal) maxVal = fila[0];
            if (fila.size() > 1 && fila[1] > maxVal) maxVal = fila[1];
            if (fila.size() > 2 && fila[2] > maxVal) maxVal = fila[2];
        }
    }
    
    // Escala general para que todo quepa en la pantalla
    double escala = std::min(centroX - margin, centroY - margin) / (maxVal * 1.2);

    // 2. Matemáticas de Proyección Ortogonal (Isométrica)
    // Ángulos de rotación de la cámara
    double angleX = 0.5;  // Inclinación hacia abajo (Pitch)
    double angleY = -0.6; // Rotación lateral (Yaw)

    // Función Lambda que convierte coordenadas 3D a píxeles 2D de pantalla
    auto proyectar = [&](double x, double y, double z, double& px, double& py) {
        // Rotación en eje Y
        double x1 = x * cos(angleY) - z * sin(angleY);
        double z1 = x * sin(angleY) + z * cos(angleY);
        // Rotación en eje X
        double y1 = y * cos(angleX) - z1 * sin(angleX);
        
        // Asignación final escalada y centrada (Y crece hacia abajo en la pantalla, por eso la resta)
        px = centroX + (x1 * escala);
        py = centroY - (y1 * escala);
    };

    wxFont fuente = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    fuente.SetPointSize(10);
    gc->SetFont(fuente, wxColour(80, 80, 80));

    // 3. Dibujar los 3 Ejes de Referencia (X=Rojo, Y=Verde, Z=Azul)
    double origenX, origenY, finX, finY;
    proyectar(0, 0, 0, origenX, origenY);

    gc->SetPen(wxPen(wxColour(255, 100, 100), 2)); // Eje X
    proyectar(maxVal + 1, 0, 0, finX, finY);
    gc->StrokeLine(origenX, origenY, finX, finY);
    gc->DrawText("X", finX + 5, finY);

    gc->SetPen(wxPen(wxColour(100, 200, 100), 2)); // Eje Y (Vertical)
    proyectar(0, maxVal + 1, 0, finX, finY);
    gc->StrokeLine(origenX, origenY, finX, finY);
    gc->DrawText("Y", finX, finY - 15);

    gc->SetPen(wxPen(wxColour(100, 100, 255), 2)); // Eje Z (Profundidad)
    proyectar(0, 0, maxVal + 1, finX, finY);
    gc->StrokeLine(origenX, origenY, finX, finY);
    gc->DrawText("Z", finX - 15, finY);

    // 4. Dibujar los puntos del dataset
    if (!Algoritmo::matrizDatos.empty()) {
        gc->SetPen(wxPen(wxColour(200, 200, 200), 1, wxPENSTYLE_DOT)); // Lápiz para la línea al piso
        double radioPunto = 5.0;

        for (size_t i = 0; i < Algoritmo::matrizDatos.size(); ++i) {
            // Extraer hasta 3 dimensiones (si tiene menos, rellena con 0)
            double xMat = Algoritmo::matrizDatos[i].size() > 0 ? Algoritmo::matrizDatos[i][0] : 0;
            double yMat = Algoritmo::matrizDatos[i].size() > 1 ? Algoritmo::matrizDatos[i][1] : 0;
            double zMat = Algoritmo::matrizDatos[i].size() > 2 ? Algoritmo::matrizDatos[i][2] : 0;

            int clase = (i < Algoritmo::listaIndices.size()) ? Algoritmo::listaIndices[i] : -1;
            wxColour colorGrupo = (clase == -1) ? paleta[0] : paleta[(clase + 1) % paleta.size()];

            double px, py, pisoX, pisoY;
            proyectar(xMat, yMat, zMat, px, py);
            proyectar(xMat, 0, zMat, pisoX, pisoY); // Proyección de la sombra en el plano XZ

            // Dibujar línea de profundidad (hacia el plano base) para efecto 3D
            gc->StrokeLine(pisoX, pisoY, px, py);

            // Dibujar el nodo
            gc->SetPen(*wxTRANSPARENT_PEN);
            gc->SetBrush(wxBrush(colorGrupo));
            gc->DrawEllipse(px - radioPunto, py - radioPunto, radioPunto * 2, radioPunto * 2);
        }
    }

    // 5. Dibujar la Leyenda (Misma lógica que en 2D)
    int maxClaseEncontrada = -1;
    for (int c : Algoritmo::listaIndices) {
        if (c > maxClaseEncontrada) maxClaseEncontrada = c;
    }

    int leyendaX = w - rightMargin + 20;
    int leyendaY = margin;
    gc->SetFont(fuente, *wxBLACK);
    gc->DrawText("Grupos (Vista 3D):", leyendaX, leyendaY);
    leyendaY += 25;

    gc->SetBrush(wxBrush(paleta[0]));
    gc->DrawEllipse(leyendaX, leyendaY, 12, 12);
    gc->DrawText("Sin asignar", leyendaX + 20, leyendaY - 1);
    leyendaY += 22;

    for (int i = 0; i <= maxClaseEncontrada; ++i) {
        if (i == -1) continue;
        gc->SetBrush(wxBrush(paleta[(i + 1) % paleta.size()]));
        gc->DrawEllipse(leyendaX, leyendaY, 12, 12);
        gc->DrawText(wxString::Format("Grupo %d", i + 1), leyendaX + 20, leyendaY - 1);
        leyendaY += 22;
    }
}
