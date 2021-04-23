// Главный модуль
#include "main.h"

// Прерывание таймера TIM3
void TIM3_IRQHandler( void )
{	// Снимаем флаг
	TIM3->SR = 0x0000;
	// Анализируем режим
	if ( State.PSIO.Ack == ENABLE )
	{	// Выключаем сигнал ACK
		MEM_nACK;
	}
	else
	{	// Включаем сигнал ACK
		MEM_ACK;
		// Перекидываем режим
		State.PSIO.Ack = ENABLE;
		// Новый таймаут
		TIM3->CNT = 0;
		// Включаем таймер
		TIM3->CR1 |= TIM_CR1_CEN;
	}
}

// Прерывание по фронту CLK
void EXTI0_1_IRQHandler( void )
{	// Подтверждаем прерывание
	EXTI->PR = 0x00000001;
	// Локальные переменные
	uint16_t AckTime;
	// Инит
	AckTime = 0;
	// Считываем данные
	State.PSIO.DataIn >>= 1;
	if ( MEM_CMD )
	{	// Принята 1
		State.PSIO.DataIn |= 0x80;
	}
	else
	{	// Принят 0
		State.PSIO.DataIn &= 0x7F;
	}
	// Считаем биты
	if ( State.PSIO.Bits > 0 )
	{	// Ещё есть биты
		State.PSIO.Bits--;
	}
	else
	{	// Кончились биты?
		if ( State.PSIO.Bits == 0 )
		{	// Биты кончились
			State.PSIO.Bits = 7;
			// Значение по умолчанию
			State.PSIO.DataOut = State.PSIO.DataIn;
			// Анализируем ответ
			switch ( State.PSIO.Mode )
			{	// Режим синхронизации
				case mdSync : {	// Принят первый байт команды
								if ( State.PSIO.DataIn == 0x81 )
								{	// Команда активации карты
									State.PSIO.Mode = mdCmd;
									// Текущий ответ
									State.PSIO.DataOut = State.MemCard.Status;
									// Посылаем ACK
									AckTime = AckNormal;
								}
								else
								if ( State.PSIO.DataIn == 0x01 )
								{	// Команда активации джойстика, нужно игнорировать любую активность до конца.
									State.PSIO.Mode = mdDone;
								}
								// Выход
								break;
							}
				// Получаем команду
				case mdCmd : {	// Меняем режим
								State.PSIO.Mode = mdParam;
								// Сохраняем байт в команду и подготовим буфер
								State.MemCard.Cmd = State.PSIO.DataIn;
								State.MemCard.Bytes = 0;
								// Отвечаем
								State.PSIO.DataOut = 0x5A;
								// Посылаем ACK
								AckTime = AckNormal;
								// Выход
								break;
							}
				// Режим получения параметров
				case mdParam : {	// Почти каждый ответ требует ACK
									AckTime = AckNormal;
									// Принимаем параметры
									switch ( State.MemCard.Cmd )
									{	// Команда чтения: R
										case 0x52 : {	// Анализируем байты
														switch ( State.MemCard.Bytes )
														{	// Просто все варианты
															case 0 : { State.PSIO.DataOut = 0x5D; break; }
															case 1 : { break; }
															case 2 : { State.MemCard.Sector = State.PSIO.DataIn * 0x0100; State.MemCard.Check = State.PSIO.DataIn; break; }
															case 3 : { State.MemCard.Sector += State.PSIO.DataIn; State.MemCard.Check ^= State.PSIO.DataIn; State.PSIO.DataOut = 0x5C;
																	   State.SDCard.CardOp = coRead; AckTime = AckDelayed; break; }
															case 4 : { State.PSIO.DataOut = 0x5D; AckTime = AckDelayed; break; }
															case 5 : { State.PSIO.DataOut = State.MemCard.Sector >> 8; AckTime = AckDelayed; break; }
															case 6 : { State.PSIO.DataOut = State.MemCard.Sector; AckTime = AckDelayed;
																	   State.PSIO.Mode = mdRdData; State.MemCard.Bytes = 0; break; }
															default : { State.PSIO.Mode = mdDone; AckTime = 0; break; }
														}
														// Сигнализируем чтением
														LED_GREEN_ON;
														// Выход
														break;
													}
										// Команда записи: W
										case 0x57 : {	// Анализируем байты
														switch ( State.MemCard.Bytes )
														{	// Просто все варианты
															case 0 : { State.PSIO.DataOut = 0x5D; break; }
															case 1 : { break; }
															case 2 : { State.MemCard.Sector = State.PSIO.DataIn * 0x0100; State.MemCard.Check = State.PSIO.DataIn; break; }
															case 3 : { State.MemCard.Sector += State.PSIO.DataIn; State.MemCard.Check ^= State.PSIO.DataIn; // break; }
																	   State.PSIO.Mode = mdWrData; State.MemCard.Bytes = 0; break; }
															default : { State.PSIO.Mode = mdDone; AckTime = 0; break; }
														}
														// Сигнализируем записью
														LED_RED_ON;
														// Выход
														break;
													}
										// Команда параметров: S
										case 0x53 : {	// Выставляем байт согласно номеру
														switch ( State.MemCard.Bytes )
														{	// Просто все варианты
															case 0 : { State.PSIO.DataOut = 0x5D; break; }
															case 1 : { State.PSIO.DataOut = 0x5C; break; }
															case 2 : { State.PSIO.DataOut = 0x5D; break; }
															case 3 : { State.PSIO.DataOut = 0x04; break; }
															case 4 : { State.PSIO.DataOut = 0x00; break; }
															case 5 : { State.PSIO.DataOut = 0x00; break; }
															case 6 : { State.PSIO.DataOut = 0x80; break; }
															default : { State.PSIO.Mode = mdDone; AckTime = 0; break; }
														}
														// Выход
														break;
													}
										// По умолчанию
										default : { State.PSIO.Mode = mdDone; break; }
									}
									// Считаем номер
									if ( State.PSIO.Mode == mdParam ) { State.MemCard.Bytes++; }
									// Выход
									break;
								}
				// Режим передачи данных для чтения
				case mdRdData : {	// Почти каждый ответ требует ACK
									AckTime = AckNormal;
									// Счётчик байт
									if ( State.MemCard.Bytes < 128 )
									{	// Это передача данных
										State.PSIO.DataOut = State.MemCard.Data[ State.MemCard.Bytes ]; State.MemCard.Check ^= State.PSIO.DataOut;
									}
									else
									{	// Это хвостик за пределами данных
										switch ( State.MemCard.Bytes )
										{	// Передача контрольной суммы
											case 128 : { State.PSIO.DataOut = State.MemCard.Check; break; }
											// Передача завершающего статуса
											case 129 : { State.PSIO.DataOut = 0x47; break; }
											// Завершение работы
											default : { State.PSIO.Mode = mdDone; AckTime = 0; LED_GREEN_OFF; break; }
										}
									}
									// Считаем
									State.MemCard.Bytes++;
									// Выход
									break;
								}
				// Режим приёма данных для записи
				case mdWrData : {	// Почти каждый ответ требует ACK
									AckTime = AckNormal;
									// Счётчик байт
									if ( State.MemCard.Bytes < 128 )
									{	// Это приём данных
										State.MemCard.Data[ State.MemCard.Bytes ] = State.PSIO.DataIn; State.MemCard.Check ^= State.PSIO.DataIn;
									}
									else
									{	// Это хвостик за пределамы данных
										switch ( State.MemCard.Bytes )
										{	// Это приём контрольной суммы
											case 128 : {	// Сравниваем контрольную сумму и выносим решение
															if ( State.MemCard.Check == State.PSIO.DataIn ) { State.MemCard.Check = 0x47; } else { State.MemCard.Check = 0x4E; }
															// Начинаем подтверждать приём
															State.PSIO.DataOut = 0x5C;
															// Выходим
															break;
														}
											// Это хвостик данных
											case 129 : { State.PSIO.DataOut = 0x5D; break; }
											// Это вывод результата команды
											case 130 : {	// Сначала проверим, что сектор задан верно
															if ( State.MemCard.Sector < 0x4000 )
															{	// Сектор верен, отдаём результат проверки
																State.PSIO.DataOut = State.MemCard.Check;
																// Какой результат проверки?
																if ( State.MemCard.Check == 0x47 )
																{	// Заказываем запись сектора в карту памяти
																	State.SDCard.CardOp = coWrite;
																	// После успешной записи обнуляется флаг
																	State.MemCard.Status &= ~StateNew;
																}
															}
															else
															{	// Сектор ошибочен, выдаём ошибку сектора
																State.PSIO.DataOut = 0xFF;
															}
															// Выход
															break;
														}
											// Завершение работы
											default : { State.PSIO.Mode = mdDone; AckTime = 0; break; }
										}
									}
									// Считаем
									State.MemCard.Bytes++;
									// Выход
									break;
								}
				// Заглушка, тупим до конца пакета
				case mdDone : { break; }
				// По умолчанию - откатываемся в начало
				default : { State.PSIO.Mode = mdSync; break; }
			}
		}
	}
	// Выставляем свои данные
	if ( State.PSIO.Mode != mdSync )
	{	// Выставляем текущий бит выводного байта
		if ( State.PSIO.DataOut & 0x01 )
		{	// Выставляем 1
			MEM_DAT1;
		}
		else
		{	// Выставляем 0
			MEM_DAT0;
		}
		// Сдвигаем данные
		State.PSIO.DataOut >>= 1;
	}
	// Требуется ACK?
	if ( AckTime > 0 )
	{	// Установим CNT
		TIM3->CNT = AckTime;
		// Устанавливаем флаг
		State.PSIO.Ack = DISABLE;
		// Сбросим события
		TIM3->SR = 0x0000;
		// Включаем таймер
		TIM3->CR1 |= TIM_CR1_CEN;
	}
}

