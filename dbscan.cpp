//
// Created by Mat on 16/03/2026.
//

#include "dbscan.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

vector<vector<double>> dbscan::matrizDatos;
vector<vector<string>> dbscan::matrizDatosCat;
vector<int> dbscan::listaIndices;
bool dbscan::verbo = true;
bool esCategorico = false;
double dbscan::epsilon = 1.0;
int dbscan::minPts = 3;
int dbscan::num_clases = 0;

string dbscan::a2decimal(double number) {
    stringstream ss;
    ss << fixed << setprecision(2) << number;
    return ss.str();
}

string dbscan::logM(const int pos) {
    string msg = "[";
    for (size_t i = 0; i < matrizDatos[pos].size(); ++i) {
        msg += a2decimal(matrizDatos[pos][i]);
        if (i < matrizDatos[pos].size() - 1) msg += ", ";
    }
    msg += "]";
    return msg;
}

void dbscan::log(const string& msg, wxTextCtrl *out) {
    if (out) {
        out->AppendText(msg);
        out->Update();
    }
}

double dbscan::calcularDistancia(const vector<double>& p1, const vector<double>& p2) {
    double suma = 0.0;
    size_t dimensiones = min(p1.size(), p2.size());
    for (size_t i = 0; i < dimensiones; ++i) {
        double diff = p1[i] - p2[i];
        suma += diff * diff;
    }
    return sqrt(suma);
}

// distancia para datos no numéricos
double dbscan::calcularDistancia(const std::vector<std::string>& p1, const std::vector<std::string>& p2) {
    // Si tienen diferente tamaño, no se pueden comparar bien
    if (p1.size() != p2.size() || p1.empty()) return 1.0;

    int diferencias = 0;

    // Comparamos atributo por atributo
    for (size_t i = 0; i < p1.size(); ++i) {
        if (p1[i] != p2[i]) {
            diferencias++;
        }
    }
    // Retornamos la proporción de diferencias
    return (double)diferencias / p1.size();
}

void dbscan::ejecutar(wxTextCtrl* consola) {
    ejecutarGenerico(matrizDatos, consola);
}

void dbscan::ejecutarCat(wxTextCtrl* consola) {
    ejecutarGenerico(matrizDatosCat, consola);
}