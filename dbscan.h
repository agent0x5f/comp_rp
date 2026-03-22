//
// Created by Mat on 16/03/2026.
//

#ifndef APPVISIONCENIDET_DBSCAN_H
#define APPVISIONCENIDET_DBSCAN_H

#include <vector>
#include <string>
#include <wx/textctrl.h>

class dbscan {
public:
    static std::vector<std::vector<double>> matrizDatos;
    static std::vector<std::vector<std::string>> matrizDatosCat;
    static std::vector<int> listaIndices;
    static bool verbo;
    static double epsilon;
    static int minPts;
    static int num_clases;

    // Las funciones sobrecargadas para la distancia
    static double calcularDistancia(const std::vector<double>& p1, const std::vector<double>& p2);
    static double calcularDistancia(const std::vector<std::string>& p1, const std::vector<std::string>& p2);


    template <typename T>
    static std::vector<int> buscarVecinos(const std::vector<std::vector<T>>& matriz, int indicePunto, wxTextCtrl* consola);

    template <typename T>
    static void expandirCluster(const std::vector<std::vector<T>>& matriz, int indicePunto, std::vector<int>& vecinos, int claseActual, wxTextCtrl* consola);

    template <typename T>
    static void ejecutarGenerico(const std::vector<std::vector<T>>& matriz, wxTextCtrl* consola);

    // Tus "envoltorios" para que la GUI los llame fácil
    static void ejecutar(wxTextCtrl* consola);
    static void ejecutarCat(wxTextCtrl* consola);


    static std::string a2decimal(double number);
    static std::string logM(const int pos);
    static void log(const std::string& msg, wxTextCtrl *out);
};

template <typename T>
std::vector<int> dbscan::buscarVecinos(const std::vector<std::vector<T>>& matriz, int indicePunto, wxTextCtrl* consola) {
    std::vector<int> vecinos;
    if (verbo && consola) log("  Midiendo distancias desde P#" + std::to_string(indicePunto) + "...\n", consola);

    for (int i = 0; i < (int)matriz.size(); ++i) {
        // Si matriz es double, llama a Euclidiana. Si es string, llama a Categórica.
        double dist = calcularDistancia(matriz[indicePunto], matriz[i]);

        if (i == indicePunto) {
            vecinos.push_back(i);
        } else {
            if (verbo && consola) log("    d(P#" + std::to_string(indicePunto) + ", P#" + std::to_string(i) + ") = " + a2decimal(dist), consola);
            if (dist <= epsilon) {
                vecinos.push_back(i);
                if (verbo && consola) log(" <= " + a2decimal(epsilon) + " [Vecino]\n", consola);
            } else {
                if (verbo && consola) log(" > " + a2decimal(epsilon) + " [Rechazado]\n", consola);
            }
        }
    }
    if (verbo && consola) log("Total vecinos en el radio: " + std::to_string(vecinos.size()) + "\n", consola);
    return vecinos;
}

template <typename T>
void dbscan::expandirCluster(const std::vector<std::vector<T>>& matriz, int indicePunto, std::vector<int>& vecinos, int claseActual, wxTextCtrl* consola) {
    listaIndices[indicePunto] = claseActual;
    if (verbo && consola) log("[Expandiendo cluster" + std::to_string(claseActual) + "] a partir del P#" + std::to_string(indicePunto) + "\n", consola);

    for (size_t i = 0; i < vecinos.size(); ++i) {
        int v = vecinos[i];
        if (v == indicePunto) continue;
        if (verbo && consola) log("Evaluando vecino pendiente P#" + std::to_string(v) + "...\n", consola);

        if (listaIndices[v] == -2) {
            listaIndices[v] = claseActual;
            if (verbo && consola) log("  Punto #" + std::to_string(v) + " rescatado del ruido -> Grupo " + std::to_string(claseActual) + "\n", consola);
        }
        if (listaIndices[v] == -1) {
            listaIndices[v] = claseActual;
            if (verbo && consola) log("  Punto #" + std::to_string(v) + " agregado -> Grupo " + std::to_string(claseActual) + "\n", consola);

            // Llama recursivamente buscando vecinos, pasándole la matriz correcta
            std::vector<int> nuevosVecinos = buscarVecinos(matriz, v, consola);

            if ((int)nuevosVecinos.size() >= minPts) {
                if (verbo && consola) log("Si tiene " + std::to_string(nuevosVecinos.size()) + " vecinos (>= " + std::to_string(minPts) + "). Sus vecinos se unen a revision.\n", consola);
                for (int nv : nuevosVecinos) {
                    if (listaIndices[nv] == -1 || listaIndices[nv] == -2) {
                        vecinos.push_back(nv);
                    }
                }
            } else {
                if (verbo && consola) log("No, solo tiene " + std::to_string(nuevosVecinos.size()) + " vecinos. No se expande desde aqui.\n", consola);
            }
        } else {
            if (verbo && consola) log("  -> P#" + std::to_string(v) + " ya pertenece al Grupo " + std::to_string(listaIndices[v]) + ", ignoramos.\n", consola);
        }
    }
}

template <typename T>
void dbscan::ejecutarGenerico(const std::vector<std::vector<T>>& matriz, wxTextCtrl* consola) {
    if (matriz.empty()) return;

    listaIndices.assign(matriz.size(), -1);
    num_clases = 0;

    if (consola) {
        log("--- Iniciando DBSCAN ---\n", consola);
        log("Epsilon: " + a2decimal(epsilon) + " | MinPts: " + std::to_string(minPts) + "\n", consola);
    }

    for (int i = 0; i < (int)matriz.size(); ++i) {
        if (listaIndices[i] != -1) continue;

        // Le pasamos la matriz para que sepa en qué dimensión está trabajando
        std::vector<int> vecinos = buscarVecinos(matriz, i, consola);

        if ((int)vecinos.size() < minPts) {
            listaIndices[i] = -2;
            if (verbo && consola) log("Punto #" + std::to_string(i) + " es Ruido.\n", consola);
        } else {
            if (verbo && consola) log("Punto #" + std::to_string(i) + " es Nucleo. Creando Grupo " + std::to_string(num_clases) + "\n", consola);

            expandirCluster(matriz, i, vecinos, num_clases, consola);
            num_clases++;
        }
    }
    if (consola) log("--- DBSCAN Finalizado: " + std::to_string(num_clases) + " grupos encontrados ---\n", consola);
}


#endif //APPVISIONCENIDET_DBSCAN_H