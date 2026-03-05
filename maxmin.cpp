#include "maxmin.h"
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
vector<vector<double>> maxmin::matrizDatos; //los datos de entrada
vector<int> maxmin::listaIndices;  //las asignaciones de las clases
bool maxmin::verbo = true; //mostrar todas las calculaciones.
int maxmin::seed = 1; //semilla para el random
int num_clases = 0; //cuantas clases existen actualmente.
double maxmin::umbral = 0.5; //parametro del min-max
double maxmin::dist_mayor = 0; //elemento mayor distancia
vector<vector<double>> maxmin::matrizDistancias;  //distancias de cada elemento con respecto al resto -equivale a columnas del excel
double dist_mayor_inicial = 0;
int limite = 0;//iteraciones de creacion de clases, solo para debug/limitar casos de error

//recibe el índice y retorna un string con todos los elementos en formato x, y...
string maxmin::logM(const int pos) {
    string msg = "[";
    for (size_t i = 0; i < matrizDatos[pos].size(); ++i) {
        msg += a2decimal(matrizDatos[pos][i]);
        if (i < matrizDatos[pos].size() - 1) {
            msg += ", ";
        }
    }
    msg += "]";
    return msg;
}
//recibe un string y lo imprime en *out
void maxmin::log(const string& msg,wxTextCtrl *out) {
    if (out) {
        out->AppendText(msg);
        out->Update();
    }
}
//retorna un n al azar, usando la semilla dada
int maxmin::obtenerIndiceAleatorio() {
    if (matrizDatos.empty()) return -1;
    std::mt19937 generador(seed); // Inicializamos el motor con la semilla que capturamos del textbox
    //Aquí podria poner un msgbox para decir que la semilla debe ser <= al número de renglones.
    std::uniform_int_distribution<int> distribucion(0, (int)matrizDatos.size() - 1);//Definimos el rango: de 0 al total de filas menos 1
    return distribucion(generador); //retornamos el número generado
}

//función auxiliar para n-dimensiones, retorna distancia
double maxmin::calcularDistancia(const std::vector<double>& p1, const std::vector<double>& p2) {
    double suma = 0.0;
    // Usamos el tamaño menor para evitar desbordamientos, aunque idealmente deberían ser iguales
    size_t dimensiones = std::min(p1.size(), p2.size());
    
    for (size_t i = 0; i < dimensiones; ++i) {
        double diff = (p1[i]) - (p2[i]);
        suma += diff * diff;
    }
    return std::sqrt(suma);
}

