#include "algoritmo.h"
#include <wx/wx.h>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
using namespace std;

string datos; //guarda el dataset de entrada
int num_param = 0; //argc
vector<int>params; //args
vector<vector<int>> Algoritmo::matrizDatos; //los datos de entrada
vector<int> Algoritmo::listaIndices;  //las asignaciones de las clases
bool Algoritmo::verbo = true; //mostrar todas las calculaciones.
int Algoritmo::seed = 1; //semilla para el random
int num_clases = 0; //cuantas clases existen actualmente.
double Algoritmo::umbral = 0.5; //parametro del min-max
double Algoritmo::dist_mayor = 0; //elemento mayor distancia
vector<vector<float>> Algoritmo::matrizDistancias;  //distancias de cada elemento con respecto al resto -equivale a columnas del excel
double dist_mayor_inicial = 0;
int limite = 0;//iteraciones de creacion de clases, solo para debug/limitar casos de error

//recibe el índice y retorna un string con todos los elementos en formato x, y...
string Algoritmo::logM(const int pos) {
    string msg = "[";
    for (size_t i = 0; i < matrizDatos[pos].size(); ++i) {
        msg += std::to_string(matrizDatos[pos][i]);
        if (i < matrizDatos[pos].size() - 1) {
            msg += ", ";
        }
    }
    msg += "]";
    return msg;
}
//recibe un string y lo imprime en *out
void Algoritmo::log(const string& msg,wxTextCtrl *out) {
    if (out) {
        out->AppendText(msg);
        out->Update();
    }
}
//retorna un n al azar, usando la semilla dada
int Algoritmo::obtenerIndiceAleatorio() {
    if (matrizDatos.empty()) return -1;
    std::mt19937 generador(seed); // Inicializamos el motor con la semilla que capturamos del textbox
    //Aquí podria poner un msgbox para decir que la semilla debe ser <= al número de renglones.
    std::uniform_int_distribution<int> distribucion(0, (int)matrizDatos.size() - 1);//Definimos el rango: de 0 al total de filas menos 1
    return distribucion(generador); //retornamos el número generado
}

//función auxiliar para n-dimensiones, retorna distancia
double Algoritmo::calcularDistancia(const std::vector<int>& p1, const std::vector<int>& p2) {
    double suma = 0.0;
    // Usamos el tamaño menor para evitar desbordamientos, aunque idealmente deberían ser iguales
    size_t dimensiones = std::min(p1.size(), p2.size());
    
    for (size_t i = 0; i < dimensiones; ++i) {
        double diff = static_cast<double>(p1[i]) - static_cast<double>(p2[i]);
        suma += diff * diff;
    }
    return std::sqrt(suma);
}

