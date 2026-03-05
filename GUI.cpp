#include "GUI.h"
#include "maxmin.h"
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
    textbox2 = new wxTextCtrl(panel, wxID_ANY, "", wxPoint(10,50), wxSize(120,600), wxTE_READONLY | wxTE_MULTILINE | wxHSCROLL| wxBORDER_SIMPLE);
    textbox2->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    calcula = new wxButton(panel,wxID_ANY, "Calcula", wxPoint(200, 10));
    calcula->Disable();
    calcula->Bind(wxEVT_BUTTON, &MyFrame::OnCalculaClick, this);
    wxArrayString opciones;
    opciones.Add("Max-Min");
    opciones.Add("Chain-map");
    //opciones.Add("soon TM");
    choice = new wxChoice(panel, wxID_ANY, wxPoint(280, 10), wxDefaultSize, opciones);
    choice->Bind(wxEVT_CHOICE, &MyFrame::OnAlgoritmoSelect, this);
    choice->SetSelection(0);//default a max-min

    etiqueta1 = new wxStaticText(panel, wxID_ANY, "Semilla: ", wxPoint(420,15));
    textbox1 = new wxTextCtrl(panel, wxID_ANY, "1", wxPoint(470,10),wxSize(60,20),wxBORDER_SIMPLE);
    textbox1->Bind(wxEVT_TEXT, &MyFrame::OnEscritura, this);
    checkbox1 = new wxCheckBox(panel,wxID_ANY,"Explica",wxPoint(540,15));
    checkbox1->SetValue(true);
    checkbox1->Bind(wxEVT_CHECKBOX, &MyFrame::OnCheckClick, this);
    consola = new wxTextCtrl(panel,wxID_ANY,"", wxPoint(140,50), wxSize(370,600),wxTE_READONLY | wxTE_MULTILINE | wxHSCROLL | wxBORDER_SIMPLE);
    consola->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    canvas = new MyGraphCanvas(panel, wxPoint(520, 50), wxSize(750, 600));
    //exporta = new wxButton(panel, wxID_ANY, "Exporta", wxPoint(1080, 10));
    limpia = new wxButton(panel, wxID_ANY, "Limpia", wxPoint(620, 10));
    limpia->Bind(wxEVT_BUTTON, &MyFrame::OnlimpiaClick, this);
    wxFrameBase::CreateStatusBar();
    grafica2d = new wxButton(panel, wxID_ANY, "Grafica 2D", wxPoint(720, 10));
    grafica2d->Bind(wxEVT_BUTTON,&MyFrame::OnButton2DClick, this);
    grafica3d = new wxButton(panel, wxID_ANY, "Grafica 3D", wxPoint(820, 10));
    grafica3d->Bind(wxEVT_BUTTON,&MyFrame::OnButton3DClick, this);
}

void MyFrame::OnAlgoritmoSelect(wxCommandEvent& event) {
    if (choice->GetSelection() != wxNOT_FOUND && !maxmin::matrizDatos.empty()) {
        calcula->Enable();
    } else {
        calcula->Disable();
    }
}

void MyFrame::OnOpenExplorer(const wxCommandEvent& event) {
    wxUnusedVar(event);
    // Filtramos
    wxFileDialog openFileDialog(this, "Selecciona un archivo", "", "",
                       R"(*.txt;*.csv;*.json;)",
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
        if (choice->GetSelection() != wxNOT_FOUND && !maxmin::matrizDatos.empty()) {
            calcula->Enable();
        } else {
            calcula->Disable();
        }
    }
}

void MyFrame::OnCalculaClick(wxCommandEvent& event) {
   int algoritmoSeleccionado = choice->GetSelection();
    //si de alguna forma hizo clic sin seleccionar, cancelamos
    if (algoritmoSeleccionado == wxNOT_FOUND) {
        wxMessageBox("Por favor selecciona un algoritmo primero.", "Aviso", wxICON_WARNING);
        return;
    }
    SetStatusText("Cálculo ejecutado...");
    // Ejecutamos el algoritmo según la opción
    switch (algoritmoSeleccionado) {
        case 0: maxmin::max_min_ini(consola);break;
        case 1: break;
        default:
            log("Error: Algoritmo no reconocido.\n", consola);
            break;
    }
    // Actualizamos el canvas después de que el algoritmo termine
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
            // Actualizamos la variable en la clase maxmin
            maxmin::seed = (int)valorTemporal;
            log(("Semilla actualizada: "+std::to_string(valorTemporal)+'\n'),consola);
        }
    }
}

void MyFrame::log(string msg, wxTextCtrl *out) {
    out->AppendText(msg);
}

void MyFrame::OnCheckClick(wxCommandEvent& event) {
    maxmin::verbo = event.IsChecked();
}

void MyFrame::OnButton2DClick(wxCommandEvent& event) {
    canvas->SetModo3D(false);
}

void MyFrame::OnButton3DClick(wxCommandEvent& event) {
    canvas->SetModo3D(true);
}
