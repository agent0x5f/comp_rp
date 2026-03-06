//
// Created by Mat on 05/03/2026.
//

#include <wx/textctrl.h>
#include "kmeans.h"
#include <wx/wx.h>
#include <vector>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <set>

using namespace std;

vector<vector<double>> kmeans::matrizDatos;
vector<int> kmeans::listaIndices;
bool kmeans::verbo = true;
int kmeans::num_clases = 0;

void kmeans::iniciar_centroides() {
    centroides.clear(); //limpiamos
    std::set<int> indices_usados;
    //vamos a seleccionar nuestros k centroides de forma random
    while (indices_usados.size() < (size_t)k) {
        int elegido = obtenerIndiceAleatorio();

        // insert() devuelve un par, el segundo valor es true si se insertó con éxito/no estaba repetido
        if (indices_usados.insert(elegido).second) {
            centroides.push_back(matrizDatos[elegido]);
        }
    }
}

double kmeans::umbral = 0.0;
int kmeans::seed = 1; //defaults - se lee del GUI
int kmeans::k = 1; //defaults - se lee del file

string kmeans::a2decimal(double number) {
    stringstream ss;
    ss << std::fixed << std::setprecision(2) << number;
    return ss.str();
}

string kmeans::logM(const int pos) {
    string msg = "[";
    for (size_t i = 0; i < matrizDatos[pos].size(); ++i) {
        msg += a2decimal(matrizDatos[pos][i]);
        if (i < matrizDatos[pos].size() - 1) msg += ", ";
    }
    msg += "]";
    return msg;
}

void kmeans::log(const string& msg, wxTextCtrl *out) {
    if (out) {
        out->AppendText(msg);
        out->Update();
    }
}
//obtenemos el primer elemento al azar
int kmeans::obtenerIndiceAleatorio() {
    if (matrizDatos.empty()) return -1;
    std::mt19937 generador(seed);  //semilla del textbox
    std::uniform_int_distribution<int> distribucion(0, (int)matrizDatos.size() - 1);//Definimos el rango: de 0 al total de filas menos 1
    return distribucion(generador); //retornamos el número generado
}
//calcula la distancia entre dos puntos de n dimensiones
double kmeans::calcularDistancia(const std::vector<double>& p1, const std::vector<double>& p2) {
    double suma = 0.0;
    const size_t dimensiones = std::min(p1.size(), p2.size());
    for (size_t i = 0; i < dimensiones; ++i) {
        const double diff = (p1[i]) - (p2[i]);
        suma += diff * diff;
    }
    return std::sqrt(suma);
}

// Busca el vecino más cercano que todavia no tiene clase asignada
int kmeans::obtenerCercanoNoVisitado(int indiceActual, wxTextCtrl *out) {
    int mas_cercano = -1;
    double minDistancia = 999999.0;

    for (int i = 0; i < (int)matrizDatos.size(); ++i) { //recorro los elementos
        // Ignoramos el punto actual y los que ya tienen clase (!= -1)
        if (i == indiceActual || listaIndices[i] != -1) continue;

        if (!matrizDatos[i].empty()) {
            double dist = calcularDistancia(matrizDatos[i], matrizDatos[indiceActual]);
            //calculo la distancia y veo si es el más cercano actualmente.
            if (dist < minDistancia) {
                minDistancia = dist;
                mas_cercano = i;
            }
        }
    }
    return mas_cercano;
}

// Función principal que ejecuta el algoritmo chainmap
void kmeans::ejecutar(wxTextCtrl* out) {
    if (matrizDatos.empty() || !out) {
        log("Error: No hay datos para procesar.\n", out);
        return;
    }

    listaIndices.assign(matrizDatos.size(), -1);
    num_clases = 0;
    log("K: " + a2decimal(k) + "\n", out);
    // Elegimos el primer punto al azar
    int puntoActual = obtenerIndiceAleatorio();
    listaIndices[puntoActual] = num_clases; // Lo asignamos a la Clase 0
    log("Inicio en #" + to_string(puntoActual) + " " + logM(puntoActual) + " -> Clase 1\n", out);


    log("Terminado: " + to_string(num_clases + 1) + " clases generadas.\n", out);
}