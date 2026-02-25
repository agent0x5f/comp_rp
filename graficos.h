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
    void OnButton2DClick(wxCommandEvent& event);
    void OnButton3DClick(wxCommandEvent& event);
private:
    wxButton* cargar_archivo; //explorador del sistema
    wxStaticText* etiqueta1; //semilla
    wxTextCtrl* textbox1; //semilla
    wxTextCtrl* textbox2; //archivo entrada
    wxButton* calcula; //run
    wxTextCtrl* consola; //terminal
    MyGraphCanvas *canvas;
    wxButton* exporta; //imprime el csv con las clases resultantes
    wxCheckBox * checkbox1;
    wxButton* limpia;//limpia la consola
    wxButton* grafica2D;
    wxButton* grafica3D;
};

#endif
