// Проект elevator 
// (c) И.К. Марчевский, 2021

/*!
\file
\brief Заголовочный файл с описанием основного класса Control
\author Марчевский Илья Константинович
\version 0.4
\date 14 апреля 2021 г.
*/

#pragma once

#include <memory>

#include "Elevator.h"
#include "FloorButtons.h"


class Queue;

/*!
\brief Основной класс --- симулятор пассажирского лифта
*/
class Control
{
private:
	/// Вектор из умных указателей на кабины лифтов
	std::vector<std::unique_ptr<Elevator>> elevators;
	
	/// Умный указатель на структуру, содержащую состояние кнопок на этажах
	std::unique_ptr<FloorButtons> floorButtons;
	
	/// Текущее время в секундах от начала моделирования	
	size_t time;

	/// Функция инкремента времени
	void TimeIncrement();

	/// Умный указатель на класс-очередь пассажиров на этажах
	std::unique_ptr<Queue> queue; 
	
	/// Проверка появления пассажиров на этажах и их передача в соответствующие списки ожидающих на этажах
	void FindAppearingPassengers();

	/// Нажатие появившимися пассажирами кнопок на этажах
	void PressingFloorButtons();

	/// Уход с этажей пассажиров, отчаившихся дождаться лифта
	void LeavingFloors();

	/// \brief Проверка того, что на этаже присутствует хотя бы один лифт
	/// 
	/// Пассажиры стоящие на этаже не нажинают этажную кнопку, пока на этаже присутствует лифт
	/// \param[in] stage этаж
	/// \return признак присутствия хотя бы одного лифта на данном этаже
	bool isElevatorOnFloor(size_t stage) const;

	/// Буфер строк статистики по пассажирам
	mutable std::vector<std::string> passStatBuffer;

public:
	
	/// Время ожидания до закрытия дверей (если только кто-то не нажмет кнопку "ход" раньше)
	const size_t waitingTime = 5;
	
	/// Время между входами двух пассажиров в лифт
	const size_t timeEntering = 2;
	
	/// Время между выходами двух пассажиров в лифт
	const size_t timeLeaving = 2;

	/// Время открывания дверей
	const size_t timeOpening = 4;

	/// Время закрывания дверей
	const size_t timeClosing = 4;

	/// Время разгона лифта
	const size_t timeAccelerating = 4;
	
	/// Время торможения лифта
	const size_t timeBreaking = 3;

	/// Скорость равномерно движения лифта (в долях этажа)
	const double veloUniform = 0.25;

	/// \brief Инициализирующий конструктор
	/// 
	/// \param[in] numberOfFloors число этажей (считая подвальный)
	/// \param[in] numberOfElevators число лифтовых кабин
	/// \param[in] capacityOfElevator емкость каждой кабины	
	Control(
		size_t numberOfFloors, 
		size_t numberOfElevators, 
		size_t capacityOfElevator);
	

	/// \brief Деструктор
	~Control();


	/// \brief Функция выполнения шага моделирования по времени
	void MakeStep();


	/// \brief Функция запроса текущего времени
	///
	/// \return текущее время в секундах от начала моделирования
	size_t getCurrentTime() const
	{
		return time;
	}


	/// \brief Функция задания назначения лифту
	/// 
	/// \param[in] elevatorNumber номер кабины, которой задается назначение
	/// \param[in] destination этаж назначения 	
	void SetElevatorDestination(size_t elevatorNumber, size_t destination)
	{
		elevators[elevatorNumber]->setDestination(destination);
	}


	/// \brief Функция задания состояния индикатора лифта (лампочка со стрелочкой, которую видят пассажиры)
	/// 
	/// \param[in] elevatorNumber номер кабины, которой задается назначение
	/// \param[in] indicator устанавливаемое значение индикатора (вверх, вниз или в обе стороны) 	
	void SetElevatorIndicator(size_t elevatorNumber, ElevatorIndicator indicator)
	{
		elevators[elevatorNumber]->setIndicator(indicator);
	}


	/// \brief Функция запроса текущего назначения
	///
	/// \param[in] elevatorNumber номер кабины, назначение которой запрашивается
	/// \return этаж назначения соответствующей кабины лифта
	size_t getElevatorDestination(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->getDestination();
	}


