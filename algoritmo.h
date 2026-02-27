#ifndef ALGORITMO_H
#define ALGORITMO_H

#include <string>
#include <vector>
class wxTextCtrl;

class Algoritmo {
public:
    static std::vector<std::vector<double>> matrizDatos;
    static std::vector<int>listaIndices;
    static int seed;
    static bool verbo;
    static double umbral;
    static double dist_mayor;
    static std::vector<std::vector<float>> matrizDistancias;

    static void max_min_ini(wxTextCtrl *out);
    static void realizarClasificacion(wxTextCtrl *out);
    static std::string logM(int pos);
    static void log(const std::string& msg,wxTextCtrl *out);
    static int obtenerIndiceAleatorio();
    static int obtenerMasLejano(int indiceReferencia, wxTextCtrl *out);
    static void max_min(wxTextCtrl *out);
    static int obtenerMasCercano(int indiceReferencia,wxTextCtrl *out);

    static std::string a2decimal(std::string text);
    static std::string a2decimal(double number);

    static double calcularDistancia(const std::vector<double>& p1, const std::vector<double>& p2);
};

#endif
