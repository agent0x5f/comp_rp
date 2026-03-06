//
// Created by Miguel on 02/03/26.
//

#ifndef APPVISIONCENIDET_CHAINMAP_H
#define APPVISIONCENIDET_CHAINMAP_H
#include <vector>
#include <iostream>
#include <fstream>
#include  <wx/wx.h>

class chainmap {
public:
    static int seed;
    static std::vector<std::vector<double>> matrizDatos;
    static void ejecutar(wxTextCtrl *out);
    static double umbral;
    static int num_clases;
    static std::vector<int> listaIndices;
    static bool verbo;
private:
    static std::string a2decimal(double number);
    static std::string logM(int pos);
    static void log(const std::string &msg, wxTextCtrl *out);
    static int obtenerIndiceAleatorio();
    static double calcularDistancia(const std::vector<double> &p1, const std::vector<double> &p2);
    static int obtenerCercanoNoVisitado(int indiceActual, wxTextCtrl *out);

};

#endif //APPVISIONCENIDET_CHAINMAP_H