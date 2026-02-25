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
    string msg = "["+std::to_string(matrizDatos[pos][0])+" ,"+ std::to_string(matrizDatos[pos][1])+"]";
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

//recibe: elemento, retorna: valor a mayor distancia de este.
int Algoritmo::obtenerMasLejano(int indiceReferencia,wxTextCtrl *out) {
    if (verbo && out) {
        log("Calculando distancias...\n",out);
    }
    int masLejano = 0;
    double maxDistanciaSq = -1.0;
    // Verificamos que la matriz no esté vacía y que el índice solicitado sea válido
    if (matrizDatos.empty() || indiceReferencia < 0 || indiceReferencia >= (int)matrizDatos.size()) {
        return 0;
    }
    // Asumimos que cada vector interno tiene al menos [0] para X y [1] para Y
    int xRef = matrizDatos[indiceReferencia][0];
    int yRef = matrizDatos[indiceReferencia][1];
    // Recorrer la matriz para comparar distancias
    for (int i = 0; i < (int)matrizDatos.size(); ++i) {
        if (i == indiceReferencia) continue; //no comparo consigo mismo

        if (matrizDatos[i].size() >= 2) {   //x,y ...
            int xActual = matrizDatos[i][0];
            int yActual = matrizDatos[i][1];

            // Cálculo de distancia euclides   sqrt(x2 - x1)^2 + (y2 - y1)^2
            double distSq = sqrt(pow(xActual - xRef, 2) + pow(yActual - yRef, 2));
            //Aquí podemos mutear el log con el boton verboso
            if (verbo && out) {
                std::stringstream ss;// Creamos un flujo de string para formatear
                ss << std::fixed << std::setprecision(2) << distSq;
                // Ahora construimos el mensaje usando ss.str()
                string mensaje = "Dist: [" + std::to_string(xActual) + ", "+std::to_string((yActual))+"] - [" + std::to_string(xRef)+", "+std::to_string((yRef)) + "] = " + ss.str() + "\n";
                log(mensaje, out);
            }
            if (distSq > maxDistanciaSq) {
                maxDistanciaSq = distSq;
                masLejano = i;
                dist_mayor = distSq;
            }
        }
    }
    return masLejano;
}

