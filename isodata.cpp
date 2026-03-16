//
// Created by Miguel on 11/03/26.
//

#include "isodata.h"
#include "kmeans.h"
#include <iomanip>
#include <set>
#include <random>
using namespace std;

// Inicialización de variables estáticas
vector<vector<double>> isodata::matrizDatos;
vector<int> isodata::listaIndices;
vector<vector<double>> isodata::centroides;

int isodata::k_esperado = 3;
int isodata::theta_n = 1;
double isodata::theta_s = 1.0;
double isodata::theta_c = 1.0;
int isodata::max_pares = 1;
int isodata::iteraciones = 10;
bool isodata::verbo = true;
int isodata::seed = 1;

//funcion principal
void isodata::ejecutar(wxTextCtrl* consola) {
    if (matrizDatos.empty()) {
        if (consola) consola->AppendText("Error: Datos vacíos para ISODATA.\n");
        return;
    }

    if (consola) consola->AppendText("--- Iniciando ISODATA ---\n");
    listaIndices.assign(matrizDatos.size(), -1);

    iniciar_centroides(); // Empezamos con un centroide o algunos al azar

    for (int iter = 1; iter <= iteraciones; ++iter) {
        if (consola) consola->AppendText("ISODATA - Iteracion " + std::to_string(iter) + "\n");

        // Asignar puntos a centroides
        asignar_puntos(consola);
        // Descartar clústeres con menos de theta_n puntos
        descartar_clusters_pequenos(consola);
        // Recalcular centroides tras el descarte
        actualizar_centroides();

        // Lógica de Decisión (Dividir o Fusionar)
        // Def. Maravall, si es iteración par o tenemos muchos clústeres, intentamos fusionar.
        // Si es impar o tenemos pocos clústeres, intentamos dividir.
        if (centroides.size() <= (size_t)(k_esperado / 2)) {
            intentar_dividir(consola);
        } else if (iter % 2 == 0 || centroides.size() >= (size_t)(k_esperado * 2)) {
            intentar_fusionar(consola);
        } else {
            intentar_dividir(consola);
        }

        // Aquí podría evaluar una condición de parada temprana si no hubo cambios, para optimizar, revisar luego
    }

    if (consola) consola->AppendText("--- ISODATA Finalizado ---\n");
    if (consola) consola->AppendText("Centroides encontrados: " + std::to_string(centroides.size()) + "\n");


    // Pasamos los datos a kmeans y usamos la funcion sobrecargada
    kmeans::matrizDatos = isodata::matrizDatos;
    if (consola) consola->AppendText("\n--- Pasando semillas a k-Means ---\n");
    //llamada a kmeans con los centroides resultantes
    kmeans mi_kmeans;
    mi_kmeans.ejecutar(centroides, consola);
}

void isodata::iniciar_centroides() {
    centroides.clear();
    if (matrizDatos.empty()) return;

    // Nos aseguramos de no pedir más centroides iniciales que la cantidad de puntos disponibles
    int num_iniciales = std::min(k_esperado, (int)matrizDatos.size());
    // Si k_esperado es 0 o menor (por algún error de configuración), forzamos al menos 1
    if (num_iniciales <= 0) num_iniciales = 1;

    std::set<int> indices_usados;

    // Usamos una semilla fija temporalmente (puedes parametrizarla después si lo deseas)
    std::mt19937 generador(seed);
    std::uniform_int_distribution<int> distribucion(0, (int)matrizDatos.size() - 1);

    while (indices_usados.size() < (size_t)num_iniciales) {
        int elegido = distribucion(generador);

        // Si el índice no estaba repetido, lo agregamos como centroide inicial
        if (indices_usados.insert(elegido).second) {
            centroides.push_back(matrizDatos[elegido]);
        }
    }
}

void isodata::asignar_puntos(wxTextCtrl* consola) {
    if (centroides.empty() || matrizDatos.empty()) return;

    for (size_t i = 0; i < matrizDatos.size(); ++i) {
        int mas_cercano = 0;
        double minDistancia = calcularDistancia(matrizDatos[i], centroides[0]);

        std::string log_linea = "";
        if (verbo) {
            log_linea = "P" + std::to_string(i) + " " + p_aString(matrizDatos[i]) + "->dC0: " + a2decimal(minDistancia);
        }

        // Comparamos contra el resto de los centroides dinámicos
        for (size_t j = 1; j < centroides.size(); ++j) {
            double d = calcularDistancia(matrizDatos[i], centroides[j]);

            if (verbo) {
                log_linea += ", dC" + std::to_string(j) + ":" + a2decimal(d);
            }

            if (d < minDistancia) {
                minDistancia = d;
                mas_cercano = j; // Actualizamos el ganador temporal
            }
        }

        if (verbo && consola) {
            log_linea += " ->Grupo " + std::to_string(mas_cercano + 1) + "\n";
            consola->AppendText(log_linea);
        }

        // Guardamos el índice del centroide ganador para este punto
        listaIndices[i] = mas_cercano;
    }
}

