#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define CUSTOMER_NUMBER 25
#define COFFEE_TIME_MIN 2
#define COFFEE_TIME_MAX 5
#define REGISTER_NUMBER 5
#define CUSTOMER_ARRIVAL_TIME_MIN 1
#define CUSTOMER_ARRIVAL_TIME_MAX 3

int servedCustomer = 0;
int fullregister = 0;
sem_t waitingQueue;
sem_t register_sem;
sem_t customer_sem;

int *custArr; // queued customer array

void *customerfunc(void *);
void *registerfunc(void *);

int coffeeTime() // Random process time
{
    srand(time(0));
    int randtime = COFFEE_TIME_MIN + rand() % (COFFEE_TIME_MAX - 1);
    sleep(randtime);
    return randtime;
}
int customerTime() // Random customer coming time
{
    srand(time(0));
    int randtime = CUSTOMER_ARRIVAL_TIME_MIN + rand() % CUSTOMER_ARRIVAL_TIME_MAX;
    return randtime;
}
int main()
{
    pthread_t registerThread[REGISTER_NUMBER];
    pthread_t customerThread[CUSTOMER_NUMBER];
    int i;
    custArr = (int *)malloc(sizeof(int) * CUSTOMER_NUMBER); // To create a set of customer that we set the limit

    sem_init(&register_sem, 0, 0); // Starting semaphores with the Semi method
    sem_init(&customer_sem, 0, 0);
    sem_init(&waitingQueue, 0, 1);

    printf("\nThe simulation is started!\n\n");

    for (int i = 0; i < REGISTER_NUMBER; i++)
    {
        pthread_create(&registerThread[i], NULL, (void *)registerfunc, (void *)&i); // Creating register and customer threads with pthread_create method
        sleep(1);                                                                   // for the previous one to finish while being created
    }
    for (int i = 0; i < CUSTOMER_NUMBER; i++)
    {
        int customerComingTime = customerTime();
        sleep(customerComingTime);

        pthread_create(&customerThread[i], NULL, (void *)customerfunc, (void *)(&i));
        printf("CUSTOMER %d IS CREATED AFTER %d SECONDS.\n\n", i, customerComingTime); // Creating customer is printed on the screen
    }
    for (int i = 0; i < CUSTOMER_NUMBER; i++)
    {
        pthread_join(customerThread[i], NULL);
    }

    sleep(COFFEE_TIME_MAX + 1);
    printf("\nAll customers are gone. Simulation is closing... \n\n");
    return EXIT_SUCCESS;
}

void *customerfunc(void *id)
{
    int customerId = *(int *)id;
    int queue;
    sem_wait(&waitingQueue); // Acces to the customer who queue

    fullregister = (++fullregister) % CUSTOMER_NUMBER; // Queue is organized according to the number of customers
    queue = fullregister;
    custArr[queue] = customerId; // Customer queues up

    sem_post(&waitingQueue);
    sem_post(&register_sem);
    sem_wait(&customer_sem); // Access to the queue is removed and the checkout processes

    pthread_exit(0);
}

void *registerfunc(void *id)
{
    int registerID = *(int *)id; // to keep register id
    int customerID, nextcustomer;
    while (1)
    {
        sem_wait(&register_sem); // Access to the vault where the customer is not present is blocked.
        sem_wait(&waitingQueue);

        servedCustomer = (++servedCustomer) % CUSTOMER_NUMBER; // The customer who will buy the coffee becomes one of the waiting customers.
        nextcustomer = servedCustomer;
        customerID = custArr[nextcustomer];
        custArr[nextcustomer] = pthread_self(); // returns the ID of the thread

        sem_post(&waitingQueue); // access to the cash register opens and the cashier processes the customer.
        sem_post(&customer_sem);

        printf("CUSTOMER %d GOES TO REGISTER %d.\n\n", customerID, registerID); // After the specified process time, the process ends and the thread ends.
        int processTime = coffeeTime();
        printf("CUSTOMER %d FINISHED BUYING FROM REGISTER %d AFTER %d SECONDS.\n\n", customerID, registerID, processTime);
    }
    pthread_exit(0);
}
