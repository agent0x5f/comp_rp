//
// Created by Mat on 16/03/2026.
//

#ifndef APPVISIONCENIDET_DBSCAN_H
#define APPVISIONCENIDET_DBSCAN_H

#include <vector>
#include <string>
#include <wx/textctrl.h>

class dbscan {
public:
    static std::vector<std::vector<double>> matrizDatos;
    static std::vector<int> listaIndices;
    static bool verbo;

    // Parámetros de DBSCAN
    static double epsilon;
    static int minPts;
    static int num_clases;

    static void ejecutar(wxTextCtrl* consola = nullptr);

    // Funciones auxiliares
    static std::string a2decimal(double number);
    static std::string logM(const int pos);
    static void log(const std::string& msg, wxTextCtrl *out);
    static double calcularDistancia(const std::vector<double>& p1, const std::vector<double>& p2);

private:
    static std::vector<int> buscarVecinos(int indicePunto, wxTextCtrl* consola);
    static void expandirCluster(int indicePunto, std::vector<int>& vecinos, int claseActual, wxTextCtrl* consola);
};

#endif //APPVISIONCENIDET_DBSCAN_H