void isodata::descartar_clusters_pequenos(wxTextCtrl* consola) {
    if (centroides.empty()) return;

    // Contamos cuántos puntos tiene cada clúster actualmente
    std::vector conteo(centroides.size(), 0);
    for (int cluster_idx : listaIndices) {
        if (cluster_idx >= 0 && cluster_idx < (int)conteo.size()) {
            conteo[cluster_idx]++;
        }
    }

    std::vector<std::vector<double>> nuevos_centroides;
    std::vector mapa_indices(centroides.size(), -1);
    int nuevo_idx = 0;
    bool descarto_algo = false;

    // Cuáles sobreviven al umbral theta_n
    for (size_t c = 0; c < centroides.size(); ++c) {
        if (conteo[c] < theta_n) {
            // No alcanza el mínimo, se descarta
            if (verbo && consola) {
                consola->AppendText("Descartando clúster " + std::to_string(c) +
                                    " por tener solo " + std::to_string(conteo[c]) +
                                    " puntos (theta_n=" + std::to_string(theta_n) + ").\n");
            }
            descarto_algo = true;
        } else {
            // Sobrevive, lo guardamos y registramos su nuevo índice
            nuevos_centroides.push_back(centroides[c]);
            mapa_indices[c] = nuevo_idx;
            nuevo_idx++;
        }
    }

    // Si theta_n es muy alto y todos van fuera.
    if (nuevos_centroides.empty() && !centroides.empty()) {
         if (consola) consola->AppendText("Advertencia: Todos los clústeres iban a ser descartados. Conservando el primero.\n");
         nuevos_centroides.push_back(centroides[0]);
         mapa_indices[0] = 0;
    }

    // Si hubo cambios, actualizamos el estado global
    if (descarto_algo) {
        centroides = nuevos_centroides;

        // Actualizamos la lista de índices para que apunte a los nuevos IDs (o -1 si quedaron huérfanos)
        for (size_t i = 0; i < listaIndices.size(); ++i) {
            int old_id = listaIndices[i];
            if (old_id != -1) {
                listaIndices[i] = mapa_indices[old_id];
            }
        }
    }
}

void isodata::actualizar_centroides() {
    if (centroides.empty() || matrizDatos.empty()) return;

    int num_clusters = centroides.size();
    int dimensiones = matrizDatos[0].size();

    // contenedores para sumar las coordenadas y contar los puntos
    std::vector sumas(num_clusters, std::vector(dimensiones, 0.0));
    std::vector conteos(num_clusters, 0);

    // Recorremos todos los datos y sumamos sus coordenadas al clúster que pertenecen
    for (size_t i = 0; i < matrizDatos.size(); ++i) {
        int cluster_idx = listaIndices[i];

        // Verificamos que el punto tenga un clúster válido asignado
        if (cluster_idx >= 0 && cluster_idx < num_clusters) {
            for (int d = 0; d < dimensiones; ++d) {
                sumas[cluster_idx][d] += matrizDatos[i][d];
            }
            conteos[cluster_idx]++;
        }
    }

    // Calculamos el nuevo promedio (centro) para cada clúster
    for (int c = 0; c < num_clusters; ++c) {
        // Nos aseguramos de no dividir entre cero (aunque descartar_clusters_pequenos ya debería haber filtrado los vacíos)
        if (conteos[c] > 0) {
            for (int d = 0; d < dimensiones; ++d) {
                centroides[c][d] = sumas[c][d] / conteos[c];
            }
        }
    }
}

