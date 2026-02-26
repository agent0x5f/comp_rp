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
    Bind(wxEVT_LEFT_DOWN, &MyGraphCanvas::OnMouseLeftDown, this);
    Bind(wxEVT_LEFT_UP, &MyGraphCanvas::OnMouseLeftUp, this);
    Bind(wxEVT_MOTION, &MyGraphCanvas::OnMouseMotion, this);
}

void MyGraphCanvas::SetModo3D(bool activar3D) {
    modo3D = activar3D;
    Refresh();
}

void MyGraphCanvas::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();

    wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
    if (!gc) return;

    int w, h;
    GetClientSize(&w, &h);

    if (modo3D) {
        Dibujar3D(gc, w, h);
    } else {
        Dibujar2D(gc, w, h);
    }

    delete gc;
}

void MyGraphCanvas::OnMouseLeftDown(wxMouseEvent& event) {
    if (modo3D) {
        isDragging = true;
        lastMousePos = event.GetPosition(); // Guardamos dónde empezó el clic
    }
}

void MyGraphCanvas::OnMouseLeftUp(wxMouseEvent& event) {
    isDragging = false; // Dejamos de rotar al soltar el clic
}

void MyGraphCanvas::OnMouseMotion(wxMouseEvent& event) {
    if (isDragging && modo3D) {
        wxPoint currentPos = event.GetPosition();
        
        // Calculamos cuánto se movió el ratón
        double deltaX = currentPos.x - lastMousePos.x;
        double deltaY = currentPos.y - lastMousePos.y;

        // Sensibilidad del giro (ajusta este valor si gira muy rápido o lento)
        double sensibilidad = 0.01; 

        // Modificamos los ángulos (deltaX mueve Yaw, deltaY mueve Pitch)
        angleYaw += deltaX * sensibilidad;
        anglePitch += deltaY * sensibilidad;

        // Opcional: Limitar el Pitch para que la gráfica no se ponga "de cabeza"
        if (anglePitch > 1.5) anglePitch = 1.5;
        if (anglePitch < -1.5) anglePitch = -1.5;

        // Actualizamos la posición anterior y mandamos a redibujar
        lastMousePos = currentPos;
        Refresh(); // Esto llama a OnPaint automáticamente
    }
}

