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

    // AHORA USA LA VARIABLE GENÉRICA
    if (!puntos_plot.empty()) {
        if (puntos_plot[0].size() > 0) minXReal = maxXReal = puntos_plot[0][0];
        if (puntos_plot[0].size() > 1) minYReal = maxYReal = puntos_plot[0][1];

        for (const auto& fila : puntos_plot) {
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
        double curX = toScreenX(minX + i * pasoXMat);
        gc->StrokeLine(curX, margin, curX, h - margin);
        gc->DrawText(wxString::Format("%d", minX + i * pasoXMat), curX - 5, h - margin + 5);

        double curY = toScreenY(minY + i * pasoYMat);
        gc->StrokeLine(margin, curY, w - rightMargin, curY);
        gc->DrawText(wxString::Format("%d", minY + i * pasoYMat), margin - 25, curY - 7);
    }

    // --- SECCIÓN DE PUNTOS ---
for (size_t i = 0; i < puntos_plot.size(); ++i) {
        if (puntos_plot[i].size() >= 2) {
            int clase = (i < clases_plot.size()) ? clases_plot[i] : -1;

            double posX = toScreenX(puntos_plot[i][0]);
            double posY = toScreenY(puntos_plot[i][1]);
            wxColour colorPunto;
            if (clase == -1) colorPunto = paleta[0];
            else if (clase == -2) colorPunto = wxColour(30, 30, 30); // Ruido (Gris casi negro)
            else colorPunto = paleta[(clase + 1) % paleta.size()];

            gc->SetBrush(wxBrush(colorPunto));
            gc->SetPen(wxPen(colorPunto, 1));
            gc->DrawEllipse(posX - 9, posY - 9, 18, 18);

            // Verificación segura para los centros (solo si es Max-Min)
            if (dibujar_centros && clase != -1 && clase != -2) {
                if (i < maxmin::matrizDistancias.size() && clase < (int)maxmin::matrizDistancias[i].size()) {
                    if (maxmin::matrizDistancias[i][clase] == 0.0f) {
                        gc->SetFont(wxFontInfo(14).Bold(), *wxBLACK);
                        double textWidth, textHeight;
                        gc->GetTextExtent("*", &textWidth, &textHeight);
                        gc->DrawText("*", posX - (textWidth / 2.0), posY - (textHeight / 2.0) + 2);
                        gc->SetPen(wxPen(*wxBLACK, 1));
                        gc->SetBrush(*wxTRANSPARENT_BRUSH);
                        gc->DrawEllipse(posX - 9, posY - 9, 18, 18);
                        gc->SetPen(wxNullPen);
                    }
                }
            }
        }
    }

    int maxClaseEncontrada = -1;
    for (int c : clases_plot) {
        if (c > maxClaseEncontrada) maxClaseEncontrada = c;
    }

    int leyendaX = w - rightMargin + 20;
    int leyendaY = margin;
    wxFont fuente = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    gc->SetFont(fuente, *wxBLACK);
    gc->DrawText("Grupos:", leyendaX, leyendaY);
    leyendaY += 25;

    gc->SetBrush(wxBrush(paleta[0]));
    gc->SetPen(wxNullPen);
    gc->DrawEllipse(leyendaX, leyendaY, 12, 12);
    gc->DrawText("Sin asignar", leyendaX + 20, leyendaY - 1);
    leyendaY += 22;

    // Solo se dibuja si existe ruido (-2) en los datos
    if (std::find(clases_plot.begin(), clases_plot.end(), -2) != clases_plot.end()) {
        gc->SetBrush(wxBrush(wxColour(30, 30, 30)));
        gc->DrawEllipse(leyendaX, leyendaY, 12, 12);
        gc->DrawText("Ruido", leyendaX + 20, leyendaY - 1);
        leyendaY += 22;
    }

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
    if (!puntos_plot.empty()) {
        for (const auto& fila : puntos_plot) {
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

   if (!puntos_plot.empty()) {
        gc->SetPen(wxPen(wxColour(180, 180, 180), 1, wxPENSTYLE_DOT));
        double radioPunto = 5.0;

        for (size_t i = 0; i < puntos_plot.size(); ++i) {
            double xMat = puntos_plot[i].size() > 0 ? puntos_plot[i][0] : 0;
            double yMat = puntos_plot[i].size() > 1 ? puntos_plot[i][1] : 0;
            double zMat = puntos_plot[i].size() > 2 ? puntos_plot[i][2] : 0;

            int clase = (i < clases_plot.size()) ? clases_plot[i] : -1;

            wxColour colorGrupo;
            if (clase == -1) colorGrupo = paleta[0];
            else if (clase == -2) colorGrupo = wxColour(30, 30, 30); // Ruido
            else colorGrupo = paleta[(clase + 1) % paleta.size()];

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
    for (int c : clases_plot) {
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

    // Solo se dibuja si existe ruido (-2) en los datos
    if (std::find(clases_plot.begin(), clases_plot.end(), -2) != clases_plot.end()) {
        gc->SetBrush(wxBrush(wxColour(30, 30, 30)));
        gc->DrawEllipse(leyendaX, leyendaY, 12, 12);
        gc->DrawText("Ruido", leyendaX + 20, leyendaY - 1);
        leyendaY += 22;
    }

    for (int i = 0; i <= maxClaseEncontrada; ++i) {
        gc->SetBrush(wxBrush(paleta[(i + 1) % paleta.size()]));
        gc->DrawEllipse(leyendaX, leyendaY, 12, 12);
        gc->DrawText(wxString::Format("Grupo %d", i + 1), leyendaX + 20, leyendaY - 1);
        leyendaY += 22;
    }
}