//recibe un elemento, retorna el elemento más cercano a este, usa euclides
int Algoritmo::obtenerMasCercano(int indiceReferencia,wxTextCtrl *out) {
    int mas_cercano = 0;
    double minDistanciaSq = 999999.0;
    // Verificamos que la matriz no esté vacía y que el índice solicitado sea válido
    if (matrizDatos.empty() || indiceReferencia < 0 || indiceReferencia >= (int)matrizDatos.size()) {
        return 0;
    }
    // Asumimos que cada vector interno tiene al menos [0] para X y [1] para Y
    int xRef = matrizDatos[indiceReferencia][0];
    int yRef = matrizDatos[indiceReferencia][1];
    // Recorrer la matriz para comparar distancias
    for (int i = 0; i < (int)matrizDatos.size(); ++i) {
        if (i == indiceReferencia) continue;

        if (matrizDatos[i].size() >= 2) {
            int xActual = matrizDatos[i][0];
            int yActual = matrizDatos[i][1];

            // Cálculo de distancia euclides   sqrt(x2 - x1)^2 + (y2 - y1)^2
            double distSq = sqrt(pow(xActual - xRef, 2) + pow(yActual - yRef, 2));
            //Aquí podemos mutear el log con el boton verboso
            if (verbo && out) {
                string mensaje = "Dist: [" + std::to_string(xActual) + ", "+std::to_string((yActual))+"] - [" + std::to_string(xRef)+", "+std::to_string((yRef)) + "] = " +  a2decimal(distSq) + "\n";
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

std::string Algoritmo::a2decimal(std::string text) {
    stringstream ss;
    ss << std::fixed << std::setprecision(2) << text;
    return ss.str();
}

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

            // Calcular la distancia inicial para el umbral
            dist_mayor_inicial = sqrt(pow(matrizDatos[n][0] - matrizDatos[lejano][0], 2) +
                                      pow(matrizDatos[n][1] - matrizDatos[lejano][1], 2));

            stringstream ss2;
            ss2 << std::fixed << std::setprecision(2) << dist_mayor_inicial;
            log("Distancia Inicial (C1-C2): " + ss2.str() + "\n", out);

            //  Inicializar matrizDistancias con los 2 primeros centros
            matrizDistancias.assign(matrizDatos.size(), vector<float>());

            // Llenar distancias hacia Centro 1 (n)
            for(int i=0; i < (int)matrizDatos.size(); ++i) {
                double d = sqrt(pow(matrizDatos[i][0] - matrizDatos[n][0], 2) + pow(matrizDatos[i][1] - matrizDatos[n][1], 2));
                matrizDistancias[i].push_back((float)d);
            }
            // Llenar distancias hacia Centro 2 (lejano)
            for(int i=0; i < (int)matrizDatos.size(); ++i) {
                double d = sqrt(pow(matrizDatos[i][0] - matrizDatos[lejano][0], 2) + pow(matrizDatos[i][1] - matrizDatos[lejano][1], 2));
                matrizDistancias[i].push_back((float)d);
            }

            // Configuramos dist_mayor para que entre al bucle
            dist_mayor = dist_mayor_inicial;

            // 5. Bucle para encontrar resto de centros
            while (dist_mayor > (Algoritmo::umbral * dist_mayor_inicial) && limite < 100) {
                max_min(out);
                ++limite;
            }
            realizarClasificacion(out);
        }
    }
}

//procede al resto de la generación del algoritmo ya tenemos las primeras dos clases.
//recordemos que listaIndices contiene las clases
//y que matrizDistancias las distancias entre los elementos
void Algoritmo::max_min(wxTextCtrl *out) {
    double maxDeLasMinimas = -1.0;
    int indiceCandidato = -1;

    // A. Buscar el siguiente candidato (MAX-MIN)
    // Nota: Ya no actualizamos distancias al inicio, eso se hace al FINAL de haber encontrado un centro válido.

    for (int i = 0; i < (int)matrizDatos.size(); ++i) {
        if (listaIndices[i] == -1) { // Si no es un centro
            // MIN: Distancia al centro más cercano
            float distMinimaACentro = *std::min_element(matrizDistancias[i].begin(), matrizDistancias[i].end());

            // MAX: El punto más lejano de todos los cercanos
            if (distMinimaACentro > maxDeLasMinimas) {
                maxDeLasMinimas = distMinimaACentro;
                indiceCandidato = i;
            }
        }
    }

    // Validar contra el umbral REAL
    // Si la distancia es > (Factor * DistanciaInicial)
    double umbralReal = Algoritmo::umbral * dist_mayor_inicial;

    if (indiceCandidato != -1 && maxDeLasMinimas > umbralReal) {
        num_clases++;
        listaIndices[indiceCandidato] = num_clases;

        log("Grupo " + to_string(num_clases+1) + " en #" + std::to_string(indiceCandidato) + ": " + logM(indiceCandidato) + "\n", out);

        // Ahora que aceptamos el centro, actualizamos la matriz de distancias
        for (int i = 0; i < (int)matrizDatos.size(); ++i) {
            double d = sqrt(pow(matrizDatos[i][0] - matrizDatos[indiceCandidato][0], 2) +
                            pow(matrizDatos[i][1] - matrizDatos[indiceCandidato][1], 2));
            matrizDistancias[i].push_back((float)d);
        }

        dist_mayor = maxDeLasMinimas; // Actualizamos para el while
    } else {
        log("Fin de búsqueda de centros. Max distancia restante (" + a2decimal(maxDeLasMinimas) + ") no supera umbral (" + a2decimal(umbralReal) + ").\n", out);
        dist_mayor = 0; // Rompemos el bucle while
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
                    double d = sqrt(pow(matrizDatos[i][0] - matrizDatos[j][0], 2) +
                                    pow(matrizDatos[i][1] - matrizDatos[j][1], 2));
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