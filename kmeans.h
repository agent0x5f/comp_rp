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
    static std::vector<std::vector<double>> matrizDatos;  //dataset de entrada
    static void ejecutar(wxTextCtrl *out);
    static void iniciar_centroides();
    static void asignacion(wxTextCtrl* consola);
    static void recalcula_centroides();
    static std::string p_aString(const std::vector<double>& punto);
    static int k;
    static int seed;
    static std::vector<int> listaIndices;
    static void log(const std::string &msg, wxTextCtrl *out);
    static bool verbo;
private:
    static std::string a2decimal(double number);
    static std::string logM(int pos);
    static double calcularDistancia(const std::vector<double>& p1, const std::vector<double>& p2);
    static int obtenerCercanoNoVisitado(int indiceActual, wxTextCtrl *out);
};

#endif //APPVISIONCENIDET_KMEANS_H