void isodata::intentar_dividir(wxTextCtrl* consola) {
    if (centroides.empty() || matrizDatos.empty()) return;

    int dimensiones = matrizDatos[0].size();
    int num_clusters = centroides.size();

    // Calcular la distancia promedio interna de cada clúster y la global
    std::vector<double> dist_promedio_cluster(num_clusters, 0.0);
    std::vector<int> conteos(num_clusters, 0);
    double dist_promedio_global = 0.0;
    int puntos_totales = 0;

    for (size_t i = 0; i < matrizDatos.size(); ++i) {
        int c = listaIndices[i];
        if (c >= 0 && c < num_clusters) {
            double d = calcularDistancia(matrizDatos[i], centroides[c]);
            dist_promedio_cluster[c] += d;
            dist_promedio_global += d;
            conteos[c]++;
            puntos_totales++;
        }
    }

    if (puntos_totales > 0) dist_promedio_global /= puntos_totales;
    for (int c = 0; c < num_clusters; ++c) {
        if (conteos[c] > 0) dist_promedio_cluster[c] /= conteos[c];
    }

    // Calcular vector de desviación estándar para cada clúster
    std::vector<std::vector<double>> desviaciones(num_clusters, std::vector<double>(dimensiones, 0.0));

    for (size_t i = 0; i < matrizDatos.size(); ++i) {
        int c = listaIndices[i];
        if (c >= 0 && c < num_clusters) {
            for (int d = 0; d < dimensiones; ++d) {
                double diff = matrizDatos[i][d] - centroides[c][d]; // Suma de diferencias al cuadrado
                desviaciones[c][d] += diff * diff;
            }
        }
    }

    for (int c = 0; c < num_clusters; ++c) {
        if (conteos[c] > 0) {
            for (int d = 0; d < dimensiones; ++d) {
                desviaciones[c][d] = std::sqrt(desviaciones[c][d] / conteos[c]);
            }
        }
    }

    // Evaluar y ejecutar las divisiones
    std::vector<std::vector<double>> nuevos_centroides;
    bool hubo_division = false;

    for (int c = 0; c < num_clusters; ++c) {
        // Encontrar la dimensión con mayor desviación estándar en este clúster
        double max_desv = 0.0;
        int dim_max = 0;
        for (int d = 0; d < dimensiones; ++d) {
            if (desviaciones[c][d] > max_desv) {
                max_desv = desviaciones[c][d];
                dim_max = d;
            }
        }

        // Reglas para hacer una división:
        // 1. El clúster está más disperso que el promedio global (dist_promedio_cluster > dist_promedio_global)
        // 2. Al dividirlo, ambos nuevos clústeres tendrían oportunidad de sobrevivir (conteos > 2 * theta_n)
        bool es_muy_disperso = (dist_promedio_cluster[c] > dist_promedio_global) && (conteos[c] > 2 * theta_n);

        // Excepción: Si tenemos muy pocos clústeres respecto a k_esperado, somos más permisivos
        bool faltan_clusters = (centroides.size() <= (size_t)(k_esperado / 2));

        if (max_desv > theta_s && (es_muy_disperso || faltan_clusters)) {
            if (verbo && consola) {
                consola->AppendText("Dividiendo clúster " + std::to_string(c) +
                                    " (Desv: " + a2decimal(max_desv) + " en dim " + std::to_string(dim_max) + ").\n");
            }

            // Creamos los gemelos
            std::vector<double> c1 = centroides[c];
            std::vector<double> c2 = centroides[c];

            // Los separamos en la dimensión problemática
            double desplazamiento = max_desv * 0.5; // Factor de separación
            c1[dim_max] += desplazamiento;
            c2[dim_max] -= desplazamiento;

            nuevos_centroides.push_back(c1);
            nuevos_centroides.push_back(c2);
            hubo_division = true;
        } else {
            // El clúster se portó bien, lo pasamos intacto a la siguiente generación
            nuevos_centroides.push_back(centroides[c]);
        }
    }

    // Si dividimos algo, actualizamos la lista global
    if (hubo_division) {
        centroides = nuevos_centroides;
        // Al alterar el número de centroides, los índices actuales pierden sentido.
        // Los reseteamos; se volverán a calcular en el próximo "asignar_puntos()".
        listaIndices.assign(matrizDatos.size(), -1);
    }
}

