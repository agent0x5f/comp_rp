#ifndef GRAFICADOR_H
#define GRAFICADOR_H

#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>

class MyGraphCanvas : public wxPanel {
public:
    MyGraphCanvas(wxWindow* parent, wxPoint pos, wxSize size);
    void SetDatos(const std::vector<std::vector<double>>& puntos,
                   const std::vector<int>& clases,
                   bool mostrar_centros = false) {
        puntos_plot = puntos;
        clases_plot = clases;
        dibujar_centros = mostrar_centros;
    }
    void OnPaint(wxPaintEvent& event);

    void LimpiarGrafico() {
        puntos_plot.clear();
        clases_plot.clear();
        dibujar_centros = false;
        Refresh(); // Obliga al canvas a redibujarse (quedará en blanco)
    }

    // Nuevos métodos para la GUI
    void SetModo3D(bool activar3D);
    bool Is3D() const { return modo3D; }

private:
    bool modo3D = false; // Estado actual del canvas
    std::vector<std::vector<double>> puntos_plot;
    std::vector<int> clases_plot;
    bool dibujar_centros = false;

    // Separamos la lógica para mayor orden
    void Dibujar2D(wxGraphicsContext* gc, int w, int h);
    void Dibujar3D(wxGraphicsContext* gc, int w, int h);
    // Variables para la rotación 3D
    double angleYaw = 0.5;   // Ángulo inicial
    double anglePitch = 0.6; // Ángulo inicial
    wxPoint lastMousePos;
    bool isDragging = false;

    // Funciones para los eventos del ratón
    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseLeftUp(wxMouseEvent& event);
    void OnMouseMotion(wxMouseEvent& event);
};

#endif
