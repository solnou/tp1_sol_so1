#include "../include/expose_metrics.h"
#include "metrics.c"
/** Mutex para sincronización de hilos */
pthread_mutex_t lock;

/** Métrica de Prometheus para el uso de CPU */
static prom_gauge_t* cpu_usage_metric;

/** Métrica de Prometheus para el uso de memoria */
static prom_gauge_t* memory_usage_metric;

/** Métrica de Prometheus para del disco con operaciones IO */
static prom_gauge_t* IOdisk_usage_metric;

/** Métrica de Prometheus para la tasa de transferencia de la red */
static prom_gauge_t* network_transfer_rate_metric;

/** Métrica de Prometheus para la cantidad de procesos en ejecución */
static prom_gauge_t* processes_metric;

/** Métrica de Prometheus para la cantidad de cambios de contexto */
static prom_gauge_t* context_switches_metric;

void update_cpu_gauge()
{

    double usage = get_cpu_usage();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(cpu_usage_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el uso de CPU\n");
    }
}

void update_memory_gauge()
{
    double usage = get_memory_usage();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(memory_usage_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el uso de memoria\n");
    }
}

void update_IOdisk()
{
    double usage = get_IOdisk();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(IOdisk_usage_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el tiempo de uso de IO del disco\n");
    }
}

void update_network_transferrate()
{
    double transfer = get_network_transfer_rate();
    if (transfer >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(network_transfer_rate_metric, transfer, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener la tasa de transferencia de la red\n");
    }
}

void update_processes_counter()
{
    double processes = get_processcounter();
    if (processes >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(processes_metric, processes, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener la cantidad de procesos en ejecución\n");
    }
}

void update_context_switchs()
{
    double context_switches = get_context_switchs();
    if (context_switches >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(context_switches_metric, context_switches, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener la cantidad de cambios de contexto\n");
    }
}

void* expose_metrics(void* arg)
{
    (void)arg; // Argumento no utilizado

    // Aseguramos que el manejador HTTP esté adjunto al registro por defecto
    promhttp_set_active_collector_registry(NULL);

    // Iniciamos el servidor HTTP en el puerto 8000
    struct MHD_Daemon* daemon = promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY, 8000, NULL, NULL);
    if (daemon == NULL)
    {
        fprintf(stderr, "Error al iniciar el servidor HTTP\n");
        return NULL;
    }

    // Mantenemos el servidor en ejecución
    while (1)
    {
        sleep(1);
    }

    // Nunca debería llegar aquí
    MHD_stop_daemon(daemon);
    return NULL;
}

void init_metrics()
{
    // Inicializamos el mutex
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        fprintf(stderr, "Error al inicializar el mutex\n");
    }

    // Inicializamos el registro de coleccionistas de Prometheus
    if (prom_collector_registry_default_init() != 0)
    {
        fprintf(stderr, "Error al inicializar el registro de Prometheus\n");
    }

    // Creamos la métrica para el uso de CPU
    cpu_usage_metric = prom_gauge_new("cpu_usage_percentage", "Porcentaje de uso de CPU", 0, NULL);
    if (cpu_usage_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de uso de CPU\n");
    }

    // Creamos la métrica para el uso de memoria
    memory_usage_metric = prom_gauge_new("memory_usage_percentage", "Porcentaje de uso de memoria", 0, NULL);
    if (memory_usage_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de uso de memoria\n");
    }

    // Creamos la metrica para el tiempo de I/O ocupado
    IOdisk_usage_metric = prom_gauge_new("IOdisk_usage_percentage", "Porcentaje de tiempo de I/O ocupado", 0, NULL);
    if (IOdisk_usage_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de tiempo de I/O ocupado\n");
    }

    // Creamos la métrica para la tasa de transferencia de la red
    network_transfer_rate_metric = prom_gauge_new("network_transfer_rate", "Tasa de transferencia de la red", 0, NULL);
    if (network_transfer_rate_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de tasa de transferencia de la red\n");
    }

    // Creamos la métrica para la cantidad de procesos en ejecución
    processes_metric = prom_gauge_new("processes_running", "Cantidad de procesos en ejecucion", 0, NULL);
    if (processes_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de cantidad de procesos en ejecución\n");
    }

    // Creamos la métrica para la cantidad de cambios de contexto
    context_switches_metric = prom_gauge_new("context_switches", "Cantidad de cambios de contexto", 0, NULL);
    if (context_switches_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de cantidad de cambios de contexto\n");
    }

    // General
    if (prom_collector_registry_must_register_metric(memory_usage_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar las métricas - memoria\n");
    }
    if (prom_collector_registry_must_register_metric(cpu_usage_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar las métricas - cpu\n");
    }
    if (prom_collector_registry_must_register_metric(IOdisk_usage_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar las métricas - IOdisk\n");
    }
    if (prom_collector_registry_must_register_metric(network_transfer_rate_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de tasa de transferencia de la red\n");
    }
    if (prom_collector_registry_must_register_metric(processes_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de cantidad de procesos en ejecucion\n");
    }
    if (prom_collector_registry_must_register_metric(context_switches_metric) == NULL)
    {
        fprintf(stderr, "Error al registrar la métrica de cantidad de cambios de contexto\n");
    }
}

void destroy_mutex()
{
    pthread_mutex_destroy(&lock);
}