// Прерывание по перепаду SEL
void EXTI2_3_IRQHandler( void )
{	// Подтверждаем прерывание
	EXTI->PR = 0x00000004;
	// Анализируем состояние SEL
	if ( MEM_SEL )
	{	// SEL = 1
		EXTI->IMR &= 0xFFFFFFFE;
		State.PSIO.Mode = mdSync;
		// Тушим зелёную лампочку
		LED_GREEN_OFF;
	}
	else
	{	// SEL = 0
		EXTI->IMR |= 0x00000001;
		State.PSIO.Bits = 7;
		// Тушим лампочки
		LED_GREEN_OFF; LED_RED_OFF;
	}
	// Обесточиваем
	MEM_DAT1; MEM_nACK;
}

//======================================================================
// Проверяем байт опций и устанавливаем нужный
FunctionalState CheckOptions( void )
{	// Переменные
	FLASH_OBProgramInitTypeDef Options;
	// Считываем текущее значение опций и RDP
	HAL_FLASHEx_OBGetConfig( &Options );
	// Сначала работаем только с опциями
	Options.OptionType = OPTIONBYTE_USER;
	// Нужно менять?
	if ( Options.USERConfig != WorkOprions )
	{	// Опции подлежат обновлению
		Options.USERConfig = WorkOprions;
		// Работа
		while ( 1 )
		{	// Разблокируем
			if ( HAL_FLASH_Unlock() != HAL_OK ) { break; }
			if ( HAL_FLASH_OB_Unlock() != HAL_OK ) { break; }
			// Стираем
			if ( HAL_FLASHEx_OBErase() != HAL_OK) { break; }
			// Записываем их
			if ( HAL_FLASHEx_OBProgram( &Options ) != HAL_OK ) { break; }
			// Заблокируем
			if ( HAL_FLASH_OB_Lock() != HAL_OK ) { break; }
			if ( HAL_FLASH_Lock() != HAL_OK ) { break; }
//			if ( HAL_FLASH_OB_Launch() != HAL_OK ) { break; }
			// Выходим
			break;
		}
		// Считываем текущее значение опций и RDP
		HAL_FLASHEx_OBGetConfig( &Options );
	}
	// Проверяем корректность только опций
	if ( Options.USERConfig == WorkOprions )
	{	// Все OK
		return ENABLE;
	}
	else
	{	// Выход с ошибкой
		return DISABLE;
	}
}

