#ifndef WRITE_JSON_H
#define WRITE_JSON_H
#include <cjson/cJSON.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../include/metrics.h"
#include "../include/expose_metrics.h"

/**
 * 
 * @brief Estructura para almacenar las métricas que se desean monitorear.
 * Seleccionamos el estado de cada una con 1 o 0.
 */
typedef struct
{
    int cpu;              /**< Estado del monitoreo de CPU: 1 habilitado, 0 deshabilitado. */
    int memory;           /**< Estado del monitoreo de memoria: 1 habilitado, 0 deshabilitado. */
    int disk;             /**< Estado del monitoreo de disco: 1 habilitado, 0 deshabilitado. */
    int network;          /**< Estado del monitoreo de red: 1 habilitado, 0 deshabilitado. */
    int processes;        /**< Estado del monitoreo del número de procesos: 1 habilitado, 0 deshabilitado. */
    int context_switches; /**< Estado del monitoreo de cambios de contexto: 1 habilitado, 0 deshabilitado. */
} MetricsConfig;

/**
 * @brief Lee la configuración de métricas desde un archivo JSON.
 * Esta función abre un archivo JSON especificado, lo lee completamente, y parsea el contenido
 * para extraer la configuración de métricas
 * @param config_file Ruta del archivo JSON de configuración.
 */
MetricsConfig read_metrics_config(const char* config_file);

/**
 * @brief Crea una cadena JSON con las métricas seleccionadas en la configuración.
 *
 * A partir de una estructura `MetricsConfig`, esta función genera un objeto JSON
 * SOLO con las metricas habilitadas.
 */
char* create_metrics_json(MetricsConfig config);

/**
 * @brief Envía las métricas al monitor.
 *
 * Esta función lee la configuración de métricas desde un archivo JSON, genera un JSON
 * con las métricas seleccionadas y envía los datos a través de un FIFO al monitor.
 */
void send_metrics_to_monitor();

#endif // WRITE_JSON_H
