#include "../include/metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

double get_memory_usage()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long total_mem = 0, free_mem = 0;

    // Abrir el archivo /proc/meminfo
    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/meminfo");
        return -1.0;
    }

    // Leer los valores de memoria total y disponible
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "MemTotal: %llu kB", &total_mem) == 1)
        {
            continue; // MemTotal encontrado
        }
        if (sscanf(buffer, "MemAvailable: %llu kB", &free_mem) == 1)
        {
            break; // MemAvailable encontrado, podemos dejar de leer
        }
    }

    fclose(fp);

    // Verificar si se encontraron ambos valores
    if (total_mem == 0 || free_mem == 0)
    {
        fprintf(stderr, "Error al leer la información de memoria desde /proc/meminfo\n");
        return -1.0;
    }

    // Calcular el porcentaje de uso de memoria
    double used_mem = total_mem - free_mem;
    double mem_usage_percent = (used_mem / total_mem) * 100.0;

    return mem_usage_percent;
}

double get_cpu_usage()
{
    static unsigned long long prev_user = 0, prev_nice = 0, prev_system = 0, prev_idle = 0, prev_iowait = 0,
                              prev_irq = 0, prev_softirq = 0, prev_steal = 0;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    unsigned long long totald, idled;
    double cpu_usage_percent;

    // Abrir el archivo /proc/stat
    FILE* fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/stat");
        return -1.0;
    }

    char buffer[BUFFER_SIZE * 4];
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        perror("Error al leer /proc/stat");
        fclose(fp);
        return -1.0;
    }
    fclose(fp);

    // Analizar los valores de tiempo de CPU
    int ret = sscanf(buffer, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait,
                     &irq, &softirq, &steal);
    if (ret < 8)
    {
        fprintf(stderr, "Error al parsear /proc/stat\n");
        return -1.0;
    }

    // Calcular las diferencias entre las lecturas actuales y anteriores
    unsigned long long prev_idle_total = prev_idle + prev_iowait;
    unsigned long long idle_total = idle + iowait;

    unsigned long long prev_non_idle = prev_user + prev_nice + prev_system + prev_irq + prev_softirq + prev_steal;
    unsigned long long non_idle = user + nice + system + irq + softirq + steal;

    unsigned long long prev_total = prev_idle_total + prev_non_idle;
    unsigned long long total = idle_total + non_idle;

    totald = total - prev_total;
    idled = idle_total - prev_idle_total;

    if (totald == 0)
    {
        fprintf(stderr, "Totald es cero, no se puede calcular el uso de CPU!\n");
        return -1.0;
    }

    // Calcular el porcentaje de uso de CPU
    cpu_usage_percent = ((double)(totald - idled) / totald) * 100.0;

    // Actualizar los valores anteriores para la siguiente lectura
    prev_user = user;
    prev_nice = nice;
    prev_system = system;
    prev_idle = idle;
    prev_iowait = iowait;
    prev_irq = irq;
    prev_softirq = softirq;
    prev_steal = steal;

    return cpu_usage_percent;
}