// Посылаем команду и ждём окончания передачи
void Card_SendCMD(const uint8_t *Buf, uint32_t Size )
{	// Локальная переменная
	uint16_t Data;
	// Если есть данные
	while ( Size > 0 )
	{	// Первым делом формируем данные, с учётом пакинга для оптимизации
		Data = *(Buf); Buf++; Size--;
		if ( Size > 0 )
		{	// Забираем второй байт тоже
			Data += (*(Buf) * 0x100); Buf++; Size--;
		}
		// Данные сформированы, ожидаем готовности буфера передачи
		while ( (SPI1->SR & SPI_SR_TXE) == 0 ) {}
		// Передаём данные
		SPI1->DR = Data;
	}
	// Ожидаем опустошения очереди передачи
	while ( (SPI1->SR & SPI_SR_FTLVL) != 0 ) { Data = SPI1->DR; }
	// Ждём завершения последней транзакции
	while ( (SPI1->SR & SPI_SR_BSY) != 0 ) { Data = SPI1->DR; }
	// Опусташаем буфер приёма
	while ( (SPI1->SR & SPI_SR_FRLVL) != 0 ) { Data = SPI1->DR; }
}

// Посылаем байт и ждём ответ
uint8_t Card_SPI( uint8_t Data )
{	// Локальная переменная
	uint8_t Res;
	// Ожидаем готовности передатчика
	while ( (SPI1->SR & SPI_SR_FTLVL) != 0 ) { }
	// Посылаем слово
	*((__IO uint8_t *)&SPI1->DR) = Data;
	// Ожидаем готовности приёмника
	while ( (SPI1->SR & SPI_SR_RXNE) == 0 ) { }
	// Вычитываем
	Res = *((__IO uint8_t *)&SPI1->DR);
	// Выход
	return Res;
}

// Ожидаем ответ
uint8_t Card_WaitResp( uint32_t *OCR, FunctionalState R7, uint8_t Count )
{	// Локальная переменная
	uint8_t Res,Cnt;
	// Инит переменной
	*(OCR) = 0xFFFFFFFF;
	// Поиск первого байта ответа
	do
	{	// Получаем данные
		Res = Card_SPI( 0xFF );
		// И уменьшаем счётчик
		Count--;
	}
	while ( ((Res & 0xC0) != 0) && (Count > 0) );
	// Проверяем, требуется ли догрузка ещё 4х байт?
	if ( R7 == ENABLE )
	{	// Догружаем ещё 4 байт (всего 40 бит вместе с Res)
		for (Cnt = 0; Cnt < 4;Cnt++)
		{	// Готовим место под байт
			*(OCR) <<= 8; *(OCR) &= 0xFFFFFF00;
			// Получаем данные
			*(OCR) += Card_SPI( 0xFF );
		}
	}
	// Ожидаем как минимум 1 байт с 0xFF
	while ( Card_SPI( 0xFF ) != 0xFF ) { }
	// Выход
	return Res;
}