	/// \brief Функция запроса текущего состояния индикатора
	///
	/// \param[in] elevatorNumber номер кабины, состояние индикатора которой запрашивается
	/// \return состояние индикатора (вверх, вниз или в обе стороны) соответствующей кабины лифта
	ElevatorIndicator getElevatorIndicator(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->getIndicator();
	}


	/// \brief Проверка того, что лифт завершил выполнение текущего назначения
	/// 
	/// Назначение считается выполненым, когда
	///    - лифт приехал на тот этаж, куда его послали, остановился, и
	///    - выполнено одно из трех условий:
	///        -# в нем есть хотя бы 1 пассажир - тогда открылись двери
	///        -# он пустой, а на этаже, на который он прибыл, нажата хотя бы одна кнопка - тогда тоже открылись двери
	///        -# он пустой, а на этаже, на который он прибыл, не нажато ни одной кнопки - тогда двери не открываются  
	/// 
	/// \param[in] elevatorNumber номер кабины, для которой проверяется данное условие
	/// \return признак выполнения назначения соответствующей кабиной лифта
	bool isElevatorAchievedDestination(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->isDestinationAchieved(floorButtons->getUpButtons(), floorButtons->getDnButtons());
	}


	/// \brief Проверка того, что лифт пустой после выхода очередного пассажира
	///
	/// Возвращает true, если лифт стоит на этаже, и после выхода очередного пассажира
	/// лифт оказался пустым --- возможно, при этом имеет смысл "включить" индикатор в оба направления, 
	/// чтобы в любом случае зашел пассажир, стоящий первым в очереди.
	/// Но это не обязательно - у Вас может быть своя логика!
	/// 
	/// Если индикатор лифта "горит" в состоянии both (в обе стороны), при этом он пустой или нет - не важно, 
	/// и в лифт входит пассажир, то индикатор автоматически переключается в то направление,
	/// какую кнопку он нажал, входя в лифт.
	/// 
	/// Будьте осторожны, "зажигайте" состояние индикатора both (в обе стороны) аккуратно, но и без него обойтись будет трудно!
	/// 
	/// \param[in] elevatorNumber номер кабины, для которой проверяется данное условие
	/// \return признак пустоты соответствующей кабины лифта после выхода очередного пассажира
	bool isElevatorEmptyAfterUnloading(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->isEmptyAfterUnloading();
	}


	/// \brief Проверка того, что кабина лифта пуста
	///
	/// Состояние лифта не проверяется - стоит он или едет, открыты или нет двери
	/// 
	/// \param[in] elevatorNumber номер кабины, для которой проверяется данное условие
	/// \return признак пустоты соответствующей кабины лифта
	bool isElevatorEmpty(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->isEmpty();
	}


	/// \brief Проверка того, что кабина лифта движется вверх
	///
	/// Может быть истиной только при закрытых дверях; едет ли лифт равномерно или с ускорением - не важно
	/// 
	/// \param[in] elevatorNumber номер кабины, для которой проверяется данное условие
	/// \return признак движения вверх соответствующей кабины лифта
	bool isElevatorGoingUp(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->isGoingUp();
	}


	/// \brief Проверка того, что кабина лифта движется вниз
	///
	/// Может быть истинной только при закрытых дверях; едет ли лифт равномерно или с ускорением - не важно
	/// 
	/// \param[in] elevatorNumber номер кабины, для которой проверяется данное условие
	/// \return признак движения вниз соответствующей кабины лифта
	bool isElevatorGoingDn(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->isGoingDn();
	}


	/// \brief Проверка того, что кабина лифта стоит (не движется)
	///
	/// Может быть истиной не только, когда лифт на этаже (при этом состояние дверей не важно), 
	/// но и между этажами, когда лифт, к примеру ехал вверх, но поступило новое назначение: а этом случае он тормозит,
	/// в течение 1 секунды стоит на месте (в этот ммомент данное условие будет выполненным), а потом разгоняется вниз
	/// 
	/// \param[in] elevatorNumber номер кабины, для которой проверяется данное условие
	/// \return признак стояния на месте соответствующей кабины лифта
	bool isElevatorStaying(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->isStaying();
	}


