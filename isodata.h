//
// Created by Miguel on 11/03/26.
//

#ifndef APPVISIONCENIDET_ISODATA_H
#define APPVISIONCENIDET_ISODATA_H
#include <vector>
#include <string>
#include <wx/textctrl.h>

class isodata {
public:
    // Datos y resultados
    static std::vector<std::vector<double>> matrizDatos;
    static std::vector<int> listaIndices;

    // Parámetros clásicos de ISODATA
    static int k_esperado;    // Número deseado de clústeres
    static int theta_n;       // Mínimo de puntos permitidos en un clúster
    static double theta_s;    // Umbral de desviación estándar para dividir (Split)
    static double theta_c;    // Umbral de distancia para fusionar (Merge)
    static int max_pares;     // Máximo de pares de clústeres que se pueden fusionar por iteración
    static int iteraciones;   // Número máximo de iteraciones permitidas

    // Configuración
    static bool verbo;

    static int seed;

    // Función principal
    static void ejecutar(wxTextCtrl* consola = nullptr);

    // Funciones de utilidad (log)
    static void log(const std::string& msg, wxTextCtrl *out);
    static std::string logM(int);
    static std::string p_aString(const std::vector<double> &punto);
    static std::string a2decimal(double number);

private:
    static std::vector<std::vector<double>> centroides;

    // Pasos internos del algoritmo ISODATA
    static void iniciar_centroides();
    static void asignar_puntos(wxTextCtrl* consola);
    static void descartar_clusters_pequenos(wxTextCtrl* consola);
    static void actualizar_centroides();
    static void intentar_dividir(wxTextCtrl* consola);
    static void intentar_fusionar(wxTextCtrl* consola);
    static double calcularDistancia(const std::vector<double>& p1, const std::vector<double>& p2);
};


#endif //APPVISIONCENIDET_ISODATA_H