#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

// Definir el pin para el LED
// Esta línea define la macro LED_PIN, que representa el número del pin GPIO para el LED.
// En este caso, se usa PICO_DEFAULT_LED_PIN, que es una constante predefinida que representa
// el pin LED predeterminado en la placa Raspberry Pi Pico.
#define LED_PIN PICO_DEFAULT_LED_PIN

// Enumeración para representar los estados del hilo
typedef enum
{
    THREAD_RUNNING,    // Hilo en ejecución
    THREAD_SUSPENDED,  // Hilo suspendido
    THREAD_TERMINATED, // Hilo terminado
} ThreadState;

// Estructura para almacenar información del hilo
typedef struct
{
    void (*thread_func)(void); // Puntero a la función del hilo
    uint32_t priority;         // Prioridad del hilo
    uint32_t remaining_time;   // Tiempo restante para la ejecución (usado en la programación de hilos)
    uint32_t thread_id;        // ID del hilo
    uint32_t waiting;          // Indicador para indicar si el hilo está esperando
    ThreadState state;         // Estado actual del hilo
} ThreadControlBlock;

// Número de hilos
#define NUM_THREADS 2

// Array de bloques de control de hilos
// Estas líneas definen el número de hilos (NUM_THREADS) y crean un
// array de estructuras ThreadControlBlock para almacenar información sobre cada hilo.
ThreadControlBlock thread_blocks[NUM_THREADS];

// Variable para rastrear el hilo actual
volatile uint32_t current_thread;

// Variable para rastrear el tiempo de ejecución
uint32_t execution_time = 0;

// Estructura que representa un semáforo
typedef struct
{
    uint32_t count;
} Semaphore;

// Instancia de semáforo para controlar el acceso al LED
Semaphore led_semaphore;

// Función para inicializar un semáforo
void semaphore_init(Semaphore *semaphore, uint32_t initial_count)
{
    semaphore->count = initial_count;
}

// Función para adquirir un semáforo
void semaphore_acquire(Semaphore *semaphore)
{
    // Esperar hasta que el semáforo esté disponible
    while (semaphore->count == 0)
    {
        printf("El ID del hilo %d está esperando el semáforo.\n", thread_blocks[current_thread].thread_id);
        yield(); // Liberar la CPU para permitir que otros hilos se ejecuten
    }
    semaphore->count--;
}

// Función para liberar un semáforo
void semaphore_release(Semaphore *semaphore)
{
    // Verificar si hay desbordamiento del contador
    if (semaphore->count < UINT32_MAX)
        semaphore->count++;
}

// Función del hilo 1
void thread_1(void)
{
    while (1)
    {
        // Verificar si el hilo está suspendido
        if (thread_blocks[current_thread].state == THREAD_SUSPENDED)
        {
            printf("El ID del hilo %d está suspendido.\n", thread_blocks[current_thread].thread_id);
            sleep_ms(1000);
        }

        // Verificar si el hilo está terminado
        if (thread_blocks[current_thread].state == THREAD_TERMINATED)
        {
            printf("El ID del hilo %d está terminado.\n", thread_blocks[current_thread].thread_id);
            sleep_ms(1000);
            continue; // Saltar el código restante en el bucle y comenzar de nuevo
        }

        // Verificar si el hilo está en ejecución
        if (thread_blocks[current_thread].state == THREAD_RUNNING)
        {
            // Adquirir el semáforo
            semaphore_acquire(&led_semaphore);
            printf("El ID del hilo %d adquirió el semáforo. Estado: ", thread_blocks[current_thread].thread_id);

            // Imprimir el estado del hilo
            switch (thread_blocks[current_thread].state)
            {
            case THREAD_RUNNING:
                printf("En ejecución\n");
                break;
            case THREAD_SUSPENDED:
                printf("Suspendido\n");
                break;
            case THREAD_TERMINATED:
                printf("Terminado\n");
                break;
            }

            // Alternar el LED
            gpio_put(LED_PIN, 1);
            sleep_ms(300);
            gpio_put(LED_PIN, 0);
            sleep_ms(300);

            // Liberar el semáforo
            semaphore_release(&led_semaphore);
            printf("El ID del hilo %d liberó el semáforo. Estado: ", thread_blocks[current_thread].thread_id);

            // Imprimir el estado del hilo
            switch (thread_blocks[current_thread].state)
            {
            case THREAD_RUNNING:
                printf("En ejecución\n");
                break;
            case THREAD_SUSPENDED:
                printf("Suspendido\n");
                break;
            case THREAD_TERMINATED:
                printf("Terminado\n");
                break;
            }
        }

        // Realizar un cambio de contexto al siguiente hilo
        yield();
    }
}