void MyGraphCanvas::Dibujar2D(wxGraphicsContext* gc, int w, int h) {
    int margin = 50;
    int rightMargin = 150;

    double anchoGrafico = w - margin - rightMargin;
    double altoGrafico = h - 2.0 * margin;

    std::vector<wxColour> paleta = {
        wxColour(150, 150, 150), wxColour(215, 50, 50), wxColour(50, 120, 215),
        wxColour(50, 200, 50), wxColour(255, 128, 0), wxColour(128, 0, 128),
        wxColour(255, 105, 180), wxColour(0, 255, 255), wxColour(255, 255, 0),
        wxColour(128, 0, 255), wxColour(0, 255, 127)
    };

    double minXReal = 0.0, maxXReal = 0.0;
    double minYReal = 0.0, maxYReal = 0.0;

    if (!Algoritmo::matrizDatos.empty()) {
        if (Algoritmo::matrizDatos[0].size() > 0) minXReal = maxXReal = Algoritmo::matrizDatos[0][0];
        if (Algoritmo::matrizDatos[0].size() > 1) minYReal = maxYReal = Algoritmo::matrizDatos[0][1];

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

    int divisiones = 10;
    double rangoXReal = std::max(10.0, maxXReal - minXReal);
    double rangoYReal = std::max(10.0, maxYReal - minYReal);

    double limitMinX = minXReal - (rangoXReal * 0.1);
    double limitMaxX = maxXReal + (rangoXReal * 0.1);
    double limitMinY = minYReal - (rangoYReal * 0.1);
    double limitMaxY = maxYReal + (rangoYReal * 0.1);

    int pasoXMat = std::max(1, (int)std::ceil((limitMaxX - limitMinX) / divisiones));
    int pasoYMat = std::max(1, (int)std::ceil((limitMaxY - limitMinY) / divisiones));

    int minX = std::floor(limitMinX / pasoXMat) * pasoXMat;
    int minY = std::floor(limitMinY / pasoYMat) * pasoYMat;

    double escalaX = anchoGrafico / (pasoXMat * divisiones);
    double escalaY = altoGrafico / (pasoYMat * divisiones);

    auto toScreenX = [&](double x) { return margin + (x - minX) * escalaX; };
    auto toScreenY = [&](double y) { return (h - margin) - (y - minY) * escalaY; };

    gc->SetPen(wxPen(wxColour(220, 220, 220), 1));
    gc->SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT), wxColour(80, 80, 80));

    for (int i = 0; i <= divisiones; ++i) {
        // Cuadrícula y etiquetas
        double curX = toScreenX(minX + i * pasoXMat);
        gc->StrokeLine(curX, margin, curX, h - margin);
        gc->DrawText(wxString::Format("%d", minX + i * pasoXMat), curX - 5, h - margin + 5);

        double curY = toScreenY(minY + i * pasoYMat);
        gc->StrokeLine(margin, curY, w - rightMargin, curY);
        gc->DrawText(wxString::Format("%d", minY + i * pasoYMat), margin - 25, curY - 7);
    }

    // Dibujar puntos
    for (size_t i = 0; i < Algoritmo::matrizDatos.size(); ++i) {
        if (Algoritmo::matrizDatos[i].size() >= 2) {
            int clase = (i < Algoritmo::listaIndices.size()) ? Algoritmo::listaIndices[i] : -1;
            gc->SetBrush(wxBrush(clase == -1 ? paleta[0] : paleta[(clase + 1) % paleta.size()]));
            gc->DrawEllipse(toScreenX(Algoritmo::matrizDatos[i][0]) - 4, toScreenY(Algoritmo::matrizDatos[i][1]) - 4, 8, 8);
        }
    }
     // leyendas
    int maxClaseEncontrada = -1;
    for (int c : Algoritmo::listaIndices) {
        if (c > maxClaseEncontrada) maxClaseEncontrada = c;
    }

    int leyendaX = w - rightMargin + 20;
    int leyendaY = margin;
    gc->SetFont(fuente, *wxBLACK);
    gc->DrawText("Grupos Encontrados:", leyendaX, leyendaY);
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
}
void MyGraphCanvas::Dibujar3D(wxGraphicsContext* gc, int w, int h) {
    int margin = 50;
    int rightMargin = 150;
    
    // Centro del área de dibujo
    double centroX = (w - rightMargin) / 2.0;
    double centroY = h / 2.0 + 50;

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
    
    // 2. Cálculo de la Cuadrícula y Escala
    int divisiones = 5; // Menos divisiones en 3D para no saturar la vista
    int pasoMat = std::max(1, (int)std::ceil(maxVal / divisiones));
    int maxGrid = pasoMat * divisiones; // El límite visual de nuestras paredes/piso

    // Ajustamos la escala para que encaje la cuadrícula completa
    double escala = std::min(centroX - margin, centroY - margin) / (maxGrid * 1.2);

    // 3. Matemáticas de Proyección
   // double angleYaw = 0.5;   // Rota el plano XY
  //  double anglePitch = 0.6; // Inclina la cámara para ver la altura Z

    auto proyectar = [&](double x, double y, double z, double& px, double& py) {
        double x1 = x * cos(angleYaw) - y * sin(angleYaw);
        double y1 = x * sin(angleYaw) + y * cos(angleYaw);
        double yp = y1 * cos(anglePitch) - z * sin(anglePitch);
        px = centroX + (x1 * escala);
        py = centroY + (yp * escala);
    };

    wxFont fuenteTextos = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    fuenteTextos.SetPointSize(9);
    gc->SetFont(fuenteTextos, wxColour(100, 100, 100));

    // --- DIBUJAR CUADRÍCULA 3D (Piso y Paredes) ---
    gc->SetPen(wxPen(wxColour(230, 230, 230), 1)); // Gris muy claro
    double px1, py1, px2, py2;

    for (int i = 0; i <= divisiones; ++i) {
        int val = i * pasoMat;

        // --- PISO (Plano XY) ---
        // Líneas paralelas a Y
        proyectar(val, 0, 0, px1, py1);
        proyectar(val, maxGrid, 0, px2, py2);
        gc->StrokeLine(px1, py1, px2, py2);
        if (i > 0) gc->DrawText(wxString::Format("%d", val), px1, py1 + 2); // Textos eje X

        // Líneas paralelas a X
        proyectar(0, val, 0, px1, py1);
        proyectar(maxGrid, val, 0, px2, py2);
        gc->StrokeLine(px1, py1, px2, py2);
        if (i > 0) gc->DrawText(wxString::Format("%d", val), px1 - 15, py1 + 2); // Textos eje Y

        // --- PAREDES (Planos XZ y YZ) ---
        if (i > 0) { // No redibujar los ejes 0,0,0
            // Líneas horizontales de altura (Z)
            proyectar(0, 0, val, px1, py1);
            proyectar(maxGrid, 0, val, px2, py2); // En pared X
            gc->StrokeLine(px1, py1, px2, py2);
            proyectar(0, maxGrid, val, px2, py2); // En pared Y
            gc->StrokeLine(px1, py1, px2, py2);
            gc->DrawText(wxString::Format("%d", val), px1 - 20, py1 - 8); // Textos eje Z

            // Líneas verticales (Altura en X)
            proyectar(val, 0, 0, px1, py1);
            proyectar(val, 0, maxGrid, px2, py2);
            gc->StrokeLine(px1, py1, px2, py2);

            // Líneas verticales (Altura en Y)
            proyectar(0, val, 0, px1, py1);
            proyectar(0, val, maxGrid, px2, py2);
            gc->StrokeLine(px1, py1, px2, py2);
        }
    }

    // --- 4. Dibujar los 3 Ejes de Referencia Principales ---
    double origenX, origenY, finX, finY;
    proyectar(0, 0, 0, origenX, origenY);

    gc->SetPen(wxPen(wxColour(255, 80, 80), 3)); // Eje X (Rojo)
    proyectar(maxGrid + (pasoMat*0.5), 0, 0, finX, finY);
    gc->StrokeLine(origenX, origenY, finX, finY);
    gc->DrawText("X", finX + 5, finY);

    gc->SetPen(wxPen(wxColour(80, 200, 80), 3)); // Eje Y (Verde)
    proyectar(0, maxGrid + (pasoMat*0.5), 0, finX, finY);
    gc->StrokeLine(origenX, origenY, finX, finY);
    gc->DrawText("Y", finX, finY + 5);

    gc->SetPen(wxPen(wxColour(80, 80, 255), 3)); // Eje Z (Azul)
    proyectar(0, 0, maxGrid + (pasoMat*0.5), finX, finY);
    gc->StrokeLine(origenX, origenY, finX, finY);
    gc->DrawText("Z", finX - 15, finY - 15);

    // --- 5. Dibujar los puntos del dataset ---
    if (!Algoritmo::matrizDatos.empty()) {
        gc->SetPen(wxPen(wxColour(180, 180, 180), 1, wxPENSTYLE_DOT)); // Lápiz para la línea al piso
        double radioPunto = 5.0;

        for (size_t i = 0; i < Algoritmo::matrizDatos.size(); ++i) {
            double xMat = Algoritmo::matrizDatos[i].size() > 0 ? Algoritmo::matrizDatos[i][0] : 0;
            double yMat = Algoritmo::matrizDatos[i].size() > 1 ? Algoritmo::matrizDatos[i][1] : 0;
            double zMat = Algoritmo::matrizDatos[i].size() > 2 ? Algoritmo::matrizDatos[i][2] : 0;

            int clase = (i < Algoritmo::listaIndices.size()) ? Algoritmo::listaIndices[i] : -1;
            wxColour colorGrupo = (clase == -1) ? paleta[0] : paleta[(clase + 1) % paleta.size()];

            double px, py, pisoX, pisoY;
            proyectar(xMat, yMat, zMat, px, py);
            proyectar(xMat, yMat, 0, pisoX, pisoY); // Sombra en Z=0

            // Dibujar línea de anclaje
            gc->StrokeLine(pisoX, pisoY, px, py);

            // Dibujar el nodo
            gc->SetPen(*wxTRANSPARENT_PEN);
            gc->SetBrush(wxBrush(colorGrupo));
            gc->DrawEllipse(px - radioPunto, py - radioPunto, radioPunto * 2, radioPunto * 2);
        }
    }

    // --- 6. Dibujar la Leyenda ---
    int maxClaseEncontrada = -1;
    for (int c : Algoritmo::listaIndices) {
        if (c > maxClaseEncontrada) maxClaseEncontrada = c;
    }

    wxFont fuenteLeyenda = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    fuenteLeyenda.SetPointSize(10);
    int leyendaX = w - rightMargin + 20;
    int leyendaY = margin;
    gc->SetFont(fuenteLeyenda, *wxBLACK);
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
