//
// Created by Miguel on 02/03/26.
//

#include "chainmap.h"
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
vector<vector<double>> chainmap::matrizDatos; //los datos de entrada
vector<int> chainmap::listaIndices;  //las asignaciones de las clases
bool chainmap::verbo = true; //mostrar todas las calculaciones.
int chainmap::seed = 1; //semilla para el random
int num_clases = 0; //cuantas clases existen actualmente.
double chainmap::umbral = 0.5; //parametro del chainmap
double chainmap::dist_mayor = 0; //elemento mayor distancia
vector<vector<float>> chainmap::matrizDistancias;  //distancias de cada elemento con respecto al resto -equivale a columnas del excel
double dist_mayor_inicial = 0;
int limite = 0;//iteraciones de creacion de clases, solo para debug/limitar casos de error