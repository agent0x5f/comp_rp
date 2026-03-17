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
vector<int> dbscan::listaIndices;
bool dbscan::verbo = true;
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

// Busca todos los puntos a una distancia <= epsilon
vector<int> dbscan::buscarVecinos(int indicePunto,wxTextCtrl* consola) {
    vector<int> vecinos;
    if (verbo && consola) log("  Midiendo distancias desde P#" + to_string(indicePunto) + "...\n", consola);

    for (int i = 0; i < (int)matrizDatos.size(); ++i) {
        double dist = calcularDistancia(matrizDatos[indicePunto], matrizDatos[i]);
        if (i == indicePunto) {
            vecinos.push_back(i); // El punto se cuenta a sí mismo
        } else {
            if (verbo && consola) log("    d(P#" + to_string(indicePunto) + ", P#" + to_string(i) + ") = " + a2decimal(dist), consola);
            // Evaluamos contra el umbral Epsilon
            if (dist <= epsilon) {
                vecinos.push_back(i);
                if (verbo && consola) log(" <= " + a2decimal(epsilon) + " [Vecino]\n", consola);
            } else {
                if (verbo && consola) log(" > " + a2decimal(epsilon) + " [Rechazado]\n", consola);
            }
        }
    }
    if (verbo && consola) log("Total vecinos en el radio: " + to_string(vecinos.size()) + "\n", consola);
    return vecinos;
}

// Contagia a los vecinos y a los vecinos de los vecinos
void dbscan::expandirCluster(int indicePunto, vector<int>& vecinos, int claseActual, wxTextCtrl* consola) {
    listaIndices[indicePunto] = claseActual; // Asignamos el punto núcleo al clúster
    if (verbo && consola) log("\n[Expandiendo cluster" + to_string(claseActual) + "] a partir del P#" + to_string(indicePunto) + "\n", consola);
    // vecinos va a crecer dentro de este mismo ciclo
    for (size_t i = 0; i < vecinos.size(); ++i) {
        int v = vecinos[i];
        if (v == indicePunto) continue; // Nos saltamos a nosotros mismos en el log
        if (verbo && consola) log("Evaluando vecino pendiente P#" + to_string(v) + "...\n", consola);

        // Si antes creíamos que era ruido, lo rescatamos porque cayó en la orilla de un núcleo
        if (listaIndices[v] == -2) {
            listaIndices[v] = claseActual;
            if (verbo && consola) log("  Punto #" + to_string(v) + " rescatado del ruido -> Grupo " + to_string(claseActual) + "\n", consola);
        }
        // Si nunca lo habíamos visitado
        if (listaIndices[v] == -1) {
            listaIndices[v] = claseActual; // Lo metemos al grupo
            if (verbo && consola) log("  Punto #" + to_string(v) + " agregado -> Grupo " + to_string(claseActual) + "\n", consola);

            // Si este nuevo vecino también es popular (Punto Núcleo)
            vector<int> nuevosVecinos = buscarVecinos(v,consola);
            if ((int)nuevosVecinos.size() >= minPts) {
                if (verbo && consola) log("Si tiene " + to_string(nuevosVecinos.size()) + " vecinos (>= " + to_string(minPts) + "). Sus vecinos se unen a revision.\n", consola);
                // Añadimos sus amigos a nuestra lista de pendientes para expandir la frontera
                for (int nv : nuevosVecinos) {
                    // Evitamos meter a la cola puntos que ya están asignados a un grupo
                    if (listaIndices[nv] == -1 || listaIndices[nv] == -2) {
                        vecinos.push_back(nv);
                    }
                }
            }
            else {
                if (verbo && consola) log("No, solo tiene " + to_string(nuevosVecinos.size()) + " vecinos. No se expande desde aqui.\n", consola);
            }
        }
        else {
            if (verbo && consola) log("  -> P#" + to_string(v) + " ya pertenece al Grupo " + to_string(listaIndices[v]) + ", ignoramos.\n", consola);
        }
    }
}

void dbscan::ejecutar(wxTextCtrl* consola) {
    if (matrizDatos.empty()) return;

    listaIndices.assign(matrizDatos.size(), -1);// Inicializamos todos los puntos como -1 (No visitado)
    num_clases = 0;

    if (consola) {
        log("--- Iniciando DBSCAN ---\n", consola);
        log("Epsilon: " + a2decimal(epsilon) + " | MinPts: " + to_string(minPts) + "\n", consola);
    }

    for (int i = 0; i < (int)matrizDatos.size(); ++i) {
        // Si el punto ya tiene grupo o ya es ruido definitivamente, lo ignoramos
        if (listaIndices[i] != -1) continue;

        vector<int> vecinos = buscarVecinos(i,consola); //a buscar

        // Si es Punto nucleo o ruido
        if ((int)vecinos.size() < minPts) {
            listaIndices[i] = -2; // Marcado como ruido
            if (verbo && consola) log("Punto #" + to_string(i) + " es Ruido.\n", consola);
        } else {
            // Se hace un nuevo grupo
            if (verbo && consola) log("Punto #" + to_string(i) + " es Nucleo. Creando Grupo " + to_string(num_clases) + "\n", consola);

            expandirCluster(i, vecinos, num_clases, consola);
            num_clases++; // Preparamos para el siguiente grupo que se forme
        }
    }
    if (consola) log("--- DBSCAN Finalizado: " + to_string(num_clases) + " grupos encontrados ---\n", consola);
}