//recibe: elemento, retorna: valor a mayor distancia de este.
int Algoritmo::obtenerMasLejano(int indiceReferencia, wxTextCtrl *out) {
    if (verbo && out) {
        log("Calculando distancias...\n",out);
    }
    int masLejano = 0;
    double maxDistanciaSq = -1.0;
    
    if (matrizDatos.empty() || indiceReferencia < 0 || indiceReferencia >= (int)matrizDatos.size()) {
        return 0;
    }

    for (int i = 0; i < (int)matrizDatos.size(); ++i) {
        if (i == indiceReferencia) continue; // no comparo consigo mismo

        // Llamada a la función n-dimensional
        double distSq = calcularDistancia(matrizDatos[i], matrizDatos[indiceReferencia]);

        if (verbo && out) { //muestro en log
            string mensaje = "Dist: " + logM(i) + " - " + logM(indiceReferencia) + " = " + a2decimal(distSq) + "\n";
            log(mensaje, out);
        }
        if (distSq > maxDistanciaSq) { //voy revisando si es el mayor
            maxDistanciaSq = distSq;
            masLejano = i;
            dist_mayor = distSq;
        }
    }
    return masLejano;
}
//recibe un elemento, retorna el elemento más cercano a este, usa euclides
//TODO: soporte para n dimensiones
int Algoritmo::obtenerMasCercano(int indiceReferencia,wxTextCtrl *out) {
    int mas_cercano = 0;
    double minDistanciaSq = 999999.0;
    // Verificamos que la matriz no esté vacía y que el índice solicitado sea válido
    if (matrizDatos.empty() || indiceReferencia < 0 || indiceReferencia >= (int)matrizDatos.size()) {
        return 0;
    }
    // Asumimos que cada vector interno tiene al menos [0] para X y [1] para Y
    //int xRef = matrizDatos[indiceReferencia][0];
    //int yRef = matrizDatos[indiceReferencia][1];
    // Recorrer la matriz para comparar distancias
    for (int i = 0; i < (int)matrizDatos.size(); ++i) {
        if (i == indiceReferencia) continue;

        if (matrizDatos[i].size() >= 2) {
            //int xActual = matrizDatos[i][0];
            //int yActual = matrizDatos[i][1];
            // Cálculo de distancia euclides   sqrt(x2 - x1)^2 + (y2 - y1)^2
            //double distSq = sqrt(pow(xActual - xRef, 2) + pow(yActual - yRef, 2));
            // Llamada a la función n-dimensional
            double distSq = calcularDistancia(matrizDatos[i], matrizDatos[indiceReferencia]);

            //Aquí podemos mutear el log con el boton verboso
            if (verbo && out) {
                string mensaje = "Dist: [" + std::to_string(matrizDatos[i][0]) + ", " +
                                 std::to_string((matrizDatos[i][1])) + "] - [" +
                                 std::to_string(matrizDatos[indiceReferencia][0]) + ", " + std::to_string(
                                     (matrizDatos[indiceReferencia][0])) + "] = " + a2decimal(distSq) + "\n";
                log(mensaje, out);
            }
            if (distSq < minDistanciaSq) {
                minDistanciaSq = distSq;
                mas_cercano = i;
            }
        }
    }
    return mas_cercano;
}

//formatea el string numérico a 2 decimales
std::string Algoritmo::a2decimal(std::string text) {
    stringstream ss;
    ss << std::fixed << std::setprecision(2) << text;
    return ss.str();
}
//formatea el string numérico a 2 decimales
std::string Algoritmo::a2decimal(double number) {
    stringstream ss;
    ss << std::fixed << std::setprecision(2) << number;
    return ss.str();
}

//inicializa parámetros
//realiza la seleccion de las primeras dos clases
//llama a la función max-min para generar las clases n:3+ hasta que se llegue al umbral.
void Algoritmo::max_min_ini(wxTextCtrl* out) {
    if (out) {
        std::fill(listaIndices.begin(), listaIndices.end(), -1);
        num_clases = 0;

        // Primer Centro
        int n = obtenerIndiceAleatorio();
        listaIndices[n] = 0;

        if (!matrizDatos.empty()) {
            stringstream ss;
            ss << std::fixed << std::setprecision(2) << umbral;
            log("Umbral factor: " + ss.str() + '\n', out);
            log("Matriz: " + to_string(matrizDatos.size()) + " filas.\n", out);
            log("Grupo 1 en #" + std::to_string(n) + ": " + logM(n) + '\n', out);

            // Segundo Centro
            int lejano = obtenerMasLejano(n, out);
            listaIndices[lejano] = ++num_clases; // Clase 1
            log("Grupo 2 en #" + std::to_string(lejano) + ": " + logM(lejano) + '\n', out);

            // Calcular la distancia inicial n-dimensional
            dist_mayor_inicial = calcularDistancia(matrizDatos[n], matrizDatos[lejano]);
            log("Distancia Inicial (C1-C2): " + a2decimal(dist_mayor_inicial) + "\n", out);

            matrizDistancias.assign(matrizDatos.size(), vector<float>());

            // Llenar distancias hacia Centro 1 (n)
            for(int i=0; i < (int)matrizDatos.size(); ++i) {
                double d = calcularDistancia(matrizDatos[i], matrizDatos[n]);
                matrizDistancias[i].push_back((float)d);
            }
            // Llenar distancias hacia Centro 2 (lejano)
            for(int i=0; i < (int)matrizDatos.size(); ++i) {
                double d = calcularDistancia(matrizDatos[i], matrizDatos[lejano]);
                matrizDistancias[i].push_back((float)d);
            }

            dist_mayor = dist_mayor_inicial;

            // Bucle para encontrar resto de centros
            while (dist_mayor > (Algoritmo::umbral * dist_mayor_inicial) && limite < 100) {
                max_min(out);
                ++limite;
            }
            realizarClasificacion(out);
        }
    }
}

