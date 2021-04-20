//
#ifndef __MAIN_H
#define __MAIN_H

//
#include <string.h>
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_flash_ex.h"

// Требуемые опции
#define WorkOprions			((uint8_t) OB_BOOT_SEL_RESET | OB_SRAM_PARITY_RESET | OB_VDDA_ANALOG_OFF | OB_BOOT1_SET | OB_BOOT0_SET | OB_STDBY_NO_RST | OB_STOP_NO_RST | OB_IWDG_SW )

// Константы времени
#define AckNormal			((uint16_t) 2 )					// Нормальное время подтверждения (в мкс)
#define AckDelayed			((uint16_t) 750 )				// Увеличенное время подтверждения (в мкс)

// Константы карты памяти
#define StateNew			((uint8_t) 0x08 )				// Статус новой карты памяти

// Макросы управления ногами карты памяти
#define MEM_SEL				(GPIOA->IDR & GPIO_PIN_2)		// Чтение сигнала выбора карты памяти
#define MEM_CLK				(GPIOA->IDR & GPIO_PIN_0)		// Чтение сигнала тактирования карты памяти
#define MEM_CMD				(GPIOA->IDR & GPIO_PIN_1)		// Чтение сигнала команды
#define MEM_DAT0			GPIOF->BRR = GPIO_PIN_0			// Установка вывода данных карты памяти в 0
#define MEM_DAT1			GPIOF->BSRR = GPIO_PIN_0		// Установка вывода данных карты памяти в 1
#define MEM_ACK				GPIOF->BRR = GPIO_PIN_1			// Установка сигнала подтверждения в 0
#define MEM_nACK			GPIOF->BSRR = GPIO_PIN_1		// Установка сигнала подтверждения в 1

// Макросы управления лампочками
#define LED_RED_ON			GPIOA->BRR = GPIO_PIN_9			// Зажигаем красную лампочку
#define LED_RED_OFF			GPIOA->BSRR = GPIO_PIN_9		// Тушим красную лампочку
#define LED_GREEN_ON		GPIOA->BRR = GPIO_PIN_10		// Зажигаем зелёную лампочку
#define LED_GREEN_OFF		GPIOA->BSRR = GPIO_PIN_10		// Тушим зелёную лампочку

// Макросы управления картой памяти
#define CARD_ON				GPIOA->BRR = GPIO_PIN_4			// Активация карты
#define CARD_OFF			GPIOA->BSRR = GPIO_PIN_4		// Деактивация карты
#define CARD_nCD			(GPIOB->IDR & GPIO_PIN_1)		// Детект вставления карты

// Макросы коснтант SPI
#define SPI_LOW_SPEED		(SPI_CR1_BR_2 | SPI_CR1_BR_1)	// Прескалер Pclk/128
#define SPI_SIZE_8BIT		(SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0)	// Размер слова 8 бит

// Константы команд SD карты
#define CMD_LENGTH			((uint32_t) 6 )					// Размер команды
#define CARD_DATA_TOKEN		((uint8_t) 0xFE )				// Токен начала данных
#define CARD_CMD17			((uint8_t) 0x51 )				// Команда чтения блока
#define CARD_CMD24			((uint8_t) 0x58 )				// Команда записи блока
// CMD0: GO_IDLE_STATE
static const uint8_t CARD_CMD0[ CMD_LENGTH ] 	= { 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };
// CMD8: SEND_IF_COND
static const uint8_t CARD_CMD8[ CMD_LENGTH ] 	= { 0x48, 0x00, 0x00, 0x01, 0xAA, 0x87 };
// CMD16: SET_BLOCKLEN
static const uint8_t CARD_CMD16[ CMD_LENGTH ] 	= { 0x50, 0x00, 0x00, 0x02, 0x00, 0xFF };
// ACMD55: APP_CMD
static const uint8_t CARD_CMD55[ CMD_LENGTH ] 	= { 0x77, 0x00, 0x00, 0x00, 0x00, 0xFF };
// CMD58: READ_OCR
static const uint8_t CARD_CMD58[ CMD_LENGTH ] 	= { 0x7A, 0x00, 0x00, 0x00, 0x00, 0xFF };
// ACMD41: APP_SEND_OP_COND
static const uint8_t CARD_ACMD41[ CMD_LENGTH ] 	= { 0x69, 0x40, 0x00, 0x00, 0x00, 0xFF };

// Имя файла образа карты памяти
static const uint8_t CARD_IMAGE[ 11 ] = { 'M', 'E', 'M', 'C', 'R', 'D', '0', '0', 'B', 'I', 'N' };

// Найденная карта
typedef enum
{
	ctNone = 0,												// Карта отсутстует
	ctSD1,													// Найдена карта SD v1
	ctSD2,													// Найдена карта SD v2+
	ctSD3,													// Найдена карта SD v2+ блочная
	ctUnknown												// Найдена неизвестная карта
} TCardType;

// Запрошенная операция
typedef enum
{
	coIdle = 0,												// Бездействие
	coRead,													// Чтение
	coWrite													// Запись
} TCardOp;

// Состояния PSIO
typedef enum
{
	mdSync = 0,												// Синхронизация
	mdCmd,													// Принимаем команду для карты
	mdParam,												// Принимаем параметры
	mdWrData,												// Режим приёма данных в буфер для записи
	mdRdData,												// Режим передачи данных
	mdDone,													// Обмен закончен
	mdCard													// Опрос карты
} TMode;

// Структура интерфейса PSIO
typedef struct
{
	TMode			Mode;									// Режим PSIO
	uint8_t			Bits;									// Счётчик бит
	uint8_t			DataIn;									// Принимаемые данные
	uint8_t			DataOut;								// Отправляемые данные
	FunctionalState	Ack;									// Формируется ACK
} TPSIO;

// Структура карты памяти
typedef struct
{
	uint8_t			Status;									// Текущий статус карты памяти
	uint8_t			Cmd;									// Текущая команда
	uint8_t			Check;									// Контрольная сумма
	uint8_t			Bytes;									// Текущий номер байта
	uint16_t		Sector;									// Номер сектора карты памяти
	uint8_t			Data[ 128 ];							// Данные сектора карты памяти
} TMemCard;

// Структура SD карты
typedef struct
{
	TCardType		CardType;								// Тип карты
	TCardOp			CardOp;									// Запрошенная операция
	uint32_t		LoadedLBA;								// Текущая страница в буфере (для оптимизации доступа к карте)
	uint8_t			CardBuf[ 512 ];							// Буфер карты памяти
	uint32_t		CardList[ 256 ];						// Список LBA секторов файла (1 сектор 512 байт)
} TSDCard;

// Глобальные данные
typedef struct
{
	TPSIO			PSIO;									// Структура интерфейса PSIO
	TMemCard		MemCard;								// Структура карты памяти
	TSDCard			SDCard;									// Структура SD карты
} TState;

static TState 		State;									// Глобальные данные

// __MAIN_H
#endif