//recibe: elemento, retorna: pos del valor a mayor distancia de este.
int maxmin::obtenerMasLejano(int indiceReferencia, wxTextCtrl *out) {
    if (verbo && out) {
        log("Calculando distancias...\n",out);
    }
    int masLejano = 0;
    double maxDistanciaSq = -1.0;
    
    if (matrizDatos.empty() || indiceReferencia < 0 || indiceReferencia >= (int)matrizDatos.size()) {
        return 0;
    }

    for (int i = 0; i < (int)matrizDatos.size(); ++i) { //recorro la lista
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
int maxmin::obtenerMasCercano(int indiceReferencia,wxTextCtrl *out) {
    int mas_cercano = 0;
    double minDistancia = 999999.0;
    // Verificamos que la matriz no esté vacía y que el índice solicitado sea válido
    if (matrizDatos.empty() || indiceReferencia < 0 || indiceReferencia >= (int)matrizDatos.size()) {return 0;}
    // Asumimos que cada vector interno tiene al menos [0] para X y [1] para Y
    for (int i = 0; i < (int)matrizDatos.size(); ++i) { //itero la lista, ignoro consigo mismo dist==0
        if (i == indiceReferencia) continue;
            // Cálculo de distancia euclides   sqrt(x2 - x1)^2 + (y2 - y1)^2
            double distSq = calcularDistancia(matrizDatos[i], matrizDatos[indiceReferencia]);// Llamada a la función n-dimensional
            //Aquí podemos mutear el log con el boton verboso
        if (verbo && out) {
            string mensaje = "Dist: " + logM(i) + " - " + logM(indiceReferencia) + " = " + a2decimal(distSq) + "\n";
            log(mensaje, out);
        }
            if (distSq < minDistancia) {
                minDistancia = distSq;
                mas_cercano = i;
            }
    }
    return mas_cercano;
}

//formatea el string numérico a 2 decimales
std::string maxmin::a2decimal(std::string text) {
    stringstream ss;
    ss << std::fixed << std::setprecision(2) << text;
    return ss.str();
}
//formatea el string numérico a 2 decimales
std::string maxmin::a2decimal(double number) {
    stringstream ss;
    ss << std::fixed << std::setprecision(2) << number;
    return ss.str();
}

//inicializa parámetros
//realiza la seleccion de las primeras dos clases
//llama a la función max-min para generar las clases n:3+ hasta que se llegue al umbral.
void maxmin::max_min_ini(wxTextCtrl* out) {
    if (out) {
        std::fill(listaIndices.begin(), listaIndices.end(), -1);
        num_clases = 0;
        log("Algoritmo Max-Min\n",out);
        // Primer Centro
        int n = obtenerIndiceAleatorio(); //toma uno al azar, se basa en la semilla en la GUI
        listaIndices[n] = 0;
        if (!matrizDatos.empty()) {
            log("Umbral factor: " + a2decimal(umbral) + "\n", out);
            log("Matriz: " + to_string(matrizDatos.size()) + " filas x "+ std::to_string(matrizDatos[0].size())+ " columnas \n", out);
            log("Grupo 1 en #" + std::to_string(n) + ": " + logM(n) + '\n', out);
            // Segundo Centro
            int lejano = obtenerMasLejano(n, out);  //busco el más lejano del primero
            listaIndices[lejano] = ++num_clases; // Clase '1', la segunda.
            log("Grupo 2 en #" + std::to_string(lejano) + ": " + logM(lejano) + '\n', out);
            // Calcular la distancia inicial entre los 2 puntos
            dist_mayor_inicial = calcularDistancia(matrizDatos[n], matrizDatos[lejano]);
            log("Distancia Inicial (C1-C2): " + a2decimal(dist_mayor_inicial) + "\n", out);
            //inicializo mi memoria
            matrizDistancias.assign(matrizDatos.size(), vector<double>());

            // Llenar distancias hacia Centro 1 (n)
            for(int i=0; i < (int)matrizDatos.size(); ++i) {
                double d = calcularDistancia(matrizDatos[i], matrizDatos[n]);
                matrizDistancias[i].push_back((double)d);
            }
            // Llenar distancias hacia Centro 2 (lejano)
            for(int i=0; i < (int)matrizDatos.size(); ++i) {
                double d = calcularDistancia(matrizDatos[i], matrizDatos[lejano]);
                matrizDistancias[i].push_back((double)d);
            }
            dist_mayor = dist_mayor_inicial;

            // Bucle para encontrar resto de centros
            while (dist_mayor > (umbral * dist_mayor_inicial) && limite < (int)(matrizDatos.size()-2) ) {
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
void maxmin::max_min(wxTextCtrl *out) {
    double maxDeLasMinimas = -1.0;
    int indiceCandidato = -1;

    for (int i = 0; i < (int)matrizDatos.size(); ++i) {//para cada punto que no es un centro
        if (listaIndices[i] == -1) { 
            double distMinimaACentro = *std::min_element(matrizDistancias[i].begin(), matrizDistancias[i].end());//cual es el min?

            if (distMinimaACentro > maxDeLasMinimas) {//cual es el max?
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
            matrizDistancias[i].push_back((double)d);
        }

        dist_mayor = maxDeLasMinimas; 
    } else {
        log("====== Fin de busqueda de centros ====== \n",out);
        log("Max dist. rest. (" + a2decimal(maxDeLasMinimas) + ") no supera umbral (" + a2decimal(umbralReal) + ").\n", out);
        dist_mayor = 0; 
    }
}

//asigna a las clases generadas el resto de elementos
void maxmin::realizarClasificacion(wxTextCtrl *out) {
    if(verbo && out) log("==== Clasificando elementos restantes ====\n", out);

    for (int i = 0; i < (int)matrizDatos.size(); ++i) {
        if (listaIndices[i] == -1) { // Solo clasificamos los que no son centros

            // Encontrar la posición de la distancia mínima en el vector de distancias de este punto.
            // Ese índice equivale al número de la clase/centro.
            // Busco el elemento mas pequeño de la lista
            auto min_it = std::min_element(matrizDistancias[i].begin(), matrizDistancias[i].end());
            int claseAsignada = std::distance(matrizDistancias[i].begin(), min_it);

            listaIndices[i] = claseAsignada;

            if (verbo && out) {
                log("Elemento " + logM(i) + " -> Clase " + to_string(claseAsignada+1) +", D: "+ a2decimal(*min_it) + "\n", out);
            }
        }
    }
}
