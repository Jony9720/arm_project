#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

// Define the pin for the LED
// This line defines the LED_PIN macro, which represents the GPIO pin number for the LED.
// In this case, PICO_DEFAULT_LED_PIN is used, which is a predefined constant representing
// the default LED pin on the Raspberry Pi Pico board.
#define LED_PIN PICO_DEFAULT_LED_PIN

// Enum to represent thread states
typedef enum
{
    THREAD_RUNNING,
    THREAD_SUSPENDED,
    THREAD_TERMINATED,
} ThreadState;

// Structure to store thread information
typedef struct
{
    void (*thread_func)(void); // Function pointer to the thread function
    uint32_t priority;         // Priority of the thread
    uint32_t remaining_time;   // Remaining time for execution (used in thread scheduling)
    uint32_t thread_id;        // ID of the thread
    uint32_t waiting;          // Flag to indicate if the thread is waiting
    ThreadState state;         // Current state of the thread
} ThreadControlBlock;

// Number of threads
#define NUM_THREADS 2

// Array of thread control blocks
// These lines define the number of threads (NUM_THREADS) and create an
// array of ThreadControlBlock structures to store information about each thread.
ThreadControlBlock thread_blocks[NUM_THREADS];

// Variable to track the current thread
volatile uint32_t current_thread;

// Variable to track execution time
uint32_t execution_time = 0;

// Structure representing a semaphore
typedef struct
{
    uint32_t count;
} Semaphore;

// Semaphore instance for controlling LED access
Semaphore led_semaphore;

// Function to initialize a semaphore
void semaphore_init(Semaphore *semaphore, uint32_t initial_count)
{
    semaphore->count = initial_count;
}

// Function to acquire a semaphore
void semaphore_acquire(Semaphore *semaphore)
{
    // Spin until the semaphore becomes available
    while (semaphore->count == 0)
    {
        printf("Thread ID %d is waiting for the semaphore.\n", thread_blocks[current_thread].thread_id);
        yield(); // Release the CPU to allow other threads to execute
    }
    semaphore->count--;
}

// Function to release a semaphore
void semaphore_release(Semaphore *semaphore)
{
    // Check for count overflow
    if (semaphore->count < UINT32_MAX)
        semaphore->count++;
}

// Thread 1 function
void thread_1(void)
{
    while (1)
    {
        // Check if the thread is suspended
        if (thread_blocks[current_thread].state == THREAD_SUSPENDED)
        {
            printf("Thread ID %d is suspended.\n", thread_blocks[current_thread].thread_id);
            sleep_ms(1000);
        }

        // Check if the thread is terminated
        if (thread_blocks[current_thread].state == THREAD_TERMINATED)
        {
            printf("Thread ID %d is terminated.\n", thread_blocks[current_thread].thread_id);
            sleep_ms(1000);
            continue; // Skip the remaining code in the loop and start over
        }

        // Check if the thread is running
        if (thread_blocks[current_thread].state == THREAD_RUNNING)
        {
            // Acquire the semaphore
            semaphore_acquire(&led_semaphore);
            printf("Thread ID %d acquired the semaphore. State: ", thread_blocks[current_thread].thread_id);

            // Print the state of the thread
            switch (thread_blocks[current_thread].state)
            {
            case THREAD_RUNNING:
                printf("Running\n");
                break;
            case THREAD_SUSPENDED:
                printf("Suspended\n");
                break;
            case THREAD_TERMINATED:
                printf("Terminated\n");
                break;
            }

            // Toggle the LED
            gpio_put(LED_PIN, 1);
            sleep_ms(300);
            gpio_put(LED_PIN, 0);
            sleep_ms(300);

            // Release the semaphore
            semaphore_release(&led_semaphore);
            printf("Thread ID %d released the semaphore. State: ", thread_blocks[current_thread].thread_id);

            // Print the state of the thread
            switch (thread_blocks[current_thread].state)
            {
            case THREAD_RUNNING:
                printf("Running\n");
                break;
            case THREAD_SUSPENDED:
                printf("Suspended\n");
                break;
            case THREAD_TERMINATED:
                printf("Terminated\n");
                break;
            }
        }

        // Perform a context switch to the next thread
        yield();
    }
}