// Инициализация карты памяти
TCardType Card_Init( void )
{	// Локальные переменные
	TCardType Res;
	uint32_t Cnt,OCR;
	uint8_t Dat, Resp;
	// Отключаем карту
	CARD_OFF; Res = ctNone;
	// Настраиваем SPI на медленную скорость PCLK/128: 48/128 = 0,375МГц
	SPI1->CR1 &= ~SPI_CR1_SPE;
	SPI1->CR1 = SPI_CR1_MSTR | SPI_LOW_SPEED;
	SPI1->CR1 |= SPI_CR1_SPE;
	// Топчемся на месте
	HAL_Delay( 1 );
	// Посылаем инит 256 байт
	for (Cnt = 0;Cnt < 256;Cnt++ )
	{	// Послыаем слово
		Card_SPI( 0xFF );
	}
	// Начинаем инициализацию карты
	CARD_ON;
	// Ожидаем готовности карты
	do
	{	// Посылаем 0xFF
		Dat = Card_SPI( 0xFF );
	} while ( Dat != 0xFF );
	// CMD0: GO_IDLE_STATE
	Card_SendCMD( &CARD_CMD0[ 0 ], CMD_LENGTH ); Resp = Card_WaitResp( &OCR, DISABLE, 128 );
	// Какой ответ получен?
	if ( Resp == 0x01 )
	{	// Карта вошла в IDLE_STATE, посылаем CMD8: SEND_IF_COND
		Card_SendCMD( &CARD_CMD8[ 0 ], CMD_LENGTH ); Resp = Card_WaitResp( &OCR, ENABLE, 128 );
		// Если был дан адекватный респонс
		if ( Resp != 0x01 )
		{	// Это ветка SDv1/MMC
			do
			{	// Посылаем ACMD41: APP_SEND_OP_COND
				Card_SendCMD( &CARD_ACMD41[ 0 ], CMD_LENGTH ); Resp = Card_WaitResp( &OCR, ENABLE, 128 );
			} while ( Resp == 0x01 );
			// Каков был ответ?
			if ( Resp == 0x00 )
			{	// Обнаружена карта SD v1
				Res = ctSD1;
			}
			else
			{	// Это ветка MMC, нам её некуда втыкать
				Res = ctUnknown;
			}
		}
		else
		{	// Это ветка SDv2
			if ( (OCR & 0x0001FF) == 0x0001AA )
			{	// Это карта SDv2
				do
				{	// Посылаем ACMD55: APP_CMD
					Card_SendCMD( &CARD_CMD55[ 0 ], CMD_LENGTH ); Resp = Card_WaitResp( &OCR, DISABLE, 128 );
					// Если ответ правильный
					if ( Resp == 0x01 )
					{	// Посылаем ACMD41: APP_SEND_OP_COND
						Card_SendCMD( &CARD_ACMD41[ 0 ], CMD_LENGTH ); Resp = Card_WaitResp( &OCR, ENABLE, 128 );
					}
				} while ( Resp == 0x01 );
				// Каков был ответ?
				if ( Resp == 0x00 )
				{	// Посылаем CMD58: READ_OCR
					Card_SendCMD( &CARD_CMD58[ 0 ], CMD_LENGTH ); Resp = Card_WaitResp( &OCR, ENABLE, 128 );
					// Каков ответ?
					if ( Resp == 0x00 )
					{	// Анализируем OCR
						if ( (OCR & 0x40000000) == 0x00000000 )
						{	// Карта обычной ёмкости
							Res = ctSD2;
						}
						else
						{	// Карта повышенной ёмкости
							Res = ctSD3;
						}
					}
					else
					{	// Эта карта неисправна
						Res = ctUnknown;
					}
				}
				else
				{	// Эта карта неисправна
					Res = ctUnknown;
				}
			}
			else
			{	// Эта карта неисправна
				Res = ctUnknown;
			}
		}
	}
	else
	{	// Карта ответила неправильно
		if ( Res != 0xFF ) { Res = ctUnknown; }
	}
	// Только для карт обычной ёмкости
	if ( (Res == ctSD1) || (Res == ctSD2) )
	{	// Устанавливаем размер блока 512 байт
		// CMD16: SET_BLOCKLEN
		Card_SendCMD( &CARD_CMD16[ 0 ], CMD_LENGTH ); Resp = Card_WaitResp( &OCR, DISABLE, 128 );
		// Каков ответ?
		if ( Resp != 0x00 )
		{	// Эта карта неисправна
			Res = ctUnknown;
		}
	}
	// Выключаем карту
	while ( (SPI1->SR & SPI_SR_BSY) != 0x0000 ) { }
	CARD_OFF;
	// Если карта инициализирована
	if ( (Res != ctNone) && (Res != ctUnknown) )
	{	// Настраиваем SPI на быструю скорость PCLK/2: 48/2 = 24МГц
		SPI1->CR1 &= ~SPI_CR1_SPE;
		SPI1->CR1 = SPI_CR1_MSTR;
		SPI1->CR1 |= SPI_CR1_SPE;
	}
	// Выходим
	return Res;
}