	/// \brief Проверка того, что кабина лифта движется равномерно
	///
	/// Может быть истинной только при закрытых дверях, когда лифт движется равномерно (не разгоняется и не тормозит) 
	/// 
	/// \param[in] elevatorNumber номер кабины, для которой проверяется данное условие
	/// \return признак равномерного движения соответствующей кабины лифта
	bool isElevatorGoingUniformly(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->isGoingUniformly();
	}


	/// \brief Проверка того, что кабина лифта ускоряется (разгоняется)
	///
	/// Может быть истинной только при закрытых дверях, когда лифт движется вверх или вниз с ускорением (разгоном)
	/// 
	/// \param[in] elevatorNumber номер кабины, для которой проверяется данное условие
	/// \return признак движения соответствующей кабины лифта с ускорением (при разгоне)
	bool isElevatorAccelerating(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->isAccelerating();
	}


	/// \brief Проверка того, что кабина лифта замедляется (тормозит)
	///
	/// Может быть истинной только при закрытых дверях, когда лифт движется вверх или вниз с замедлением (тормозит)
	/// 
	/// \param[in] elevatorNumber номер кабины, для которой проверяется данное условие
	/// \return признак движения соответствующей кабины лифта с замедлением (при торможении)
	bool isElevatorBreaking(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->isBreaking();
	}


	/// \brief Проверка того, что у кабины лифта в данный момент открываются двери
	///
	/// Может быть истинной только при нахождении лифта на этаже, когда он не движется
	/// 
	/// \param[in] elevatorNumber номер кабины, для которой проверяется данное условие
	/// \return признак того, что у соответствующей кабины лифта происходит открывание дверей
	bool isElevatorDoorsOpening(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->isDoorsOpening();
	}


	/// \brief Проверка того, что у кабины лифта в данный момент закрываются двери
	///
	/// Может быть истинной только при нахождении лифта на этаже, когда он не движется
	/// 
	/// \param[in] elevatorNumber номер кабины, для которой проверяется данное условие
	/// \return признак того, что у соответствующей кабины лифта происходит закрывание дверей
	bool isElevatorDoorsClosing(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->isDoorsClosing();
	}


	/// \brief Проверка того, что у кабины лифта в данный момент открыты двери
	///
	/// Может быть истинной только при нахождении лифта на этаже, когда он не движется
	/// 
	/// \param[in] elevatorNumber номер кабины, для которой проверяется данное условие
	/// \return признак того, что у соответствующей кабины лифта открыты двери
	bool isElevatorDoorsOpened(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->isDoorsOpened();
	}


	/// \brief Проверка того, что у кабина лифта в данный момент стоит на этаже с закрытыми дверьми
	///
	/// Может быть истинной только при нахождении лифта на этаже
	/// 
	/// \param[in] elevatorNumber номер кабины, для которой проверяется данное условие
	/// \return признак того, что соответствующая кабина лифта стоит на этаже с закрытыми дверьми
	bool isElevatorStayingDoorsClosed(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->isStayingDoorsClosed();
	}


	/// \brief Функция запроса текущего положения лифта
	///
	/// \param[in] elevatorNumber номер кабины, положение которой запрашивается
	/// \return дробное число; если дробная часть нудевая - то лифт на этаже
	double getElevatorPosition(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->getPosition();
	}


	/// \brief Функция запроса состояний кнопок "вверх" на этажах
	///
	/// \return константную ссылка на вектор признаков нажатости кнопок "вверх" на этажах
	const std::vector<bool>& getFloorUpButtons() const
	{
		return floorButtons->getUpButtons();
	}


	/// \brief Функция запроса состояний кнопок "вниз" на этажах
	///
	/// \return константную ссылка на вектор признаков нажатости кнопок "вниз" на этажах
	const std::vector<bool>& getFloorDnButtons() const
	{
		return floorButtons->getDnButtons();
	}


	/// \brief Функция запроса состояния кнопки "вверх" на конкретном этаже
	///
	/// \param[in] floor номер этажа, на котором опрашивается кнопка
	/// \return признак нажатости кнопки "вверх" на соответствующем этаже
	bool getFloorUpButton(size_t floor) const
	{
		return floorButtons->getUpButton(floor);
	}


	/// \brief Функция запроса состояния кнопки "вниз" на конкретном этаже
	///
	/// \param[in] floor номер этажа, на котором опрашивается кнопка
	/// \return признак нажатости кнопки "вниз" на соответствующем этаже
	bool getFloorDnButton(size_t floor) const
	{
		return floorButtons->getDnButton(floor);
	}