// Thread 2 function
void thread_2(void)
{
    while (1)
    {
        // Check if the thread is suspended
        if (thread_blocks[current_thread].state == THREAD_SUSPENDED)
        {
            printf("Thread ID %d is suspended.\n", thread_blocks[current_thread].thread_id);
            sleep_ms(1000);
            continue; // Skip the remaining code in the loop and start over
        }

        // Check if the thread is terminated
        if (thread_blocks[current_thread].state == THREAD_TERMINATED)
        {
            printf("Thread ID %d is terminated.\n", thread_blocks[current_thread].thread_id);
            sleep_ms(1000);
            continue; // Skip the remaining code in the loop and start over
        }

        // Check if the thread is running
        if (thread_blocks[current_thread].state == THREAD_RUNNING)
        {
            // Acquire the semaphore
            semaphore_acquire(&led_semaphore);
            printf("Thread ID %d acquired the semaphore. State: ", thread_blocks[current_thread].thread_id);

            // Print the state of the thread
            switch (thread_blocks[current_thread].state)
            {
            case THREAD_RUNNING:
                printf("Running\n");
                break;
            case THREAD_SUSPENDED:
                printf("Suspended\n");
                break;
            case THREAD_TERMINATED:
                printf("Terminated\n");
                break;
            }

            // Toggle the LED
            gpio_put(LED_PIN, 1);
            sleep_ms(300);
            gpio_put(LED_PIN, 0);
            sleep_ms(300);

            // Release the semaphore
            semaphore_release(&led_semaphore);
            printf("Thread ID %d released the semaphore. State: ", thread_blocks[current_thread].thread_id);

            // Print the state of the thread
            switch (thread_blocks[current_thread].state)
            {
            case THREAD_RUNNING:
                printf("Running\n");
                break;
            case THREAD_SUSPENDED:
                printf("Suspended\n");
                break;
            case THREAD_TERMINATED:
                printf("Terminated\n");
                break;
            }
        }

        // Perform a context switch to the next thread
        yield();
    }
}

// Kernel tick handler for timer interrupt
// Function is an interrupt handler that is called periodically by a timer interrupt.
// It serves as a kernel tick, which is a regular time interval at which the system performs certain tasks or checks.
// This typically includes tasks such as updating system time, checking for time-sensitive events, or performing periodic operations.

void tick_handler(struct repeating_timer *t)
{
    // The variable tick represents the current tick count or system time. It is incremented with each kernel tick.
    static uint32_t tick = 0;
    tick++;

    // Iterate through all threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        // Skip the current thread
        if (i != current_thread)
        {
            // Check if it's time to reset the remaining time of the thread
            // This condition ensures that the remaining time of the thread is reset only when
            // the current tick count is a multiple of the thread's priority.
            // If the condition is true, it means that the thread's priority is met,
            // and it is time to reset the remaining execution time of the thread.
            if (tick % thread_blocks[i].priority == 0)
                //  This sets the remaining execution time of the thread to 0, effectively resetting it.
                //  This indicates that the thread is eligible for execution and can be scheduled by the thread scheduler.
                thread_blocks[i].remaining_time = 0;
        }
    }
}