// Чтение сектора карты памяти без DMA
FunctionalState Card_Read( TCardType CardType, uint8_t *Buf, uint32_t *Loaded, uint32_t Addr )
{	// Локальные переменные
	FunctionalState Res;
	uint8_t Cmd[ 6 ];
	uint8_t Dat,Resp;
	uint32_t Cnt;
	// Инит
	Res = DISABLE;
	// Посмотрим, у нас в буфере уже загружено?
	if ( *(Loaded) != Addr )
	{	// Сохраняем новый номер сектора
		*(Loaded) = Addr;
		// Корректируем адрес для старых карт
		if ( (CardType == ctSD1) || (CardType == ctSD2) )
		{	// У старых карт адрес вместо LBA
			Addr *= 0x00000200;
		}
		// Работаем
		while ( 1 )
		{	// Если тип карты неправильный - выходим
			if ( CardType == ctNone ) { break; }
			if ( CardType == ctUnknown ) { break; }
			// Готовим команду на чтение сектора
			Cmd[ 0 ] = CARD_CMD17;
			Cmd[ 1 ] = Addr >> 24;
			Cmd[ 2 ] = Addr >> 16;
			Cmd[ 3 ] = Addr >> 8;
			Cmd[ 4 ] = Addr;
			Cmd[ 5 ] = 0xFF;
			// Включаем карту
			CARD_ON;
			// Ожидаем готовности карты
			do
			{	// Посылаем 0xFF
				Dat = Card_SPI( 0xFF );
			} while ( Dat != 0xFF );
			// Посылаем команду чтения
			Card_SendCMD( &Cmd[ 0 ], CMD_LENGTH ); Resp = Card_WaitResp( (uint32_t *)&Cmd[ 0 ], DISABLE, 128 );
			// Анализируем ответ на команду
			if ( Resp != 0x00 ) { break; }
			// Ожидаем токен данных
			Cnt = 2048;
			do
			{	// Считываем данные
				Dat = Card_SPI( 0xFF );
				// Считаем
				Cnt--;
			} while ( (Dat == 0xFF) && (Cnt > 0) );
			// Таймаут?
			if ( Cnt == 0 ) { break; }
			// Ошибка в токене?
			if ( Dat != CARD_DATA_TOKEN ) { break; }
			// Начались данные, загружаем
			for (Cnt = 0;Cnt < 512;Cnt++)
			{	// Считываем данные
				*(Buf) = Card_SPI( 0xFF ); Buf++;
			}
			// Дочитываем CRC
			Cmd[ 0 ] = Card_SPI( 0xFF );
			Cmd[ 1 ] = Card_SPI( 0xFF );
			// Без ошибок
			Res = ENABLE;
			// Выход
			break;
		}
	}
	else
	{	// Без ошибок
		Res = ENABLE;
	}
	// Выключаем карту
	while ( (SPI1->SR & SPI_SR_BSY) != 0x0000 ) { }
	CARD_OFF;
	// Если была ошибка, обнулим номер
	if ( Res == DISABLE ) { *(Loaded) = 0xFFFFFFFF; }
	// Выход
	return Res;
}

// Запись сектора карты памяти без DMA
FunctionalState Card_Write( TCardType CardType, uint8_t *Buf, uint32_t *Loaded, uint32_t Addr )
{	// Локальные переменные
	FunctionalState Res;
	uint8_t Cmd[ 6 ];
	uint8_t Dat,Resp;
	uint32_t Cnt;
	// Инит
	Res = DISABLE;
	// Корректируем адрес для старых карт
	if ( (CardType == ctSD1) || (CardType == ctSD2) )
	{	// У старых карт адрес вместо LBA
		Addr *= 0x00000200;
	}
	// Работаем
	while ( 1 )
	{	// Если тип карты неправильный - выходим
		if ( CardType == ctNone ) { break; }
		if ( CardType == ctUnknown ) { break; }
		// Готовим команду на чтение сектора
		Cmd[ 0 ] = CARD_CMD24;
		Cmd[ 1 ] = Addr >> 24;
		Cmd[ 2 ] = Addr >> 16;
		Cmd[ 3 ] = Addr >> 8;
		Cmd[ 4 ] = Addr;
		Cmd[ 5 ] = 0xFF;
		// Включаем карту
		CARD_ON;
		// Ожидаем готовности карты
		do
		{	// Посылаем 0xFF
			Dat = Card_SPI( 0xFF );
		} while ( Dat != 0xFF );
		// Посылаем команду чтения
		Card_SendCMD( &Cmd[ 0 ], CMD_LENGTH ); Resp = Card_WaitResp( (uint32_t *)&Cmd[ 0 ], DISABLE, 128 );
		// Анализируем ответ на команду
		if ( Resp != 0x00 ) { break; }
		// Посылаем токен данных
		Card_SPI( CARD_DATA_TOKEN );
		// Посылаем данные в цикле
		// Начались данные, загружаем
		for (Cnt = 0;Cnt < 512;Cnt++)
		{	// Считываем данные
			Card_SPI( *(Buf) ); Buf++;
		}
		// Досылаем CRC
		Card_SPI( 0xFF );
		Card_SPI( 0xFF );
		// Без ошибок
		Res = ENABLE;
		// Выход
		break;
	}
	// Выключаем карту
	while ( (SPI1->SR & SPI_SR_BSY) != 0x0000 ) { }
	CARD_OFF;
	// Успешно?
	if ( Res == ENABLE )
	{	// Сохраняем новый номер сектора
		*(Loaded) = Addr;
	}
	else
	{	// Обнуляем
		*(Loaded) = 0xFFFFFFFF;
	}
	// Выход
	return Res;
}