//Procede al resto de la generación del algoritmo ya tenemos las primeras dos clases.
//Recordemos que listaIndices contiene las clases
//y que matrizDistancias las distancias entre los elementos
void Algoritmo::max_min(wxTextCtrl *out) {
    double maxDeLasMinimas = -1.0;
    int indiceCandidato = -1;

    for (int i = 0; i < (int)matrizDatos.size(); ++i) {
        if (listaIndices[i] == -1) { 
            float distMinimaACentro = *std::min_element(matrizDistancias[i].begin(), matrizDistancias[i].end());

            if (distMinimaACentro > maxDeLasMinimas) {
                maxDeLasMinimas = distMinimaACentro;
                indiceCandidato = i;
            }
        }
    }

    double umbralReal = umbral * dist_mayor_inicial;

    if (indiceCandidato != -1 && maxDeLasMinimas > umbralReal) {
        num_clases++;
        listaIndices[indiceCandidato] = num_clases;
        log("Max dist. rest. (" + a2decimal(maxDeLasMinimas) + ") supera umbral (" + a2decimal(umbralReal) + ").\n", out);
        log("Grupo " + to_string(num_clases+1) + " en #" + std::to_string(indiceCandidato) + ": " + logM(indiceCandidato) + "\n", out);

        // Actualizamos la matriz de distancias con el nuevo centro
        for (int i = 0; i < (int)matrizDatos.size(); ++i) {
            double d = calcularDistancia(matrizDatos[i], matrizDatos[indiceCandidato]);
            matrizDistancias[i].push_back((float)d);
        }

        dist_mayor = maxDeLasMinimas; 
    } else {
        log("====== Fin de búsqueda de centros ====== \n",out);
        log("Max dist. rest. (" + a2decimal(maxDeLasMinimas) + ") no supera umbral (" + a2decimal(umbralReal) + ").\n", out);
        dist_mayor = 0; 
    }
}

//asigna a las clases generadas el resto de elementos
void Algoritmo::realizarClasificacion(wxTextCtrl *out) {
    if(verbo && out) log("==== Clasificando elementos restantes ====\n", out);
    for (int i = 0; i < (int)matrizDatos.size(); ++i) {
        if (listaIndices[i] == -1) { // Solo clasificamos los que no son centros
            float distMinima = 999999.0;
            int claseAsignada = -1;
            // Buscar centro más cercano
            for (int j = 0; j < (int)matrizDatos.size(); ++j) {
                if (listaIndices[j] != -1) { // Comparamos contra centros
                    // Reemplazo de la fórmula por la función n-dimensional
                    double d = calcularDistancia(matrizDatos[i], matrizDatos[j]);
                    if (d < distMinima) {
                        distMinima = d;
                        claseAsignada = listaIndices[j];
                    }
                }
            }
            listaIndices[i] = claseAsignada;
            if (verbo && out) {
                log("Elemento " + logM(i) + " -> Clase " + to_string(claseAsignada+1) + "\n", out);
            }
        }
    }
}
