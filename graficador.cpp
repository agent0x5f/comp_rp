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
}

void MyGraphCanvas::Dibujar3D(wxGraphicsContext* gc, int w, int h) {
    int margin = 50;
    int rightMargin = 150;
    double centroX = (w - rightMargin) / 2.0;
    double centroY = h / 2.0 + 50;

    std::vector<wxColour> paleta = {
        wxColour(150, 150, 150), wxColour(215, 50, 50), wxColour(50, 120, 215),
        wxColour(50, 200, 50), wxColour(255, 128, 0), wxColour(128, 0, 128),
        wxColour(255, 105, 180), wxColour(0, 255, 255), wxColour(255, 255, 0),
        wxColour(128, 0, 255), wxColour(0, 255, 127)
    };

    double maxVal = 1.0; 
    for (const auto& fila : Algoritmo::matrizDatos) {
        for (double v : fila) if (v > maxVal) maxVal = v;
    }
    
    int divisiones = 5;
    int pasoMat = std::max(1, (int)std::ceil(maxVal / divisiones));
    int maxGrid = pasoMat * divisiones;
    double escala = std::min(centroX - margin, centroY - margin) / (maxGrid * 1.5);

    // NUEVA ROTACIÓN: Yaw (Izquierda 40°) y Pitch (Abajo 30°)
    double angleYaw = 0.70;   
    double anglePitch = 0.52; 

    auto proyectar = [&](double x, double y, double z, double& px, double& py) {
        double x1 = x * cos(angleYaw) - y * sin(angleYaw);
        double y1 = x * sin(angleYaw) + y * cos(angleYaw);
        double yp = y1 * cos(anglePitch) - z * sin(anglePitch);
        px = centroX + (x1 * escala);
        py = centroY + (yp * escala);
    };

    // Cuadrícula
    gc->SetPen(wxPen(wxColour(230, 230, 230), 1));
    double px1, py1, px2, py2;
    for (int i = 0; i <= divisiones; ++i) {
        int v = i * pasoMat;
        proyectar(v, 0, 0, px1, py1); proyectar(v, maxGrid, 0, px2, py2); gc->StrokeLine(px1, py1, px2, py2);
        proyectar(0, v, 0, px1, py1); proyectar(maxGrid, v, 0, px2, py2); gc->StrokeLine(px1, py1, px2, py2);
    }

    // Ejes
    gc->SetPen(wxPen(wxColour(255, 80, 80), 3)); // X
    proyectar(0,0,0,px1,py1); proyectar(maxGrid,0,0,px2,py2); gc->StrokeLine(px1,py1,px2,py2);
    gc->SetPen(wxPen(wxColour(80, 200, 80), 3)); // Y
    proyectar(0,0,0,px1,py1); proyectar(0,maxGrid,0,px2,py2); gc->StrokeLine(px1,py1,px2,py2);
    gc->SetPen(wxPen(wxColour(80, 80, 255), 3)); // Z
    proyectar(0,0,0,px1,py1); proyectar(0,0,maxGrid,px2,py2); gc->StrokeLine(px1,py1,px2,py2);

    // Puntos
    for (size_t i = 0; i < Algoritmo::matrizDatos.size(); ++i) {
        double x = Algoritmo::matrizDatos[i].size() > 0 ? Algoritmo::matrizDatos[i][0] : 0;
        double y = Algoritmo::matrizDatos[i].size() > 1 ? Algoritmo::matrizDatos[i][1] : 0;
        double z = Algoritmo::matrizDatos[i].size() > 2 ? Algoritmo::matrizDatos[i][2] : 0;
        double px, py, pzX, pzY;
        proyectar(x, y, z, px, py);
        proyectar(x, y, 0, pzX, pzY);
        gc->SetPen(wxPen(wxColour(180, 180, 180), 1, wxPENSTYLE_DOT));
        gc->StrokeLine(px, py, pzX, pzY);
        int clase = (i < Algoritmo::listaIndices.size()) ? Algoritmo::listaIndices[i] : -1;
        gc->SetBrush(wxBrush(clase == -1 ? paleta[0] : paleta[(clase + 1) % paleta.size()]));
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->DrawEllipse(px - 4, py - 4, 8, 8);
    }
}
