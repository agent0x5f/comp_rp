#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include "graficador.h"
#include <algorithm>
#include <cmath>
#include "maxmin.h"
#include "chainmap.h"

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
        lastMousePos = event.GetPosition(); 
    }
}

void MyGraphCanvas::OnMouseLeftUp(wxMouseEvent& event) {
    isDragging = false; 
}

void MyGraphCanvas::OnMouseMotion(wxMouseEvent& event) {
    if (isDragging && modo3D) {
        wxPoint currentPos = event.GetPosition();
        
        double deltaX = currentPos.x - lastMousePos.x;
        double deltaY = currentPos.y - lastMousePos.y;
        double sensibilidad = 0.01; 

        angleYaw -= deltaX * sensibilidad;
        anglePitch -= deltaY * sensibilidad;

        if (anglePitch > 1.5) anglePitch = 1.5;
        if (anglePitch < -1.5) anglePitch = -1.5;

        lastMousePos = currentPos;
        Refresh(); 
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

    if (!maxmin::matrizDatos.empty()) {
        if (maxmin::matrizDatos[0].size() > 0) minXReal = maxXReal = maxmin::matrizDatos[0][0];
        if (maxmin::matrizDatos[0].size() > 1) minYReal = maxYReal = maxmin::matrizDatos[0][1];

        for (const auto& fila : maxmin::matrizDatos) {
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

    // Dibujo de la cuadrícula
    for (int i = 0; i <= divisiones; ++i) {
        double curX = toScreenX(minX + i * pasoXMat);
        gc->StrokeLine(curX, margin, curX, h - margin);
        gc->DrawText(wxString::Format("%d", minX + i * pasoXMat), curX - 5, h - margin + 5);

        double curY = toScreenY(minY + i * pasoYMat);
        gc->StrokeLine(margin, curY, w - rightMargin, curY);
        gc->DrawText(wxString::Format("%d", minY + i * pasoYMat), margin - 25, curY - 7);
    }

    // --- SECCIÓN DE DIBUJO DE PUNTOS Y CENTROS ---
    for (size_t i = 0; i < maxmin::matrizDatos.size(); ++i) {
        if (maxmin::matrizDatos[i].size() >= 2) {
            int clase = (i < maxmin::listaIndices.size()) ? maxmin::listaIndices[i] : -1;

            double posX = toScreenX(maxmin::matrizDatos[i][0]);
            double posY = toScreenY(maxmin::matrizDatos[i][1]);

            // 1. Dibujar el punto normal (elipse)
            gc->SetBrush(wxBrush(clase == -1 ? paleta[0] : paleta[(clase + 1) % paleta.size()]));
            gc->SetPen(wxPen(clase == -1 ? paleta[0] : paleta[(clase + 1) % paleta.size()], 1));
            gc->DrawEllipse(posX - 9, posY - 9, 18, 18);

            // 2. Comprobar si este punto es el centro de su clase
            bool esCentro = false;
            // Validamos que tenga una clase asignada y que la matrizDistancias tenga registrada esa columna
            if (clase != -1 && clase < (int)maxmin::matrizDistancias[i].size()) {
                // Si la distancia a su propio centro es exactamente 0, significa que ESTE es el centro
                if (maxmin::matrizDistancias[i][clase] == 0.0f) {
                    esCentro = true;
                }
            }

            // 3. Si es un centro, dibujamos el asterisco y resaltamos el borde
            if (esCentro) {
                // Dibujar el asterisco
                gc->SetFont(wxFontInfo(14).Bold(), *wxBLACK);
                double textWidth, textHeight;
                gc->GetTextExtent("*", &textWidth, &textHeight);
                gc->DrawText("*", posX - (textWidth / 2.0), posY - (textHeight / 2.0) + 2);

                // Dibujar un borde negro para que el centro destaque más
                gc->SetPen(wxPen(*wxBLACK, 1));
                gc->SetBrush(*wxTRANSPARENT_BRUSH);
                gc->DrawEllipse(posX - 9, posY - 9, 18, 18);

                // Restauramos el pen nulo por seguridad
                gc->SetPen(wxNullPen);
            }
        }
    }
    // --- FIN SECCIÓN DE PUNTOS ---

    // Restaurar parámetros visuales para la leyenda
    int maxClaseEncontrada = -1;
    for (int c : maxmin::listaIndices) {
        if (c > maxClaseEncontrada) maxClaseEncontrada = c;
    }

    int leyendaX = w - rightMargin + 20;
    int leyendaY = margin;
    wxFont fuente = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    gc->SetFont(fuente, *wxBLACK);
    gc->DrawText("Grupos:", leyendaX, leyendaY);
    leyendaY += 25;

    // Leyenda: Sin asignar
    gc->SetBrush(wxBrush(paleta[0]));
    gc->SetPen(wxNullPen);
    gc->DrawEllipse(leyendaX, leyendaY, 12, 12);
    gc->DrawText("Sin asignar", leyendaX + 20, leyendaY - 1);
    leyendaY += 22;

    // Leyenda: Grupos creados
    for (int i = 0; i <= maxClaseEncontrada; ++i) {
        wxColour colorGrupo = paleta[(i + 1) % paleta.size()];
        gc->SetBrush(wxBrush(colorGrupo));
        gc->DrawEllipse(leyendaX, leyendaY, 12, 12);
        gc->DrawText(wxString::Format("Grupo %d", i + 1), leyendaX + 20, leyendaY - 1);
        leyendaY += 22;
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
    if (!maxmin::matrizDatos.empty()) {
        for (const auto& fila : maxmin::matrizDatos) {
            for (double val : fila) if (val > maxVal) maxVal = val;
        }
    }
    
    int divisiones = 5; 
    int pasoMat = std::max(1, (int)std::ceil(maxVal / divisiones));
    int maxGrid = pasoMat * divisiones; 
    double escala = std::min(centroX - margin, centroY - margin) / (maxGrid * 1.2);

    auto proyectar = [&](double xOriginal, double yOriginal, double z, double& px, double& py) {
        double x = yOriginal;
        double y = xOriginal;
        double x1 = x * cos(angleYaw) - y * sin(angleYaw);
        double y1 = x * sin(angleYaw) + y * cos(angleYaw);
        double yp = y1 * cos(anglePitch) - z * sin(anglePitch);
        px = centroX + (x1 * escala);
        py = centroY + (yp * escala);
    };

    wxFont fuenteTextos = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    fuenteTextos.SetPointSize(9);
    gc->SetFont(fuenteTextos, wxColour(100, 100, 100));

    gc->SetPen(wxPen(wxColour(230, 230, 230), 1)); 
    double px1, py1, px2, py2;

    for (int i = 0; i <= divisiones; ++i) {
        int val = i * pasoMat;
        proyectar(val, 0, 0, px1, py1);
        proyectar(val, maxGrid, 0, px2, py2);
        gc->StrokeLine(px1, py1, px2, py2);
        if (i > 0) gc->DrawText(wxString::Format("%d", val), px1, py1 + 2);

        proyectar(0, val, 0, px1, py1);
        proyectar(maxGrid, val, 0, px2, py2);
        gc->StrokeLine(px1, py1, px2, py2);
        if (i > 0) gc->DrawText(wxString::Format("%d", val), px1 - 15, py1 + 2);

        if (i > 0) {
            proyectar(0, 0, val, px1, py1);
            proyectar(maxGrid, 0, val, px2, py2);
            gc->StrokeLine(px1, py1, px2, py2);
            proyectar(0, maxGrid, val, px2, py2);
            gc->StrokeLine(px1, py1, px2, py2);
            gc->DrawText(wxString::Format("%d", val), px1 - 20, py1 - 8);

            proyectar(val, 0, 0, px1, py1);
            proyectar(val, 0, maxGrid, px2, py2);
            gc->StrokeLine(px1, py1, px2, py2);

            proyectar(0, val, 0, px1, py1);
            proyectar(0, val, maxGrid, px2, py2);
            gc->StrokeLine(px1, py1, px2, py2);
        }
    }

    double origenX, origenY, finX, finY;
    proyectar(0, 0, 0, origenX, origenY);

    gc->SetPen(wxPen(wxColour(255, 80, 80), 3)); 
    proyectar(maxGrid + (pasoMat*0.5), 0, 0, finX, finY);
    gc->StrokeLine(origenX, origenY, finX, finY);
    gc->DrawText("X", finX + 5, finY);

    gc->SetPen(wxPen(wxColour(80, 200, 80), 3)); 
    proyectar(0, maxGrid + (pasoMat*0.5), 0, finX, finY);
    gc->StrokeLine(origenX, origenY, finX, finY);
    gc->DrawText("Y", finX, finY + 5);

    gc->SetPen(wxPen(wxColour(80, 80, 255), 3)); 
    proyectar(0, 0, maxGrid + (pasoMat*0.5), finX, finY);
    gc->StrokeLine(origenX, origenY, finX, finY);
    gc->DrawText("Z", finX - 15, finY - 15);

    if (!maxmin::matrizDatos.empty()) {
        gc->SetPen(wxPen(wxColour(180, 180, 180), 1, wxPENSTYLE_DOT));
        double radioPunto = 5.0;

        for (size_t i = 0; i < maxmin::matrizDatos.size(); ++i) {
            double xMat = maxmin::matrizDatos[i].size() > 0 ? maxmin::matrizDatos[i][0] : 0;
            double yMat = maxmin::matrizDatos[i].size() > 1 ? maxmin::matrizDatos[i][1] : 0;
            double zMat = maxmin::matrizDatos[i].size() > 2 ? maxmin::matrizDatos[i][2] : 0;

            int clase = (i < maxmin::listaIndices.size()) ? maxmin::listaIndices[i] : -1;
            wxColour colorGrupo = (clase == -1) ? paleta[0] : paleta[(clase + 1) % paleta.size()];

            double px, py, pisoX, pisoY;
            proyectar(xMat, yMat, zMat, px, py);
            proyectar(xMat, yMat, 0, pisoX, pisoY);

            gc->StrokeLine(pisoX, pisoY, px, py);
            gc->SetPen(*wxTRANSPARENT_PEN);
            gc->SetBrush(wxBrush(colorGrupo));
            gc->DrawEllipse(px - radioPunto, py - radioPunto, radioPunto * 2, radioPunto * 2);
        }
    }

    int maxClaseEncontrada = -1;
    for (int c : maxmin::listaIndices) {
        if (c > maxClaseEncontrada) maxClaseEncontrada = c;
    }

    wxFont fuenteLeyenda = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    fuenteLeyenda.SetPointSize(10);
    int leyendaX = w - rightMargin + 20;
    int leyendaY = margin;
    gc->SetFont(fuenteLeyenda, *wxBLACK);
    gc->DrawText("Grupos:", leyendaX, leyendaY);
    leyendaY += 25;

    gc->SetBrush(wxBrush(paleta[0]));
    gc->DrawEllipse(leyendaX, leyendaY, 12, 12);
    gc->DrawText("Sin asignar", leyendaX + 20, leyendaY - 1);
    leyendaY += 22;

    for (int i = 0; i <= maxClaseEncontrada; ++i) {
        gc->SetBrush(wxBrush(paleta[(i + 1) % paleta.size()]));
        gc->DrawEllipse(leyendaX, leyendaY, 12, 12);
        gc->DrawText(wxString::Format("Grupo %d", i + 1), leyendaX + 20, leyendaY - 1);
        leyendaY += 22;
    }
}
