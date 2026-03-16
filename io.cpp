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
#include "isodata.h"

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

        // Extracción del K - Sincronizado para kmeans e isodata
        size_t posk = contenido.find("\"k\"");
        if (posk != string::npos) {
            size_t posDosPuntos = contenido.find(":", posk);
            if (posDosPuntos != string::npos) {
                try {
                    kmeans::k = std::stoi(contenido.substr(posDosPuntos + 1));
                    isodata::k_esperado = kmeans::k;
                    if (salida) maxmin::log("K actualizado: " + std::to_string(kmeans::k) + "\n", salida);
                } catch (...) {
                    if (salida) maxmin::log("Error: Formato de k incorrecto en el JSON.\n", salida);
                }
            }
        }

        // Extracción de tn
        size_t postn = contenido.find("\"tn\"");
        if (postn != string::npos) {
            size_t posDosPuntos = contenido.find(":", postn);
            if (posDosPuntos != string::npos) {
                try {
                    isodata::theta_n = std::stoi(contenido.substr(posDosPuntos + 1));
                } catch (...) { }
            }
        }

        // Extracción de ts
        size_t posts = contenido.find("\"ts\"");
        if (posts != string::npos) {
            size_t posDosPuntos = contenido.find(":", posts);
            if (posDosPuntos != string::npos) {
                try {
                    isodata::theta_s = std::stod(contenido.substr(posDosPuntos + 1));
                } catch (...) { }
            }
        }

        // Extracción de tc
        size_t postc = contenido.find("\"tc\"");
        if (postc != string::npos) {
            size_t posDosPuntos = contenido.find(":", postc);
            if (posDosPuntos != string::npos) {
                try {
                    isodata::theta_c = std::stod(contenido.substr(posDosPuntos + 1));
                } catch (...) { }
            }
        }

        // Extracción de max_pares
        size_t posmaxpares = contenido.find("\"max_pares\"");
        if (posmaxpares != string::npos) {
            size_t posDosPuntos = contenido.find(":", posmaxpares);
            if (posDosPuntos != string::npos) {
                try {
                    isodata::max_pares = std::stoi(contenido.substr(posDosPuntos + 1));
                } catch (...) { }
            }
        }

        // Extracción de iteraciones
        size_t positer = contenido.find("\"iter\"");
        if (positer != string::npos) {
            size_t posDosPuntos = contenido.find(":", positer);
            if (posDosPuntos != string::npos) {
                try {
                    isodata::iteraciones = std::stoi(contenido.substr(posDosPuntos + 1));
                } catch (...) { }
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

            if (linea.empty()) continue;

            // parametros
            if (linea[0] == '@') {
                try {
                    size_t posDospuntos = linea.find(":") + 1;
                    if (posDospuntos == 0) continue; // Si no hay ':', es un comentario puro (como @comentario)

                    string valorStr = linea.substr(posDospuntos);

                    if (linea.find("@umbral:") != string::npos) {
                        maxmin::umbral = std::stod(valorStr);
                        if (salida) maxmin::log("Umbral actualizado: " + std::to_string(maxmin::umbral) + "\n", salida);
                    }
                    else if (linea.find("@k:") != string::npos) {
                        int k_val = std::stoi(valorStr);
                        kmeans::k = k_val;
                        isodata::k_esperado = k_val; // Sincronizamos ambos
                        if (salida) maxmin::log("k actualizado: " + std::to_string(k_val) + "\n", salida);
                    }
                    else if (linea.find("@tn:") != string::npos) {
                        isodata::theta_n = std::stoi(valorStr);
                    }
                    else if (linea.find("@ts:") != string::npos) {
                        isodata::theta_s = std::stod(valorStr);
                    }
                    else if (linea.find("@tc:") != string::npos) {
                        isodata::theta_c = std::stod(valorStr);
                    }
                    else if (linea.find("@max_pares:") != string::npos) {
                        isodata::max_pares = std::stoi(valorStr);
                    }
                    else if (linea.find("@iter:") != string::npos) {
                        isodata::iteraciones = std::stoi(valorStr);
                    }
                } catch (...) {
                    if (salida) maxmin::log("Error: Formato incorrecto en parámetro: " + linea + "\n", salida);
                }
                continue; // Saltamos a la siguiente línea porque esto no es una coordenada
            }

            // --- lectura de cordenadas (X, Y, Z...) ---
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