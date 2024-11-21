/**
 * @file metrics.h
 * @brief Funciones para obtener el uso de CPU y memoria desde el sistema de archivos /proc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Tamaño del buffer utilizado para leer datos.
 */
#define BUFFER_SIZE 256

/**
 * @brief Obtiene el porcentaje de uso de memoria desde /proc/meminfo.
 *
 * Lee los valores de memoria total y disponible desde /proc/meminfo y calcula
 * el porcentaje de uso de memoria.
 *
 * @return Uso de memoria como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_memory_usage();

/**
 * @brief Obtiene el porcentaje de uso de CPU desde /proc/stat.
 *
 * Lee los tiempos de CPU desde /proc/stat y calcula el porcentaje de uso de CPU
 * en un intervalo de tiempo.
 *
 * @return Uso de CPU como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_cpu_usage();

/**
 * @brief Obtenemos el porcentaje del tiempo que el disco nvme0n1 ha estado ocupado en I/O respecto al tiempo total
 * transcurrido
 *
 * Lee el campo 13 desde /proc/diskstats para saber el tiempo que ha tardado realizando operaciones IO y lo compara para
 * hacer un analisis temporal, con respecto al tiempoa activo que obtiene de uptime.
 *
 * @return  Uso de disco como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_IOdisk();

/**
 * @brief Calcula la tasa de transferencia efectiva de la red.
 *
 * Lee los bytes recibidos y enviados desde /proc/net/dev y calcula la tasa de transferencia efectiva
 * en bytes por segundo.
 *
 * @return Tasa de transferencia efectiva en bytes por segundo, o -1.0 en caso de error.
 */
double get_network_transferrate();

/**
 * @brief Obtiene la cantidad de procesos en ejecución.
 *
 * Revisa en el archivo /proc/stats la cantidad de procesos en ejecución.
 *
 * @return Cantidad de procesos en ejecución, o -1 en caso de error.
 */
int get_processcounter();

/**
 * @brief Obtiene la cantidad de cambios de contexto.
 *
 * Revisa en el archivo /proc/stats la cantidad de cambios de contexto.
 *
 * @return Cantidad de cambios de contexto, o -1 en caso de error.
 */
int get_context_switchs();
