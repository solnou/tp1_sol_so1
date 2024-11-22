#include "../include/write_json.h"
#include <sys/stat.h> // Para usar mkfifo
#include <unistd.h>   // Para usar access
#include <fcntl.h>    // Para usar open
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

MetricsConfig read_metrics_config(const char* config_file)
{
    MetricsConfig config = {0, 0, 0, 0, 0, 0};
    FILE* file = fopen(config_file, "r");
    if (!file)
    {
        fprintf(stderr, "Error al abrir el archivo de configuración: %s\n", config_file);
        return config;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* data = malloc(length + 1);
    fread(data, 1, length, file);
    fclose(file);
    data[length] = '\0';

    cJSON* json = cJSON_Parse(data);
    if (json)
    {
        // printf("Archivo JSON parseado con éxito.\n");
        cJSON* metrics = cJSON_GetObjectItem(json, "metrics");
        if (metrics)
        {
            config.cpu = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "cpu"));
            config.memory = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "memory"));
            config.disk = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "disk"));
            config.network = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "network"));
            config.processes = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "processes"));
            config.context_switches = cJSON_IsTrue(cJSON_GetObjectItem(metrics, "context_switches"));
        }
        else
        {
            printf("Objeto 'metrics' no encontrado en JSON.\n");
        }
        cJSON_Delete(json);
    }
    else
    {
        printf("Error al parsear el archivo JSON.\n");
    }

    free(data);
    return config;
}

char* create_metrics_json(MetricsConfig config)
{
    // Crear un objeto cJSON para las métricas
    cJSON* json = cJSON_CreateObject();

    if (config.cpu)
    {
        double cpu_usage = get_cpu_usage();
        if (cpu_usage < 0) {
            fprintf(stderr, "Valor invalido de uptime\n");
        }
        cJSON_AddNumberToObject(json, "cpu_usage", cpu_usage);
    }

    if (config.memory)
    {
        double memory_usage = get_memory_usage();
        cJSON_AddNumberToObject(json, "memory_usage", memory_usage);
    }

    if (config.disk)
    {
        double disk_usage = get_IOdisk();
        cJSON_AddNumberToObject(json, "disk_usage", disk_usage);
    }

    if (config.network)
    {
        double network_usage = get_network_transfer_rate();
        cJSON_AddNumberToObject(json, "network_usage", network_usage);
    }

    if (config.processes)
    {
        int process_count = get_processcounter();
        cJSON_AddNumberToObject(json, "process_count", process_count);
    }

    if (config.context_switches)
    {
        int ctxt_usage = get_context_switchs();
        cJSON_AddNumberToObject(json, "context_switches", ctxt_usage);
    }

    // Convertir el objeto cJSON a una cadena
    char* json_string = cJSON_Print(json);
    if (json_string != NULL)
    {
        printf("JSON de métricas creado con éxito:\n%s\n", json_string);
    }
    cJSON_Delete(json);

    return json_string;
}

void send_metrics_to_monitor() {
    // Crear el FIFO si no existe
    const char* fifo_path = "/tmp/monitor_fifo";
    if (access(fifo_path, F_OK) == -1) {
        if (mkfifo(fifo_path, 0666) == -1) {
            perror("Error al crear el FIFO");
            return;
        } else {
            printf("FIFO creado con éxito: %s\n", fifo_path);
        }
    } else {
        printf("FIFO ya existe: %s\n", fifo_path);
    }

    // Verificar permisos del FIFO
    if (access(fifo_path, W_OK) == -1) {
        perror("No se tienen permisos de escritura en el FIFO");
        return;
    }

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        // Construir la ruta absoluta al archivo de configuración
        char config_file_path[1024];
        snprintf(config_file_path, sizeof(config_file_path), "%s/config.json", "/home/sol/so-i-24-chp2-solnou");

        // Leer la configuración directamente en esta función
        MetricsConfig config = read_metrics_config(config_file_path);

        char* json_string = create_metrics_json(config);

        if (json_string != NULL)
        {
            // Abrir el FIFO en modo escritura
            int fifo_fd = open(fifo_path, O_WRONLY);
            if (fifo_fd != -1)
            {
                // Escribir los datos en el FIFO y cerrar el descriptor de archivo
                ssize_t bytes_written = write(fifo_fd, json_string, strlen(json_string));
                if (bytes_written == -1)
                {
                    perror("Error al escribir en el pipe");
                }
                close(fifo_fd);
            }
            else
            {
                perror("Error al abrir el FIFO");
            }
            free(json_string);
        }
    }
    else
    {
        perror("Error al obtener el directorio actual");
    }
}