// Инициализация таблицы секторов по имени файла, поддерживается пока только FAT16
FunctionalState Card_FSInit( TSDCard *SDCard, const uint8_t *FName )
{	// Локальные переменные
	FunctionalState Res;
	uint8_t *Buf;
	uint8_t Pos;
	uint16_t ClustSize,Reserv,RootSize,FATSize,Cluster;
	uint32_t Cnt,LBA,SysOrg,FATOrg,RootOrg,DataOrg;
	int Compare;
	// Инит
	Res = DISABLE; SysOrg = 0; Cluster = 0xFFFF;
	// Начинаем с самого сначала
	while ( 1 )
	{	// Вычитываем сектор 0
		if ( Card_Read( SDCard->CardType, &SDCard->CardBuf[ 0 ], &SDCard->LoadedLBA, SysOrg ) == DISABLE ) { break; }
		// Анализируем сектор #0 на MBR
		if ( *((uint16_t *)&SDCard->CardBuf[ 0x01FE ]) != 0xAA55 ) { break; }
		// Проверим косвенные признаки MBR
		if ( ((SDCard->CardBuf[ 0x01BE ] == 0x00) || (SDCard->CardBuf[ 0x01BE ] == 0x80)) &&
			 ((SDCard->CardBuf[ 0x01CE ] == 0x00) || (SDCard->CardBuf[ 0x01CE ] == 0x80)) &&
			 ((SDCard->CardBuf[ 0x01DE ] == 0x00) || (SDCard->CardBuf[ 0x01DE ] == 0x80)) &&
			 ((SDCard->CardBuf[ 0x01EE ] == 0x00) || (SDCard->CardBuf[ 0x01EE ] == 0x80)) )
		{	// Похоже на MBR, анализируем таблицу разделов
			for (Cnt = 0;Cnt < 4;Cnt++)
			{	// Анализируем признак раздела
				if ( (SDCard->CardBuf[ (Cnt * 0x0010) + 0x01C2 ] == 0x01) ||	// Сигнатура 0x01: FAT12
					 (SDCard->CardBuf[ (Cnt * 0x0010) + 0x01C2 ] == 0x04) ||	// Сигнатура 0x04: FAT16
					 (SDCard->CardBuf[ (Cnt * 0x0010) + 0x01C2 ] == 0x06) ||	// Сигнатура 0x06: Big FAT16
					 (SDCard->CardBuf[ (Cnt * 0x0010) + 0x01C2 ] == 0x0E) )		// Сигнатура 0x0E: vFAT
				{	// Сигнатура подошла, забираем адрес MBS раздела
					SysOrg = SDCard->CardBuf[ (Cnt * 0x0010) + 0x01C6 ];
					SysOrg += (SDCard->CardBuf[ (Cnt * 0x0010) + 0x01C7 ] * 0x00000100);
					SysOrg += (SDCard->CardBuf[ (Cnt * 0x0010) + 0x01C8 ] * 0x00010000);
					SysOrg += (SDCard->CardBuf[ (Cnt * 0x0010) + 0x01C9 ] * 0x01000000);
					// Выходим
					break;
				}
			}
		}
		// Загружаем сектор предполагаемого MBS
		if ( Card_Read( SDCard->CardType, &SDCard->CardBuf[ 0 ], &SDCard->LoadedLBA, SysOrg ) == DISABLE ) { break; }
		// Анализируем сектор на MBS
		if ( *((uint16_t *)&SDCard->CardBuf[ 0x01FE ]) != 0xAA55 ) { break; }
		if ( SDCard->CardBuf[ 0x000D ] == 0x00 ) { break; }
		if ( (SDCard->CardBuf[ 0x0010 ] == 0x00) || (SDCard->CardBuf[ 0x0010 ] > 0x02) ) { break; }
		if ( SDCard->CardBuf[ 0x0015 ] != 0xF8 ) { break; }
		if ( *((uint32_t *)&SDCard->CardBuf[ 0x001C ]) != SysOrg ) { break; }
		if ( SDCard->CardBuf[ 0x0026 ] != 0x29 ) { break; }
		if ( *((uint16_t *)&SDCard->CardBuf[ 0x0036 ]) != 0x4146 ) { break; }
		if ( *((uint16_t *)&SDCard->CardBuf[ 0x0038 ]) != 0x3154 ) { break; }
		if ( SDCard->CardBuf[ 0x003A ] != 0x36 ) { break; }
		// Заполняем локальные переменные, которые нужны для математики
		ClustSize = SDCard->CardBuf[ 0x000D ];
		Reserv = *((uint16_t *)&SDCard->CardBuf[ 0x000E ]);
		RootSize = (SDCard->CardBuf[ 0x0012 ] * 0x0100) + SDCard->CardBuf[ 0x0011 ];
		FATSize = *((uint16_t *)&SDCard->CardBuf[ 0x0016 ]);
		// Вычисляем координаты FAT и ROOT
		FATOrg = SysOrg + Reserv;
		RootOrg = FATOrg + (FATSize * 2);
		DataOrg = RootOrg + (RootSize / 16 );
		// Все данные получены, приступаем к поиску имени файла нужного имиджа
		for (LBA = 0;LBA < (RootSize / 16);LBA++)
		{	// Загружаем сектор корневой папки
			if ( Card_Read( SDCard->CardType, &SDCard->CardBuf[ 0 ], &SDCard->LoadedLBA, RootOrg + LBA ) == ENABLE )
			{	// Перебираем 16 элементов, которые могут находиться в секторе
				for (Cnt = 0;Cnt < 16;Cnt++)
				{	// Сравниваем имя
					Compare = memcmp( &SDCard->CardBuf[ Cnt * 32 ], &CARD_IMAGE[ 0 ], 11 );
					if (  Compare == 0 )
					{	// Файл найден, проверим размер
						if ( *((uint32_t *)&SDCard->CardBuf[ (Cnt * 32) + 0x001C ]) == 0x00020000 )
						{	// Размер подходит, копируем номер кластера
							Cluster = *((uint16_t *)&SDCard->CardBuf[ (Cnt * 32) + 0x001A ]);
							// Без ошибок
							Res = ENABLE;
							// Выходим
							break;
						}
					}
				}
				// Если файл найден - выходим экстренно
				if ( Res == ENABLE ) { break; }
			}
			else
			{	// ошибка загрузки - вываливаемся
				break;
			}
		}
		// Файл найден, данные получены, начинаем построение таблицы доступа
		if ( Res == ENABLE )
		{	// У нас есть номер кластера, готовимся заполнять табличку
			Pos = 0;
			do
			{	// Проверяем номер кластера
				if ( Cluster < 0x0002 )
				{	// Ошибка, выходим
					Res = DISABLE; break;
				}
				// Вычисляем LBA данных кластера
				LBA = DataOrg + ((Cluster - 2) * ClustSize);
				// В цикле по размеру кластера заполняем элементы таблицы
				for (Cnt = 0;Cnt < ClustSize;Cnt++)
				{	// Вычисляем LBA сектроа внутри кластера
					SDCard->CardList[ Pos ] = LBA + Cnt;
					// Следующий элемент
					Pos++; if ( Pos == 0 ) { break; }
				}
				// Если есть ещё элементы, надо получить новый номер кластера
				// А для этого надо вычислить номер сектора, где этот кластер находится и загрузить его по цепочке
				if ( Pos != 0 )
				{	// Вычисляем сектор нахождения кластера
					LBA = FATOrg; Reserv = Cluster;
					while ( Reserv > 256 ) { LBA++; Reserv -= 256; }
					// Загружаем этот сектор в память
					if ( Card_Read( SDCard->CardType, &SDCard->CardBuf[ 0 ], &SDCard->LoadedLBA, LBA ) == ENABLE )
					{	// Забираем новый номер кластера
						Cluster = *((uint16_t *)&SDCard->CardBuf[ Reserv * 2 ]);
					}
					else
					{	// Ошибка загрузки
						Res = DISABLE; break;
					}
				}
			} while ( (Cluster != 0xFFFF) && (Pos != 0) );
		}
		// Выход
		break;
	}
	// Выход
	return Res;
}

