//
// Created by Mat on 05/03/2026.
//

#ifndef APPVISIONCENIDET_KMEANS_H
#define APPVISIONCENIDET_KMEANS_H

#include <vector>
#include <iostream>
#include <fstream>
#include  <wx/wx.h>

class kmeans {
public:
    static std::vector<std::vector<double>> matrizDatos;
    static void ejecutar(wxTextCtrl *out);
    static double umbral;
    static int num_clases;
    static int k;
    static int seed;
    static std::vector<int> listaIndices;
    static void log(const std::string &msg, wxTextCtrl *out);
private:
    static bool verbo;
    static std::string a2decimal(double number);
    static std::string logM(int pos);
    static int obtenerIndiceAleatorio();
    static double calcularDistancia(const std::vector<double>& p1, const std::vector<double>& p2);
    static int obtenerCercanoNoVisitado(int indiceActual, wxTextCtrl *out);
};

#endif //APPVISIONCENIDET_KMEANS_H