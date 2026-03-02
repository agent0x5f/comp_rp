//
// Created by Miguel on 02/03/26.
//

#ifndef APPVISIONCENIDET_CHAINMAP_H
#define APPVISIONCENIDET_CHAINMAP_H
#include <vector>


class chainmap {
    static std::vector<std::vector<double>> matrizDatos;
    static std::vector<int> listaIndices;
    static bool verbo;
    static int seed;
    static double umbral;
    static double dist_mayor;
    static std::vector<std::vector<float>> matrizDistancias;
};

#endif //APPVISIONCENIDET_CHAINMAP_H