	/// \brief Функция сброса (выключения) кнопки "вверх" на конкретном этаже
	///
	/// \param[in] floor номер этажа, на котором сбрасывается (гасится) кнопка
	/// Добавлена на всякий случай; если на этаже есть пассажиры, и они увидят, что нужная им кнопка погасла - то они ее нажмут	
	void unsetUpButton(size_t floor)
	{
		floorButtons->unsetUpButton(floor);
	}


	/// \brief Функция сброса (выключения) кнопки "вниз" на конкретном этаже
	///
	/// \param[in] floor номер этажа, на котором сбрасывается (гасится) кнопка
	/// Добавлена на всякий случай; если на этаже есть пассажиры, и они увидят, что нужная им кнопка погасла - то они ее нажмут
	void unsetDnButton(size_t floor)
	{
		floorButtons->unsetDnButton(floor);
	}


	/// \brief Функция запроса состояний кнопок в кабине лифта
	///
	/// \param[in] elevatorNumber номер кабины, состояние кнопок в которой запрашивается
	/// \return константную ссылку на вектор признаков нажатости кнопок в соответствующей кабине лифта
	const std::vector<bool>& getElevatorButtons(size_t elevatorNumber) const
	{
		return elevators[elevatorNumber]->getButtons();
	}


	/// \brief Функция запроса состояния конкретной кнопки в кабине лифта
	///
	/// \param[in] elevatorNumber номер кабины, состояние кнопки в которой запрашивается
	/// \param[in] floor кнопка, состояние которой запрашивается
	/// \return признак нажатости конкретной кнопки в соответствующей кабине лифта
	bool getElevatorButton(size_t elevatorNumber, size_t floor) const
	{
		return elevators[elevatorNumber]->getButton(floor);
	}

	
	/// \brief Функция добавления пассажира в очередь
	/// 
	/// \param[in] passProp_ константная ссылка на список параметров пассажира	
	void AddPassengerToQueue(const PassengerProperties& passProp_);
	

	/// \brief Функция печати в файл или на экран состояния лифта в текущий момент времени
	///
	/// Если вызывается без аргумента - печать на экран, если с аргументом - печать в файл с данным именем.
	/// 
	/// Если вызывать эту функцию на каждом шаге по времени - получится полный протокол работы кабины лифта
	/// 
	/// \param[in] elevatorNumber номер кабины, состояние котрой печатается
	/// \param[in] fname имя файла, в корорый выводить состояние
	void PrintElevatorState(size_t elevatorNumber, const std::string& fname = "") const;
	

	/// \brief Функция печати в файл или на экран состояния кнопок в кабинах и на этажах в текущий момент времени
	///
	/// Если вызывается без аргумента - печать на экран, если с аргументом - печать в файл с данным именем.
	/// 
	/// Если вызывать эту функцию на каждом шаге по времени - получится полный протокол состояния всех кнопок в кабинах и на этажах
	/// 
	/// \param[in] fname имя файла, в корорый выводить состояние
	void PrintButtonsState(const std::string& fname = "") const;


	/// \brief Функция печати в файл или на экран событий, произошедших с пассажирами за последний шаг (последнюю секунду)  
	///
	/// Если вызывается без аргумента - печать на экран, если с аргументом - печать в файл с данным именем.
	/// 
	/// Выводит сообщения:
	///    - о появлении пассажира на этаже
	///    - о входе пассажира в кабину лифта
	///    - о выходе пассажира из лифта
	///    - о том, что пассажир ушел, не дождавшись лифта
	/// 
	/// \warning Нужна только для отладки. В процессе работы системы управнения эта информация недоступна и не может быть использована!
	/// 
	/// \param[in] fname имя файла, в корорый выводить состояние
	void PrintPassengerState(const std::string& fname = "") const;


	/// \brief Функция печати в файл или на экран итоговой статистики, включая итоговый "рейтинг" (чем меньше - тем лучше!)
	///  
	/// \param[in] passengersDetails признак печати статистики по каждому пассажиру
	/// \param[in] fname имя файла, в корорый выводить состояние
	void PrintStatistics(bool passengersDetails, const std::string& fname = "") const;

};

