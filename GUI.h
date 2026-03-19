#ifndef GRAFICOS_H
#define GRAFICOS_H

#include <wx/wx.h>
#include "graficador.h"

class MyFrame : public wxFrame {
public:
    MyFrame();
    void OnOpenExplorer(const wxCommandEvent& event);
    void OnCalculaClick(wxCommandEvent& event);
    void OnEscritura(wxCommandEvent &event);
    static void log(std::string msg, wxTextCtrl *out);
    void OnCheckClick(wxCommandEvent& event);
    void OnlimpiaClick(wxCommandEvent& event);

    void OnGuardarClick(wxCommandEvent &event);
    // Declaración de las nuevas funciones para los botones 2D y 3D
    void OnButton2DClick(wxCommandEvent& event);
    void OnButton3DClick(wxCommandEvent& event);

    //selection de algoritmo
    void OnAlgoritmoSelect(wxCommandEvent& event);

private:
    wxButton* cargar_archivo;
    wxStaticText* etiqueta1;
    wxTextCtrl* textbox1;
    wxTextCtrl* textbox2;
    wxButton* calcula;
    wxTextCtrl* consola;
    MyGraphCanvas *canvas;
    wxCheckBox * checkbox1;
    wxButton* limpia;
    wxButton* guardar;
    wxButton* grafica2d;
    wxButton* grafica3d;
    wxChoice * choice;
};

#endif