void isodata::intentar_fusionar(wxTextCtrl* consola) {
    // Si hay menos de 2 clústeres, no hay nada que fusionar
    if (centroides.size() < 2) return;

    int num_clusters = centroides.size();
    int dimensiones = matrizDatos[0].size();

    // Contar puntos por clúster para el promedio ponderado
    std::vector<int> conteos(num_clusters, 0);
    for (int c : listaIndices) {
        if (c >= 0 && c < num_clusters) {
            conteos[c]++;
        }
    }

    // Estructura auxiliar para guardar las distancias entre pares
    struct ParClusters {
        double distancia;
        int c1;
        int c2;
    };
    std::vector<ParClusters> pares_candidatos;

    // Calcular distancias entre todos los pares únicos
    for (int i = 0; i < num_clusters - 1; ++i) {
        for (int j = i + 1; j < num_clusters; ++j) {
            double dist = calcularDistancia(centroides[i], centroides[j]);
            if (dist < theta_c) {
                pares_candidatos.push_back({dist, i, j});
            }
        }
    }

    // Si nadie cumplió la condición de cercanía, terminamos
    if (pares_candidatos.empty()) return;

    // Ordenar los pares de menor a mayor distancia (priorizamos fusionar los más cercanos)
    std::sort(pares_candidatos.begin(), pares_candidatos.end(),
              [](const ParClusters& a, const ParClusters& b) {
                  return a.distancia < b.distancia;
              });

    // Ejecutar las fusiones
    std::vector<bool> fusionado(num_clusters, false);
    std::vector<std::vector<double>> nuevos_centroides;
    int fusiones_realizadas = 0;

    for (const auto& par : pares_candidatos) {
        // Respetamos el límite de fusiones por iteración
        if (fusiones_realizadas >= max_pares) break;

        // Un clúster solo puede participar en una fusión por iteración
        if (fusionado[par.c1] || fusionado[par.c2]) continue;

        if (verbo && consola) {
            consola->AppendText("Fusionando clústeres " + std::to_string(par.c1) +
                                " y " + std::to_string(par.c2) +
                                " (Dist: " + a2decimal(par.distancia) + " < " + a2decimal(theta_c) + ").\n");
        }

        // Crear el nuevo centroide (promedio ponderado)
        std::vector<double> nuevo_centro(dimensiones, 0.0);
        int total_puntos = conteos[par.c1] + conteos[par.c2];

        if (total_puntos == 0) { // revisar, de nuevo...
            for (int d = 0; d < dimensiones; ++d) {
                nuevo_centro[d] = (centroides[par.c1][d] + centroides[par.c2][d]) / 2.0;
            }
        } else {
            for (int d = 0; d < dimensiones; ++d) {
                nuevo_centro[d] = (centroides[par.c1][d] * conteos[par.c1] +
                                   centroides[par.c2][d] * conteos[par.c2]) / total_puntos;
            }
        }

        nuevos_centroides.push_back(nuevo_centro);
        fusionado[par.c1] = true;
        fusionado[par.c2] = true;
        fusiones_realizadas++;
    }

    // Agregar a la nueva lista los clústeres que no participaron en ninguna fusión
    for (int i = 0; i < num_clusters; ++i) {
        if (!fusionado[i]) {
            nuevos_centroides.push_back(centroides[i]);
        }
    }

    // Actualizar el estado global
    if (fusiones_realizadas > 0) {
        centroides = nuevos_centroides;
        // Los índices viejos ya no sirven porque el número y orden de centroides cambió
        listaIndices.assign(matrizDatos.size(), -1);
    }
}

std::string isodata::p_aString(const std::vector<double> &punto) {
    string msg = "[";
    for (size_t i = 0; i < punto.size(); ++i) {
        msg += a2decimal(punto[i]);
        if (i < punto.size() - 1) msg += ", ";
    }
    msg += "]";
    return msg;
}


string isodata::a2decimal(double number) {
    stringstream ss;
    ss << std::fixed << std::setprecision(2) << number;
    return ss.str();
}

string isodata::logM(const int pos) {
    string msg = "[";
    for (size_t i = 0; i < matrizDatos[pos].size(); ++i) {
        msg += a2decimal(matrizDatos[pos][i]);
        if (i < matrizDatos[pos].size() - 1) msg += ", ";
    }
    msg += "]";
    return msg;
}

void isodata::log(const string& msg, wxTextCtrl *out) {
    if (out) {
        out->AppendText(msg);
        out->Update();
    }
}
//calcula la distancia entre dos puntos de n dimensiones
double isodata::calcularDistancia(const std::vector<double>& p1, const std::vector<double>& p2) {
    double suma = 0.0;
    const size_t dimensiones = std::min(p1.size(), p2.size());
    for (size_t i = 0; i < dimensiones; ++i) {
        const double diff = (p1[i]) - (p2[i]);
        suma += diff * diff;
    }
    return std::sqrt(suma);
}