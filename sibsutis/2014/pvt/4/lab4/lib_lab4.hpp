/****************************************/
/*				Заголовочный файл lab4				*/
/****************************************/

#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <unistd.h>

/*Список констант*/
enum {
	NUM_THREAD_PROD = 1,			/*Количество производителей*/
	NUM_THREAD_CONS = 4, 			/*Количество потребителей*/
	SIZE_QUEUE = 5,						/*Длина очереди*/
	MIN_ELEM_RAND = 1,				/*Минимальный элемент рандома*/
	MAX_ELEM_RAND = 9,	      /*Максимальный элементов рандома*/
};

/*Базовый тип очереди*/
typedef std::vector<int> queueArrayBaseType;

/*Дескриптор очереди*/
typedef struct {
    queueArrayBaseType buf[SIZE_QUEUE];		// Буфер очереди
    unsigned ukEnd;												// Указатель на хвост (по нему включают)
    unsigned ukBegin;											// Указатель на голову (по нему исключают)
    unsigned len;													// Количество элементов в очереди
} QueueArray;

/*Функции работы с очередью*/
void initQueueArray(QueueArray *F);													// Инициализация очереди
void putQueueArray(QueueArray *F, queueArrayBaseType E);		// Включение в очередь
void getQueueArray(QueueArray *F, queueArrayBaseType *E);		// Исключение из очереди
int isFullQueueArray(QueueArray *F);												// Предикат: полна ли очередь
int isEmptyQueueArray(QueueArray *F);												// Предикат: пуста ли очередь
int getCountQueueArray(QueueArray *F);

/*Функции работы с потоками*/
bool sort_v(int i, int j);
void producer();
void consumer(int id);