// Вход
int main( void )
{	// Переменные
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	uint32_t Cnt,Ofs;
	uint8_t LBA;

	// Базовая настройка HAL
	HAL_Init();

	// Включаем генератор HSI48
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
	RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	HAL_RCC_OscConfig( &RCC_OscInitStruct );
	// Инит CPU, AHB и APB
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_0 );

	// Включаем тактирование
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();

	// Инит периферии
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_TIM3_CLK_ENABLE();
	__HAL_RCC_SPI1_CLK_ENABLE();

	// Включаем ножки лампочек
	GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );
	LED_GREEN_OFF; LED_RED_OFF;

	// Включаем ножки PSIO
	// Входы: PA0 - SEL, PA2 - CLK, PA1 - DAT
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );
	// Выходы: PF0 - CMD, PF1 - ACK
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );
	MEM_DAT1;
	// Выход: PF1 - ACK
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );
	MEM_nACK;

	// Включаем ногу управления картой
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );
	CARD_OFF;
	// Включаем ногу статуса карты
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );
	// Включаем ножки SPI: SCK, MISO, MOSI
	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

	// Настраиваем SPI1 на работу мастера
	SPI1->CR1 = SPI_CR1_MSTR | SPI_LOW_SPEED;
	SPI1->CR2 = SPI_CR2_SSOE | SPI_CR2_FRXTH | SPI_SIZE_8BIT;

	// Запускаем таймер ACK с базой 1мс
	TIM3->PSC = 0x002F; TIM3->ARR = 0xFFFF; TIM3->CCMR1 = 0x0000; TIM3->CCMR2 = 0x0000;
	TIM3->CCER = 0x0000; TIM3->SMCR = 0x0000; TIM3->DIER = TIM_DIER_UIE;
	TIM3->CR1 = TIM_CR1_DIR | TIM_CR1_OPM; TIM3->CR2 = 0x0000;

	// EXTI: PA0 - SEL, PA2 - CLK
	SYSCFG->EXTICR[0] &= 0xFFFFF0F0;
	EXTI->FTSR |= 0x00000004; EXTI->RTSR |= 0x00000005;

	// Инициализируем переменные перед разрешением прерываний
	State.PSIO.Mode = mdSync; State.PSIO.Bits = 0; State.PSIO.DataIn = 0x00; State.PSIO.DataOut = 0; State.PSIO.Ack = DISABLE;
	State.MemCard.Status = StateNew;
	State.SDCard.CardType = ctNone; State.SDCard.CardOp = coIdle; State.SDCard.LoadedLBA = 0xFFFFFFFF;

	// Обнуляем буферы
	for (Cnt = 0;Cnt < 512;Cnt++) { State.SDCard.CardBuf[ Cnt ] = 0x00; }
	for (Cnt = 0;Cnt < 256;Cnt++) { State.SDCard.CardList[ Cnt ] = 0; }

	// NVIC
	HAL_NVIC_SetPriority( EXTI0_1_IRQn, 0, 0 );
	HAL_NVIC_EnableIRQ( EXTI0_1_IRQn );
	HAL_NVIC_SetPriority( EXTI2_3_IRQn, 0, 0 );
	HAL_NVIC_EnableIRQ( EXTI2_3_IRQn );
	HAL_NVIC_SetPriority( TIM3_IRQn, 0, 0 );
	HAL_NVIC_EnableIRQ( TIM3_IRQn );

	// Проверяем байт опций
	CheckOptions();

	// Основной цикл
	while ( 1 )
	{	// Обрабатываем сигнал вытаскивания карты
		if ( CARD_nCD == 0 )
		{	// Карта вставлена
			if ( State.SDCard.CardType == ctNone )
			{	// Включаем зелёную лампочку
				LED_GREEN_ON; LED_RED_OFF;
				// Карту только что поменяли, пытаемся обнаружить
				State.SDCard.CardType = Card_Init();
				// Карта обнаружена?
				if ( State.SDCard.CardType != ctUnknown )
				{	// Анализируем файловую систему карты
					if ( Card_FSInit( &State.SDCard, &CARD_IMAGE[ 0 ] ) == ENABLE )
					{	// Файлоавая система опознана, разрешаем работу
						EXTI->IMR |= 0x00000004;
						// Выключаем лампочки
						LED_GREEN_OFF; LED_RED_OFF;
					}
					else
					{	// Файловая система не опознана
						State.SDCard.CardType = ctUnknown;
						// Зажигаем обе лампочки
						LED_GREEN_ON; LED_RED_ON;
					}
				}
				else
				{	// Зажигаем обе лампочки
					LED_GREEN_ON; LED_RED_ON;
				}
			}
		}
		else
		{	// Карта отсутствует
			if ( State.SDCard.CardType != ctNone )
			{	// Только вытащили, отключаем PSIO
				EXTI->IMR &= 0xFFFFFFFA;
				// Обнуляем все переменные
				State.PSIO.Mode = mdSync; State.PSIO.Bits = 0; State.PSIO.DataIn = 0x00; State.PSIO.DataOut = 0; State.PSIO.Ack = DISABLE;
				State.MemCard.Status = StateNew;
				State.SDCard.CardType = ctNone; State.SDCard.CardOp = coIdle; State.SDCard.LoadedLBA = 0xFFFFFFFF;
			}
			// Потушим обе лампочки
			LED_GREEN_OFF; LED_RED_OFF;
		}
		// Если карта есть
		if ( (State.SDCard.CardType != ctNone) && (State.SDCard.CardType != ctUnknown) )
		{	// Заказана запись?
			if ( State.SDCard.CardOp == coWrite )
			{	// Вычисляем сектор чтения и смещение в блоке
				Ofs = State.MemCard.Sector & 0x03FF;
				LBA = (Ofs >> 2) & 0x000000FF;
				Ofs = (Ofs << 7) & 0x00000180;
				// Считываем сектор в буфер
				Card_Read( State.SDCard.CardType, &State.SDCard.CardBuf[ 0 ], &State.SDCard.LoadedLBA, State.SDCard.CardList[ LBA ] );
				// Подменяем наш сектор
				for (Cnt = 0;Cnt < 128;Cnt++)
				{	// Переносим данные
					State.SDCard.CardBuf[ Ofs + Cnt ] = State.MemCard.Data[ Cnt ];
				}
				// Пишем сетор назад
				Card_Write( State.SDCard.CardType, &State.SDCard.CardBuf[ 0 ], &State.SDCard.LoadedLBA, State.SDCard.CardList[ LBA ] );
				// Потушем лампочку
				LED_RED_OFF;
				// Снимаем флаг
				State.SDCard.CardOp = coIdle;
			}
			// Заказано чтение?
			if ( State.SDCard.CardOp == coRead )
			{	// Вычисляем сектор чтения и смещение в блоке
				Ofs = State.MemCard.Sector & 0x03FF;
				LBA = (Ofs >> 2) & 0x000000FF;
				Ofs = (Ofs << 7) & 0x00000180;
				// Считываем сектор в буфер
				Card_Read( State.SDCard.CardType, &State.SDCard.CardBuf[ 0 ], &State.SDCard.LoadedLBA, State.SDCard.CardList[ LBA ] );
				// Копируем нужный сектор
				for (Cnt = 0;Cnt < 128;Cnt++)
				{	// Переносим данные
					State.MemCard.Data[ Cnt ] = State.SDCard.CardBuf[ Ofs + Cnt ];
				}
				// Снимаем флаг
				State.SDCard.CardOp = coIdle;
			}
		}
	}
}