// Kernel thread scheduler
// Function is responsible for selecting the next thread to run based on their states and priorities.
// It ensures that the most appropriate thread is chosen for execution.
void scheduler()
{
    // Check if the current thread is suspended
    if (thread_blocks[current_thread].state == THREAD_SUSPENDED)
    {
        // Find the next available thread
        uint32_t next_thread = (current_thread + 1) % NUM_THREADS;
        while (thread_blocks[next_thread].state != THREAD_RUNNING)
        {
            // The variable next_thread is initialized with the index of the next thread in a circular manner ((current_thread + 1) % NUM_THREADS).
            // This ensures that the scheduler cycles through all the thread control blocks.
            next_thread = (next_thread + 1) % NUM_THREADS;

            // Check if all threads are suspended or terminated
            if (next_thread == current_thread)
            // If the loop completes without finding a running thread, it means that no active threads are available for execution.
            {
                printf("No active threads. Exiting.\n");
                sleep_ms(1000);
                exit(0);
            }
        }

        // Switch to the next thread
        current_thread = next_thread;
    }
    else
    {

        // Find the next available thread based on priority
        // Similar to the previous block, the next_thread is initialized with the index of the next thread in a circular manner.
        // In this case, the scheduler searches for a running thread with the minimum priority among all the available threads.
        uint32_t next_thread = (current_thread + 1) % NUM_THREADS;
        uint32_t min_priority = UINT32_MAX;
        while (thread_blocks[next_thread].state != THREAD_RUNNING || thread_blocks[next_thread].priority >= min_priority)
        {
            // The loop continues until it finds a running thread with a priority lower than min_priority.
            // If a thread with a lower priority is found, min_priority is updated accordingly.
            // If no running thread with a lower priority is found, or if there are no running threads at all, the program prints a message and exits.
            // Once a suitable thread is found, current_thread is updated, and that thread becomes the next thread to execute.
            if (thread_blocks[next_thread].state == THREAD_RUNNING && thread_blocks[next_thread].priority < min_priority)
                min_priority = thread_blocks[next_thread].priority;

            next_thread = (next_thread + 1) % NUM_THREADS;

            // Check if all threads are suspended or terminated
            if (next_thread == current_thread)
            {
                printf("No active threads. Exiting.\n");
                sleep_ms(1000);
                exit(0);
            }
        }

        // Switch to the next thread
        current_thread = next_thread;
    }
}

void suspend_thread(uint32_t thread_id)
{
    thread_blocks[thread_id].state = THREAD_SUSPENDED;
    printf("Thread ID %d suspended.\n", thread_id);
}

void resume_thread(uint32_t thread_id)
{
    thread_blocks[thread_id].state = THREAD_RUNNING;
    printf("Thread ID %d resumed.\n", thread_id);
}

void terminate_thread(uint32_t thread_id)
{
    thread_blocks[thread_id].state = THREAD_TERMINATED;
    printf("Thread ID %d terminated.\n", thread_id);
}

void yield()
{
    // printf("execution_time -----> %d\n", execution_time / 1000);
    uint32_t next_thread = (current_thread + 1) % NUM_THREADS;
    current_thread = next_thread;

    printf("Context Switch: Thread ID %d | Priority %d\n", thread_blocks[current_thread].thread_id, thread_blocks[current_thread].priority);
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

// Function to initialize threads
void init_threads()
{
    // Initialize thread 1
    thread_blocks[0].thread_func = thread_1;
    thread_blocks[0].priority = 3;
    thread_blocks[0].remaining_time = 0;
    thread_blocks[0].thread_id = 1;
    thread_blocks[0].waiting = 0;
    thread_blocks[0].state = THREAD_RUNNING;

    // Initialize thread 2
    thread_blocks[1].thread_func = thread_2;
    thread_blocks[1].priority = 2;
    thread_blocks[1].remaining_time = 0;
    thread_blocks[1].thread_id = 2;
    thread_blocks[1].waiting = 0;
    thread_blocks[1].state = THREAD_RUNNING;

    // Set the current thread to thread 1
    current_thread = 0;
}

int main()
{
    stdio_init_all();
    // Initialize the LED pin
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Initialize the semaphore for LED access
    semaphore_init(&led_semaphore, 1);

    // Initialize the thread blocks
    init_threads();

    // Set up the timer interrupt for kernel tick
    struct repeating_timer kernel_timer;
    add_repeating_timer_us(1000, tick_handler, NULL, &kernel_timer);

    while (1)
    {
        // Execute the current thread function
        thread_blocks[current_thread].thread_func();

        // Perform thread scheduling
        scheduler();

        // Update execution time
        execution_time++;

        // Check if the execution time has reached a threshold
        if (execution_time >= 1000)
        {
            printf("Execution time limit reached. Exiting.\n");
            sleep_ms(1000);
            exit(0);
        }
    }

    return 0;
}