double get_IOdisk()
{
    FILE* fp = NULL;
    char buffer[BUFFER_SIZE];
    double io_busy_porcent = 0;
    unsigned long long io_busy_time = 0, uptime_ms = 0;
    char device_name[32];
    static unsigned long long prev_io_busy_time = 0;
    static unsigned long long prev_uptime_ms = 0;

    // Abrir y leer /proc/diskstats
    fp = fopen("/proc/diskstats", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/diskstats");
        return -1.0;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        unsigned long long device_busy_time;

        // Buscar el dispositivo "nvme0n1" y extraer el tiempo ocupado en I/O (campo 13)
        if (sscanf(buffer, "%hhu %llu %*s %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u", device_name, &device_busy_time) ==
            2)
        {
            if (strcmp(device_name, "nvme0n1") == 0)
            {
                io_busy_time = device_busy_time;
                break; // Detenemos la búsqueda una vez que encontramos el dispositivo
            }
        }
    }

    // Para calcular el valor porcentual del uso de disco, necesitamos el tiempo de actividad del sistema
    fp = fopen("/proc/uptime", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/uptime");
        return -1.0;
    }

    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        perror("Error al leer /proc/uptime");
        fclose(fp);
        return -1.0;
    }
    fclose(fp); // Cerrar el archivo después de leer

    // Obtener el valor de uptime_seconds
    double uptime_seconds;
    if (sscanf(buffer, "%lf", &uptime_seconds) != 1)
    {
        fprintf(stderr, "Error al leer el tiempo de actividad desde /proc/uptime\n");
        return -1.0;
    }

    // Conversion necesaria
    uptime_ms = (unsigned long long)(uptime_seconds * 1000);
    // Calcular las diferencias entre las lecturas actuales y anteriores
    unsigned long long io_busy_time_diff = io_busy_time - prev_io_busy_time;
    unsigned long long uptime_ms_diff = uptime_ms - prev_uptime_ms;
    if (uptime_ms_diff == 0)
    {
        fprintf(stderr, "Valor invalido de uptime\n");
        return -1.0;
    }
    // Obtenemos el porcentaje del tiempo que el disco nvme0n1 ha estado ocupado en I/O respecto al tiempo total
    // transcurrido
    io_busy_porcent = ((double)io_busy_time_diff / uptime_ms_diff) * 100;

    // Actualizar los valores anteriores para la siguiente lectura
    prev_io_busy_time = io_busy_time;
    prev_uptime_ms = uptime_ms;

    return io_busy_porcent;
}

double get_network_transfer_rate()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long rx_bytes = 0, tx_bytes = 0;
    static unsigned long long prev_rx_bytes = 0, prev_tx_bytes = 0;
    static struct timeval prev_time = {0, 0};
    struct timeval current_time;
    double transfer_rate = 0.0;

    // Obtener el tiempo actual
    gettimeofday(&current_time, NULL);

    // Abrir el archivo /proc/net/dev
    fp = fopen("/proc/net/dev", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/net/dev");
        return -1.0;
    }

    // Leer los valores de bytes recibidos y enviados
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        // Leer los bytes recibidos y enviados
        if (sscanf(buffer, "%*s %llu %*s %*s %*s %*s %*s %*s %*s %llu", &rx_bytes, &tx_bytes) == 2)
        {
            break; // Solo necesitamos la primera línea con datos relevantes
        }
    }

    fclose(fp);

    // Calcular la diferencia de tiempo en segundos
    double time_diff = (current_time.tv_sec - prev_time.tv_sec) + (current_time.tv_usec - prev_time.tv_usec) / 1e6;

    // Calcular la tasa de transferencia efectiva
    if (time_diff > 0)
    {
        unsigned long long rx_diff = rx_bytes - prev_rx_bytes;
        unsigned long long tx_diff = tx_bytes - prev_tx_bytes;
        transfer_rate = (double)(rx_diff + tx_diff) / time_diff;
    }

    // Actualizar los valores anteriores
    prev_rx_bytes = rx_bytes;
    prev_tx_bytes = tx_bytes;
    prev_time = current_time;

    return transfer_rate;
}

int get_processcounter()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    int running_processes = 0;

    // Abrir el archivo /proc/stat
    fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/stat");
        return -1.0;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {

        // Buscar la línea que contiene procs_running
        if (sscanf(buffer, "procs_running %d", &running_processes) == 1)
        {
            break; // Procesos running encontrados, podemos dejar de leer
        }
    }

    fclose(fp);

    return running_processes;
}

int get_context_switchs()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    int context_switches = 0;

    // Abrir el archivo /proc/stat
    fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/stat");
        return -1.0;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {

        // Buscar la línea que contiene ctxt
        if (sscanf(buffer, "ctxt %u", &context_switches) == 1)
        {
            break; // Cambios de contexto, podemos dejar de leer
        }
    }

    fclose(fp);

    return context_switches;
}