// Función del hilo 2
void thread_2(void)
{
    while (1)
    {
        // Verificar si el hilo está suspendido
        if (thread_blocks[current_thread].state == THREAD_SUSPENDED)
        {
            printf("El ID del hilo %d está suspendido.\n", thread_blocks[current_thread].thread_id);
            sleep_ms(1000);
            continue; // Saltar el código restante en el bucle y comenzar de nuevo
        }

        // Verificar si el hilo está terminado
        if (thread_blocks[current_thread].state == THREAD_TERMINATED)
        {
            printf("El ID del hilo %d está terminado.\n", thread_blocks[current_thread].thread_id);
            sleep_ms(1000);
            continue; // Saltar el código restante en el bucle y comenzar de nuevo
        }

        // Verificar si el hilo está en ejecución
        if (thread_blocks[current_thread].state == THREAD_RUNNING)
        {
            // Adquirir el semáforo
            semaphore_acquire(&led_semaphore);
            printf("El ID del hilo %d adquirió el semáforo. Estado: ", thread_blocks[current_thread].thread_id);

            // Imprimir el estado del hilo
            switch (thread_blocks[current_thread].state)
            {
            case THREAD_RUNNING:
                printf("En ejecución\n");
                break;
            case THREAD_SUSPENDED:
                printf("Suspendido\n");
                break;
            case THREAD_TERMINATED:
                printf("Terminado\n");
                break;
            }

            // Alternar el LED
            gpio_put(LED_PIN, 1);
            sleep_ms(300);
            gpio_put(LED_PIN, 0);
            sleep_ms(300);

            // Liberar el semáforo
            semaphore_release(&led_semaphore);
            printf("El ID del hilo %d liberó el semáforo. Estado: ", thread_blocks[current_thread].thread_id);

            // Imprimir el estado del hilo
            switch (thread_blocks[current_thread].state)
            {
            case THREAD_RUNNING:
                printf("En ejecución\n");
                break;
            case THREAD_SUSPENDED:
                printf("Suspendido\n");
                break;
            case THREAD_TERMINATED:
                printf("Terminado\n");
                break;
            }
        }

        // Realizar un cambio de contexto al siguiente hilo
        yield();
    }
}

// Manejador de tick del kernel para interrupción del temporizador
// La función es un manejador de interrupciones que se llama periódicamente por una interrupción de temporizador.
// Sirve como un tick del kernel, que es un intervalo de tiempo regular en el que el sistema realiza ciertas tareas o comprobaciones.
// Esto típicamente incluye tareas como actualizar el tiempo del sistema, comprobar eventos sensibles al tiempo o realizar operaciones periódicas.
void tick_handler(struct repeating_timer *t)
{
    // La variable tick representa el recuento de ticks actual o el tiempo del sistema. Se incrementa con cada tick del kernel.
    static uint32_t tick = 0;
    tick++;

    // Iterar a través de todos los hilos
    for (int i = 0; i < NUM_THREADS; i++)
    {
        // Omitir el hilo actual
        if (i != current_thread)
        {
            // Verificar si es hora de restablecer el tiempo restante del hilo
            // Esta condición asegura que el tiempo restante del hilo se restablezca solo cuando
            // el recuento de ticks actual es un múltiplo de la prioridad del hilo.
            // Si la condición es verdadera, significa que se cumple la prioridad del hilo,
            // y es hora de restablecer el tiempo de ejecución restante del hilo.
            if (tick % thread_blocks[i].priority == 0)
                // Esto establece el tiempo de ejecución restante del hilo a 0, efectivamente restableciéndolo.
                // Esto indica que el hilo es elegible para ejecución y puede ser programado por el planificador de hilos.
                thread_blocks[i].remaining_time = 0;
        }
    }
}

