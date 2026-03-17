#include "GUI.h"
#include "maxmin.h"
#include "graficador.h"
#include <wx/wx.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "io.h"
#include "chainmap.h"
#include "kmeans.h"
#include "isodata.h"
#include "dbscan.h"
using namespace std;

MyFrame::MyFrame() : wxFrame(nullptr, wxID_ANY, "Programa", wxPoint(50, 50), wxSize(1500, 900)) {
    auto* panel = new wxPanel(this, wxID_ANY); // El panel puede seguir siendo local si no lo vas a modificar después

    cargar_archivo = new wxButton(panel, wxID_ANY, "Cargar Archivo", wxPoint(10, 10), wxSize(150, 30));
    cargar_archivo->Bind(wxEVT_BUTTON, &MyFrame::OnOpenExplorer, this);
    textbox2 = new wxTextCtrl(panel, wxID_ANY, "", wxPoint(10,50), wxSize(120,780), wxTE_READONLY | wxTE_MULTILINE | wxHSCROLL| wxBORDER_SIMPLE);
    textbox2->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    calcula = new wxButton(panel,wxID_ANY, "Calcula", wxPoint(200, 10));
    calcula->Disable();
    calcula->Bind(wxEVT_BUTTON, &MyFrame::OnCalculaClick, this);
    wxArrayString opciones;
    opciones.Add("Max-Min");
    opciones.Add("Chain-map");
    opciones.Add("k-Means");
    opciones.Add("ISODATA");
    opciones.Add("Db-Scan");
    //opciones.Add("soon TM");
    choice = new wxChoice(panel, wxID_ANY, wxPoint(300, 10), wxDefaultSize, opciones);
    choice->Bind(wxEVT_CHOICE, &MyFrame::OnAlgoritmoSelect, this);
    choice->SetSelection(0);//default a max-min

    etiqueta1 = new wxStaticText(panel, wxID_ANY, "Semilla: ", wxPoint(420,15));
    textbox1 = new wxTextCtrl(panel, wxID_ANY, "1", wxPoint(470,10),wxSize(60,20),wxBORDER_SIMPLE);
    textbox1->Bind(wxEVT_TEXT, &MyFrame::OnEscritura, this);
    checkbox1 = new wxCheckBox(panel,wxID_ANY,"Explica",wxPoint(540,15));
    checkbox1->SetValue(true);
    checkbox1->Bind(wxEVT_CHECKBOX, &MyFrame::OnCheckClick, this);
    consola = new wxTextCtrl(panel,wxID_ANY,"", wxPoint(140,50), wxSize(570,780),wxTE_READONLY | wxTE_MULTILINE | wxHSCROLL | wxBORDER_SIMPLE);
    consola->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    canvas = new MyGraphCanvas(panel, wxPoint(720, 50), wxSize(750, 600));
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
        canvas->Refresh();
        if (choice->GetSelection() != wxNOT_FOUND && !maxmin::matrizDatos.empty()) {
            calcula->Enable();
        } else {
            calcula->Disable();
        }
    }
}

void MyFrame::OnCalculaClick(wxCommandEvent& event) {
    //Validar selección
    int algoritmoSeleccionado = choice->GetSelection();
    //Obtener parámetros de la interfaz (Ejemplo: Semilla)
    long semilla_ui = 1;
    textbox1->GetValue().ToLong(&semilla_ui); // Lee el valor del textbox1 y lo convierte a número
    // Preparar GUI
    SetStatusText("Calculo en proceso...");
    consola->Clear();
    canvas->LimpiarGrafico();
    //Ejecutar el algoritmo seleccionado
    switch (algoritmoSeleccionado) {
        case 0: // Opción: "Max-Min"
            maxmin::seed = (int)semilla_ui;
            maxmin::max_min_ini(consola);
            // Le pasamos los datos de maxmin al graficador
            canvas->SetDatos(maxmin::matrizDatos, maxmin::listaIndices, true);
            break;
        case 1: // Opción: "Chain Map"
            chainmap::seed = (int)semilla_ui;
            chainmap::matrizDatos = maxmin::matrizDatos;
            chainmap::umbral = maxmin::umbral;
            chainmap::ejecutar(consola);
            // Le pasamos los datos de chainmap al graficador
            canvas->SetDatos(chainmap::matrizDatos, chainmap::listaIndices, false);
            break;
        case 2: // Opción: "k-means"
            kmeans::seed = (int)semilla_ui;
            kmeans::matrizDatos = maxmin::matrizDatos;
            kmeans::ejecutar(consola);
            canvas->SetDatos(kmeans::matrizDatos, kmeans::listaIndices, false);
            break;
        case 3: // Opción: "ISODATA"
            isodata::seed = (int)semilla_ui;
            // Cargamos los datos originales leídos del archivo
            isodata::matrizDatos = maxmin::matrizDatos;
            // Ejecutamos (esto hará las divisiones/fusiones y llamará a kmeans internamente)
            isodata::ejecutar(consola);
            // Graficamos usando los índices finales generados por k-Means en el último paso
            canvas->SetDatos(kmeans::matrizDatos, kmeans::listaIndices, false);
            break;
        case 4: // Opción: "Db-Scan"
            // Le pasamos los datos originales
            dbscan::matrizDatos = maxmin::matrizDatos;
            // Ejecutamos la búsqueda de densidad
            dbscan::ejecutar(consola);
            // Mandamos los resultados (incluyendo el ruido en -2) al graficador
            canvas->SetDatos(dbscan::matrizDatos, dbscan::listaIndices, false);
            break;
    }
    // Ahora sí, cuando haga Refresh, tendrá los datos correctos
    canvas->Refresh();
    SetStatusText("Calculo finalizado.");
}


void MyFrame::OnlimpiaClick(wxCommandEvent& event) {
    if (consola) { consola->Clear();}
    if (canvas) {canvas->LimpiarGrafico();}
    textbox2->Clear();
    SetStatusText("Interfaz y gráficos limpiados.");
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
    chainmap::verbo = event.IsChecked();
    kmeans::verbo = event.IsChecked();
    isodata::verbo = event.IsChecked();
    dbscan::verbo = event.IsChecked();
}

void MyFrame::OnButton2DClick(wxCommandEvent& event) {
    canvas->SetModo3D(false);
}

void MyFrame::OnButton3DClick(wxCommandEvent& event) {
    canvas->SetModo3D(true);
}
