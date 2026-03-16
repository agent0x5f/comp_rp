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
static std::vector<std::vector<double>> centroides;
int kmeans::seed = 1; //defaults - se lee del GUI
int kmeans::k = 3; //defaults - se lee del file


void kmeans::iniciar_centroides() {
    centroides.clear(); //limpiamos
    std::set<int> indices_usados;
    //vamos a seleccionar nuestros k centroides de forma random
    std::mt19937 generador(seed);
    std::uniform_int_distribution<int> distribucion(0, (int)matrizDatos.size() - 1);
    while (indices_usados.size() < (size_t)k) {
        int elegido = distribucion(generador);

        // insert() devuelve un par, el segundo valor es true si se insertó con éxito/no estaba repetido
        if (indices_usados.insert(elegido).second) {
            centroides.push_back(matrizDatos[elegido]);
        }
    }
}

void kmeans::asignacion(wxTextCtrl* consola) {
    for (int i = 0; i < (int)matrizDatos.size(); ++i) {
        auto punto = matrizDatos[i];
        int mas_cercano = 0;
        double minDistancia = calcularDistancia(punto, centroides[0]);

        string log_linea = "";
        if (verbo) log_linea = "P" + std::to_string(i) + " " + p_aString(punto) + "->dC0: " + a2decimal(minDistancia);

        // Comparamos contra el resto de los centroides
        for (int j = 1; j < k; j++) {
            auto d = calcularDistancia(punto, centroides[j]);
            if (verbo) log_linea += ", dC" + std::to_string(j) + ":" + a2decimal(d);

            if (d < minDistancia) {
                minDistancia = d;
                mas_cercano = j; // Actualizamos el ganador temporal
            }
        }
        if (verbo && consola) {
            log_linea += " ->Grupo " + std::to_string(mas_cercano+1) + "\n";
            consola->AppendText(log_linea);
        }
        // Guardamos el índice del centroide ganador
        kmeans::listaIndices[i] = mas_cercano;
    }
}

void kmeans::recalcula_centroides() {
    std::vector<std::vector<double>> nuevos_centroides;
    const int dimensiones = matrizDatos[0].size();
    // Calculamos el nuevo centro para cada clúster 'x'
    for (int x = 0; x < k; x++) {
        std::vector<double> suma(dimensiones, 0.0);
        int puntos_en_cluster = 0;
        // Recorremos los puntos para ver cuáles pertenecen a este clúster 'x'
        for (size_t p = 0; p < matrizDatos.size(); ++p) {
            if (listaIndices[p] == x) {
                for (int d = 0; d < dimensiones; ++d) { // Sumamos cada una de sus dimensiones
                    suma[d] += matrizDatos[p][d];
                }
                puntos_en_cluster++;
            }
        }
        // Si encontramos puntos en este clúster, calculamos el promedio
        if (puntos_en_cluster > 0) {
            for (int d = 0; d < dimensiones; ++d) {
                suma[d] /= puntos_en_cluster; // Promedio = suma / total -ojo con el div entre 0
            }
            nuevos_centroides.push_back(suma);
        } else {
            // Si un clúster se quedó vacío, conservamos su centroide viejo para que no desaparezca.
            nuevos_centroides.push_back(centroides[x]);
        }
    }
    // Actualizamos los centroides con los nuevos
    centroides = nuevos_centroides;
}

std::string kmeans::p_aString(const std::vector<double> &punto) {
    string msg = "[";
    for (size_t i = 0; i < punto.size(); ++i) {
        msg += a2decimal(punto[i]);
        if (i < punto.size() - 1) msg += ", ";
    }
    msg += "]";
    return msg;
}


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

//Bucle principal de asignación y recálculo
void kmeans::ciclo_principal(wxTextCtrl* consola) {
    bool convergencia = false;
    int iteracion = 0;
    int max_iteraciones = 100; // Límite de cansancio

    // main loop
    while (!convergencia && iteracion < max_iteraciones) {
        iteracion++;
        if (consola) consola->AppendText("Iteracion " + std::to_string(iteracion) + "...\n");

        if (verbo && consola) {
            for (int c = 0; c < k; ++c) {
                consola->AppendText("Centroide " + std::to_string(c) + ": " + p_aString(centroides[c]) + "\n");
            }
        }

        std::vector<int> indices_anteriores = listaIndices;
        asignacion(consola);  // Asignar cada punto a su centroide más cercano

        // Comprobación de convergencia
        if (listaIndices == indices_anteriores) {
            convergencia = true;
            if (consola) consola->AppendText("Convergencia alcanzada en la iter: " + std::to_string(iteracion) + "\n");
            break;
        }
        recalcula_centroides(); // Si hubo cambios, recalculamos los centroides
    }
    if (!convergencia && consola) {
        consola->AppendText("Fin por límite de iteraciones (" + std::to_string(max_iteraciones) + ").\n");
    }
    if (consola) consola->AppendText("Proceso k-Means finalizado con exito.\n");
}

// Semillas random
void kmeans::ejecutar(wxTextCtrl* consola) {
    if (matrizDatos.empty() || k <= 0) {
        if (consola) consola->AppendText("Error: Datos vacíos o K inválido.\n");
        return;
    }
    if (k > (int)matrizDatos.size()) {
        k = matrizDatos.size();
        if (consola) consola->AppendText("Nota: K mayor al número de puntos. Ajustando a " + std::to_string(k) + "\n");
    }

    listaIndices.assign(matrizDatos.size(), -1);
    if (consola) consola->AppendText("Iniciando k-Means con K aleatorios: " + std::to_string(k) + "...\n");

    iniciar_centroides();
    ciclo_principal(consola);
}

// Semillas dadas desde fuera - ISODATA
void kmeans::ejecutar(const std::vector<std::vector<double>>& semillas_isodata, wxTextCtrl* consola) {
    if (matrizDatos.empty() || semillas_isodata.empty()) {
        if (consola) consola->AppendText("Error: Datos vacíos o semillas de ISODATA inválidas.\n");
        return;
    }

    k = semillas_isodata.size();
    centroides = semillas_isodata;

    if (k > (int)matrizDatos.size()) {
        k = matrizDatos.size();
        centroides.resize(k); // Recortamos por seguridad
        if (consola) consola->AppendText("Nota: K mayor al número de puntos. Ajustando a " + std::to_string(k) + "\n");
    }

    listaIndices.assign(matrizDatos.size(), -1);
    if (consola) consola->AppendText("Iniciando k-Means con " + std::to_string(k) + " semillas de ISODATA...\n");

    ciclo_principal(consola);
}