// Planificador de hilos del kernel
// La función es responsable de seleccionar el siguiente hilo a ejecutar basado en sus estados y prioridades.
// Asegura que el hilo más apropiado sea elegido para la ejecución.
void scheduler()
{
    // Verificar si el hilo actual está suspendido
    if (thread_blocks[current_thread].state == THREAD_SUSPENDED)
    {
        // Encontrar el siguiente hilo disponible
        uint32_t next_thread = (current_thread + 1) % NUM_THREADS;
        while (thread_blocks[next_thread].state != THREAD_RUNNING)
        {
            // La variable next_thread se inicializa con el índice del siguiente hilo de manera circular ((current_thread + 1) % NUM_THREADS).
            // Esto asegura que el planificador recorra todos los bloques de control de hilos.
            next_thread = (next_thread + 1) % NUM_THREADS;

            // Verificar si todos los hilos están suspendidos o terminados
            if (next_thread == current_thread)
            // Si el bucle se completa sin encontrar un hilo en ejecución, significa que no hay hilos activos disponibles para ejecución.
            {
                printf("No hay hilos activos. Saliendo.\n");
                sleep_ms(1000);
                exit(0);
            }
        }

        // Cambiar al siguiente hilo
        current_thread = next_thread;
    }
    else
    {
        // Encontrar el siguiente hilo disponible basado en la prioridad
        // Similar al bloque anterior, next_thread se inicializa con el índice del siguiente hilo de manera circular.
        // En este caso, el planificador busca un hilo en ejecución con la prioridad mínima entre todos los hilos disponibles.
        uint32_t next_thread = (current_thread + 1) % NUM_THREADS;
        uint32_t min_priority = UINT32_MAX;
        while (thread_blocks[next_thread].state != THREAD_RUNNING || thread_blocks[next_thread].priority >= min_priority)
        {
            // El bucle continúa hasta encontrar un hilo en ejecución con una prioridad menor que min_priority.
            // Si se encuentra un hilo con una prioridad menor, min_priority se actualiza en consecuencia.
            // Si no se encuentra un hilo en ejecución con una prioridad menor, o si no hay hilos en ejecución en absoluto, el programa imprime un mensaje y sale.
            // Una vez que se encuentra un hilo adecuado, current_thread se actualiza y ese hilo se convierte en el siguiente hilo a ejecutar.
            if (thread_blocks[next_thread].state == THREAD_RUNNING && thread_blocks[next_thread].priority < min_priority)
                min_priority = thread_blocks[next_thread].priority;

            next_thread = (next_thread + 1) % NUM_THREADS;

            // Verificar si todos los hilos están suspendidos o terminados
            if (next_thread == current_thread)
            {
                printf("No hay hilos activos. Saliendo.\n");
                sleep_ms(1000);
                exit(0);
            }
        }

        // Cambiar al siguiente hilo
        current_thread = next_thread;
    }
}

void suspend_thread(uint32_t thread_id)
{
    thread_blocks[thread_id].state = THREAD_SUSPENDED;
    printf("Hilo ID %d suspendido.\n", thread_id);
}

void resume_thread(uint32_t thread_id)
{
    thread_blocks[thread_id].state = THREAD_RUNNING;
    printf("Hilo ID %d reanudado.\n", thread_id);
}

void terminate_thread(uint32_t thread_id)
{
    thread_blocks[thread_id].state = THREAD_TERMINATED;
    printf("Hilo ID %d terminado.\n", thread_id);
}

void yield()
{
    // printf("execution_time -----> %d\n", execution_time / 1000);
    uint32_t next_thread = (current_thread + 1) % NUM_THREADS;
    current_thread = next_thread;

    printf("Cambio de contexto: Hilo ID %d | Prioridad %d\n", thread_blocks[current_thread].thread_id, thread_blocks[current_thread].priority);
    thread_blocks[current_thread].remaining_time = thread_blocks[current_thread].priority;

    if (execution_time == 30 * 1000)
    {
        suspend_thread(0);
    }
    else if (execution_time == 60 * 1000)
    {
        resume_thread(0);
    }
    else if (execution_time == 90 * 1000)
    {
        terminate_thread(0);
    }
    execution_time += 1000;
}

// Función para inicializar los hilos
void init_threads()
{
    // Inicializar el hilo 1
    thread_blocks[0].thread_func = thread_1;
    thread_blocks[0].priority = 3;
    thread_blocks[0].remaining_time = 0;
    thread_blocks[0].thread_id = 1;
    thread_blocks[0].waiting = 0;
    thread_blocks[0].state = THREAD_RUNNING;

    // Inicializar el hilo 2
    thread_blocks[1].thread_func = thread_2;
    thread_blocks[1].priority = 2;
    thread_blocks[1].remaining_time = 0;
    thread_blocks[1].thread_id = 2;
    thread_blocks[1].waiting = 0;
    thread_blocks[1].state = THREAD_RUNNING;

    // Establecer el hilo actual al hilo 1
    current_thread = 0;
}

int main()
{
    stdio_init_all();
    // Inicializar el pin del LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Inicializar el semáforo para el acceso al LED
    semaphore_init(&led_semaphore, 1);

    // Inicializar los bloques de control de hilos
    init_threads();

    // Configurar la interrupción del temporizador para el tick del kernel
    struct repeating_timer kernel_timer;
    add_repeating_timer_us(1000, tick_handler, NULL, &kernel_timer);

    while (1)
    {
        // Ejecutar la función del hilo actual
        thread_blocks[current_thread].thread_func();

        // Realizar la programación de hilos
        scheduler();

        // Actualizar el tiempo de ejecución
        execution_time++;

        // Verificar si el tiempo de ejecución ha alcanzado un umbral
        if (execution_time >= 1000)
        {
            printf("Límite de tiempo de ejecución alcanzado. Saliendo.\n");
            sleep_ms(1000);
            exit(0);
        }
    }

    return 0;
}

