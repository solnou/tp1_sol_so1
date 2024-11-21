/**
 * @file main.c
 * @brief Entry point of the system
 */

// #include "expose_metrics.h"
#include "../include/expose_metrics.h"
#include "../include/metrics.h"
#include "expose_metrics.c"
#include <stdbool.h>

/**
 * @brief Tiempo de espera en segundos entre actualizaciones de métricas.
 */
#define SLEEP_TIME 1

/**
 * @brief Función principal del programa.
 *
 * @param argc Número de argumentos de la línea de comandos.
 * @param argv Array de argumentos de la línea de comandos.
 * @return int Código de salida del programa.
 */
int main(int argc, char* argv[])
{
    init_metrics();
    // Creamos un hilo para exponer las métricas vía HTTP
    pthread_t tid;
    if (pthread_create(&tid, NULL, expose_metrics, NULL) != 0)
    {
        fprintf(stderr, "Error al crear el hilo del servidor HTTP\n");
        return EXIT_FAILURE;
    }

    // Bucle principal para actualizar las métricas cada segundo
    while (true)
    {
        update_cpu_gauge();
        update_memory_gauge();
        update_IOdisk();
        update_network_transferrate();
        update_processes_counter();
        update_context_switchs();
        sleep(SLEEP_TIME);
    }

    return EXIT_SUCCESS;
}
