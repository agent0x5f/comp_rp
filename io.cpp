//
// Created by Miguel on 17/02/26.
//

#include "io.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "chainmap.h"
#include "maxmin.h"
#include "kmeans.h"

using namespace std;

string io::procesarEntrada(string const& path, wxTextCtrl* salida) {
    std::ifstream archivo(path);
    if (!archivo.is_open()) return "Error al abrir";

    maxmin::matrizDatos.clear();

    // Leemos el archivo en un solo string para poder analizar el formato
    std::stringstream buffer;
    buffer << archivo.rdbuf();
    string contenido = buffer.str();
    archivo.close(); // Ya lo tenemos en memoria, podemos cerrar el archivo

    if (contenido.empty()) return "Archivo vacío";

    // Buscamos el primer carácter válido para identificar el formato
    size_t primerCaracter = contenido.find_first_not_of(" \t\r\n");
    if (primerCaracter == string::npos) return "Archivo vacío";

    // LECTURA FORMATO JSON
    if (contenido[primerCaracter] == '{') {
        if (salida) maxmin::log("Detectado formato JSON...\n", salida);

        // Extracción del Umbral
        size_t posUmbral = contenido.find("\"umbral\"");
        if (posUmbral != string::npos) {
            size_t posDosPuntos = contenido.find(":", posUmbral);
            if (posDosPuntos != string::npos) {
                try {
                    maxmin::umbral = std::stod(contenido.substr(posDosPuntos + 1));
                    if (salida) maxmin::log("Umbral actualizado: " + std::to_string(maxmin::umbral) + "\n", salida);
                } catch (...) {
                    if (salida) maxmin::log("Error: Formato de umbral incorrecto en el JSON.\n", salida);
                }
            }
        }

        // Extracción del K - para kmeans
        size_t posk = contenido.find("\"k\"");
        if (posk != string::npos) {
            size_t posDosPuntos = contenido.find(":", posk);
            if (posDosPuntos != string::npos) {
                try {
                    kmeans::k = std::stoi(contenido.substr(posDosPuntos + 1));
                    if (salida) kmeans::log("K actualizado: " + std::to_string(kmeans::k) + "\n", salida);
                } catch (...) {
                    if (salida) kmeans::log("Error: Formato de k incorrecto en el JSON.\n", salida);
                }
            }
        }

        // Extracción de Datos
        size_t posDatos = contenido.find("\"datos\"");
        if (posDatos != string::npos) {
            size_t posInicioArreglo = contenido.find("[", posDatos);
            size_t posFinArreglo = contenido.rfind("]"); // Último corchete

            if (posInicioArreglo != string::npos && posFinArreglo != string::npos) {
                string stringDatos = contenido.substr(posInicioArreglo + 1, posFinArreglo - posInicioArreglo - 1);

                size_t i = 0;
                while (i < stringDatos.length()) {
                    size_t inicioSub = stringDatos.find("[", i);
                    if (inicioSub == string::npos) break;
                    size_t finSub = stringDatos.find("]", inicioSub);
                    if (finSub == string::npos) break;

                    string puntoStr = stringDatos.substr(inicioSub + 1, finSub - inicioSub - 1);

                    vector<double> filaActual;
                    stringstream ssPunto(puntoStr);
                    string token;
                    while (getline(ssPunto, token, ',')) {
                        try {
                            filaActual.push_back(stod(token));
                        } catch (...) { /* Ignorar espacios extra */ }
                    }
                    if (!filaActual.empty()) maxmin::matrizDatos.push_back(filaActual);

                    i = finSub + 1; // Avanzar al siguiente sub-arreglo
                }
            }
        }
    }
    //  LECTURA FORMATO CLÁSICO (Línea por línea)
    else {
        if (salida) maxmin::log("Detectado formato clásico...\n", salida);

        stringstream ssArchivo(contenido);
        string linea;

        while (getline(ssArchivo, linea)) {
            // Quitamos espacios en blanco accidentales al inicio
            linea.erase(0, linea.find_first_not_of(" \t\r\n"));

            // Comentarios: Si la línea está vacía o empieza con '@', pero NO es un parametro de entrada.
            if (linea.empty()) continue;
            if (linea[0] == '@' && linea.find("umbral") == string::npos && linea.find("k:") == string::npos) continue;
            // Extracción del Umbral: Buscamos la etiqueta "@umbral:"
            if (linea.find("@umbral:") != string::npos) {
                try {
                    size_t posDospuntos = linea.find(":") + 1;
                    string valorStr = linea.substr(posDospuntos);
                    maxmin::umbral = std::stod(valorStr);
                    if (salida) maxmin::log("Umbral actualizado: " + std::to_string(maxmin::umbral) + "\n", salida);
                } catch (...) {
                    if (salida) maxmin::log("Error: Formato de umbral incorrecto en el archivo.\n", salida);
                }
                continue; // No agregamos esta línea a la matriz
            }

            // Extracción del Umbral: Buscamos la etiqueta "@umbral:"
            if (linea.find("@k:") != string::npos) {
                try {
                    size_t posDospuntos = linea.find(":") + 1;
                    string valorStr = linea.substr(posDospuntos);
                    kmeans::k = std::stoi(valorStr);
                    if (salida) maxmin::log("k actualizado: " + std::to_string(kmeans::k) + "\n", salida);
                } catch (...) {
                    if (salida) maxmin::log("Error: Formato de k incorrecto en el archivo.\n", salida);
                }
                continue; // No agregamos esta línea a la matriz
            }

            // Lectura de coordenadas (X, Y)
            stringstream ss(linea);
            string valor;
            vector<double> filaActual;

            while (getline(ss, valor, ',')) {
                try {
                    filaActual.push_back(stod(valor));
                } catch (...) { /* Ignorar basura */ }
            }

            if (!filaActual.empty()) {
                maxmin::matrizDatos.push_back(filaActual);
            }
        }
    }

    // Inicializar listaIndices con -1 según el tamaño de los datos leídos
    maxmin::listaIndices.assign(maxmin::matrizDatos.size(), -1);

    std::filesystem::path p(path);
    return p.filename().string();
}