//
// Created by Miguel on 17/02/26.
//

#include "io.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include "algoritmo.h"
using namespace std;

string io::procesarEntrada(string const& path, wxTextCtrl* salida) {
    std::ifstream archivo(path);
    if (!archivo.is_open()) return "Error al abrir";

    Algoritmo::matrizDatos.clear();
    string linea;

    while (getline(archivo, linea)) {
        // Quitamos espacios en blanco accidentales al inicio y final
        linea.erase(0, linea.find_first_not_of(" \t\r\n"));

        // Comentarios: Si la línea está vacía o empieza con '@', pero NO es el umbral
        if (linea.empty()) continue;
        if (linea[0] == '@' && linea.find("umbral") == string::npos) continue;

        // Extracción del Umbral: Buscamos la etiqueta "@umbral:"
        if (linea.find("@umbral:") != string::npos) {
            try {
                // Buscamos la posición después de los dos puntos
                size_t posDospuntos = linea.find(":") + 1;
                string valorStr = linea.substr(posDospuntos);
                // Convertimos el resto de la línea a double
                Algoritmo::umbral = std::stod(valorStr);
                if (salida) Algoritmo::log("Umbral actualizado: " + std::to_string(Algoritmo::umbral) + "\n", salida);
            } catch (...) {
                if (salida) Algoritmo::log("Error: Formato de umbral incorrecto en el archivo.\n", salida);
            }
            continue; // No agregamos esta línea a la matriz de datos
        }

        // 4. Lectura de coordenadas (X, Y)
        stringstream ss(linea);
        string valor;
        vector<double> filaActual;

        while (getline(ss, valor, ',')) {
            try {
                filaActual.push_back(stod(valor));
            } catch (...) { /* Ignorar basura */ }
        }

        if (!filaActual.empty()) {
            Algoritmo::matrizDatos.push_back(filaActual);
        }
    }

    archivo.close();
    Algoritmo::listaIndices.assign(Algoritmo::matrizDatos.size(), -1);

    std::filesystem::path p(path);
    return p.filename().string();
}
