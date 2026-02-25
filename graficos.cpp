#include "graficos.h"
#include "algoritmo.h"
#include "graficador.h"
#include <wx/wx.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "io.h"
using namespace std;

MyFrame::MyFrame() : wxFrame(nullptr, wxID_ANY, "Programa", wxPoint(50, 50), wxSize(1280, 720)) {
    auto* panel = new wxPanel(this, wxID_ANY); // El panel puede seguir siendo local si no lo vas a modificar después

    cargar_archivo = new wxButton(panel, wxID_ANY, "Cargar Archivo", wxPoint(10, 10), wxSize(150, 30));
    cargar_archivo->Bind(wxEVT_BUTTON, &MyFrame::OnOpenExplorer, this);
    textbox2 = new wxTextCtrl(panel, wxID_ANY, "", wxPoint(10,50), wxSize(150,600), wxTE_READONLY | wxTE_MULTILINE | wxBORDER_SIMPLE);
    calcula = new wxButton(panel,wxID_ANY, "Calcula", wxPoint(220, 10));
    calcula->Disable();
    calcula->Bind(wxEVT_BUTTON, &MyFrame::OnCalculaClick, this);
   // checkbox1 = new wxCheckBox(panel,wxID_ANY, "Explicado", wxPoint(400,15));
    etiqueta1 = new wxStaticText(panel, wxID_ANY, "Semilla: ", wxPoint(320,15));
    textbox1 = new wxTextCtrl(panel, wxID_ANY, "1", wxPoint(370,10),wxSize(60,20),wxBORDER_SIMPLE);
    textbox1->Bind(wxEVT_TEXT, &MyFrame::OnEscritura, this);
    checkbox1 = new wxCheckBox(panel,wxID_ANY,"Explica",wxPoint(440,15));
    checkbox1->SetValue(true);
    checkbox1->Bind(wxEVT_CHECKBOX, &MyFrame::OnCheckClick, this);
    consola = new wxTextCtrl(panel,wxID_ANY,"", wxPoint(180,50), wxSize(330,600),wxTE_READONLY | wxTE_MULTILINE | wxBORDER_SIMPLE);
    canvas = new MyGraphCanvas(panel, wxPoint(520, 50), wxSize(750, 600));
    exporta = new wxButton(panel, wxID_ANY, "Exporta", wxPoint(1080, 10));
    limpia = new wxButton(panel, wxID_ANY, "Limpia", wxPoint(520, 10));
    limpia->Bind(wxEVT_BUTTON, &MyFrame::OnlimpiaClick, this);
    wxFrameBase::CreateStatusBar();
    grafica2d = new wxButton(panel, wxID_ANY, "Grafica 2D", wxPoint(580, 10));
    grafica2d->Bing(wxEVT_BUTTON,&MyFrame::OnButton2DClick, this);
    grafica3d = new wxButton(panel, wxID_ANY, "Grafica 3D", wxPoint(640, 10));
    grafica3d->Bing(wxEVT_BUTTON,&MyFrame::OnButton3DClick, this);
}

void MyFrame::OnOpenExplorer(const wxCommandEvent& event) {
    wxUnusedVar(event);
    // Filtramos
    wxFileDialog openFileDialog(this, "Selecciona un archivo", "", "",
                       R"(*.txt;*.csv;)",
                       wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_OK) {
        string path = openFileDialog.GetPath().ToStdString();

        // 1. Llamamos al algoritmo y lo imprimimos en el visualizador
        //procesar entrada también guarda los datos en un string.
        string nombreParaMostrar = io::procesarEntrada(path);
        filesystem::path p(path);
        ifstream archivo(p);
        stringstream buffer;
        buffer << archivo.rdbuf();
        string datos = buffer.str();
        textbox2->SetValue(datos);
        log("Datos cargados\n",consola);
        log("Semilla predeterminada: 1\n",consola);
        canvas->Refresh();
        calcula->Enable();
    }
}

void MyFrame::OnCalculaClick(wxCommandEvent& event) {
    //Feedback visual en la barra de estado
    SetStatusText("Cálculo ejecutado desde Algoritmo.");
    log("===================================\n",consola);
    // 2. Llamamos al algoritmo y le pasamos nuestra consola
    Algoritmo::max_min_ini(consola);
    canvas->Refresh();
}


void MyFrame::OnlimpiaClick(wxCommandEvent &event){
    consola->Clear();
}

void MyFrame::OnEscritura(wxCommandEvent& event){
    // Obtenemos el texto del textbox que disparó el evento
    wxString texto = event.GetString();

    if (!texto.IsEmpty()) {
        int valorTemporal;
        if (texto.ToInt(&valorTemporal)) {
            // Actualizamos la variable en la clase Algoritmo
            Algoritmo::seed = (int)valorTemporal;
            log(("Semilla actualizada: "+std::to_string(valorTemporal)+'\n'),consola);
        }
    }
}

void MyFrame::log(string msg, wxTextCtrl *out) {
    out->AppendText(msg);
}

void MyFrame::OnCheckClick(wxCommandEvent& event) {
    Algoritmo::verbo = event.IsChecked();
}

void MyFrame::OnButton2DClick(wxCommandEvent& event) {
    canvas->SetModo3D(false);
}

void MyFrame::OnButton3DClick(wxCommandEvent& event) {
    canvas->SetModo3D(true);
}
