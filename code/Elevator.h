// Проект elevator 
// (c) И.К. Марчевский, 2021-2022

/*!
\file
\brief Заголовочный файл с описанием класса Elevator и сопутствующих структур
\author Марчевский Илья Константинович
\version 0.7
\date 20 февраля 2022 г.
*/

#pragma once

#include <string>
#include <vector>

#include "Passenger.h"

/*! \enum ElevatorIndicator
 \brief Состояния лампочки (индикатора направления движения) кабины лифта

 Индикатор (лампочка со стрелочкой) "зажигается" Вами при помощи системы управления. 
 Пассажиры в кабину садятся только те, которые едут в указанном стрелочкой 
 (или двумя стрелочками, если зажечь состояние "both") направлении. 

 Хотя бывают пассажиры, которые не смотрят на стрелочки и садятся в лифт, едущий не туда 
 (в симуляции тоже такие будут встречаться, причем вероятность их появления повышается с увеличением времени ожидания на этаже!)

 Если индикатор лифта "горит" в состоянии both (он пустой или нет - не важно),
 и в лифт входит пассажир, то индикатор автоматически переключается в то направление,
 какую кнопку он нажал, входя в лифт.
 
 Если ваша система управления работает "криво", то возможна ситуация, что стрелочка будет гореть вверх, 
 а реально лифт поедет вниз. Едущие вниз при этом в него не сядут.
*/
enum class ElevatorIndicator {
	both,       ///< стрелочки в обоих направлениях
	up,         ///< стрелочка вверх
	down,       ///< стрелочка вниз
};


/*! \enum ElevatorIndicator
 \brief Состояние лифта (не может быть использовано напрямую в системе управления, внутренний параметр)
*/
enum class ElevatorStatus {
	staying,   ///< стоит (не движется)
	movingUp,  ///< движется вверх
	movingDn,  ///< движется вниз
};


/*! \enum ElevatorAcceleration
 \brief Ускорение лифта (не может быть использовано напрямую в системе управления, внутренний параметр)
*/
enum class ElevatorAcceleration{
	breaking,      ///< замедляется (тормозит)
	accelerating,  ///< ускоряется (разгоняется)
	uniform,       ///< движется равномерно или стоит
};


/*! \enum ElevatorDoorsStatus
 \brief Состояние дверей кабины лифта (не может быть использовано напрямую в системе управления, внутренний параметр)
*/
enum class ElevatorDoorsStatus {
	opening,          ///< открываются
	openedUnloading,  ///< открыты (идет высадка пассажиров)
	openedLoading,    ///< открыты (идет посадка пассажиров)
	closing,          ///< закрываются
	closed,			  ///< закрыты
	waiting,          ///< открыты (лифт ожидает отправления)
};


/*!
\brief Класс --- кабина лифта
*/
class Elevator
{	
	friend class Control;

private:	
	/// Вектор из пассажиров в лифте
	std::vector<Passenger> passengers;

	/// Множество нажатых кнопок
	std::vector<bool> buttons;
	
	/// Максимальная вместимость
	const size_t capacity;

	/// Индикатор движения лифта
	ElevatorIndicator indicator;

	/// Текущее положение лифта - номер этажа, умноженный на 100
	size_t position;

	/// Текущее назначение лифта
	size_t destinationFloor;

	/// id лифта
	size_t myid;

	/// Cтатус лифта (направление движения или признак остановки)
	ElevatorStatus status;

	/// Ускорение лифта 
	ElevatorAcceleration acceleration;

	/// Состояние дверей лифта
	ElevatorDoorsStatus doorsStatus;


	void setIndicator(ElevatorIndicator indicator);

	ElevatorIndicator getIndicator() const
	{
		return indicator;
	}

	bool isGoingButtonPressed() const;

	size_t lastChechedPassenger;
	size_t timeToSelfProgramme;

	size_t getNumberOfPassengers() const
	{
		return passengers.size();
	}

	std::string getStateString() const;

	bool isEmpty() const
	{
		return (passengers.size() == 0);
	}

	bool isEmptyAfterUnloading() const
	{
		return ((doorsStatus == ElevatorDoorsStatus::openedUnloading) && (isEmpty()));
	}

	bool isDestinationAchieved(const std::vector<bool>& upButtons, const std::vector<bool>& dnButtons) const;

	bool isStaying() const
	{
		return (status == ElevatorStatus::staying);
	}

	bool isGoingUp() const
	{
		return (status == ElevatorStatus::movingUp);
	}

	bool isGoingDn() const
	{
		return (status == ElevatorStatus::movingDn);
	}

	bool isGoingUniformly() const
	{
		return ((status == ElevatorStatus::movingDn || status == ElevatorStatus::movingUp) && (acceleration == ElevatorAcceleration::uniform)); 
	}

	bool isAccelerating() const
	{
		return ((status == ElevatorStatus::movingDn || status == ElevatorStatus::movingUp) && (acceleration == ElevatorAcceleration::accelerating));
	}

	bool isBreaking() const
	{
		return ((status == ElevatorStatus::movingDn || status == ElevatorStatus::movingUp) && (acceleration == ElevatorAcceleration::breaking));
	}

	bool isDoorsOpening() const
	{
		return ((status == ElevatorStatus::staying) && (doorsStatus == ElevatorDoorsStatus::opening));
	}

	bool isDoorsClosing() const
	{
		return ((status == ElevatorStatus::staying) && (doorsStatus == ElevatorDoorsStatus::closing));
	}

	bool isDoorsOpened() const
	{
		return ((status == ElevatorStatus::staying) && (doorsStatus == ElevatorDoorsStatus::openedLoading || doorsStatus == ElevatorDoorsStatus::openedUnloading || doorsStatus == ElevatorDoorsStatus::waiting));
	}

	bool isStayingDoorsClosed() const
	{
		return ((status == ElevatorStatus::staying) && ((position % 100) == 0) && (doorsStatus == ElevatorDoorsStatus::closed));
	}

	double getPosition() const
	{
		return position / 100 + 0.01 * (position % 100);
	}

	void setDestination(size_t i);
	
	size_t getDestination() const
	{
		return destinationFloor;
	}

	const std::vector<bool>& getButtons() const
	{
		return buttons;
	}

	bool getButton(size_t i) const
	{
		return buttons[i];
	}


public:
	/// \brief Инициализирующий конструктор
	/// 	
	/// \param[in] numberOfFloors количество этажей (считая подвал)
	/// \param[in] maxCapacity максимальная вместимость кабины лифта
	/// \param[in] id порядковый номер кабины лифта
	Elevator(size_t numberOfFloors, size_t maxCapacity, size_t id);
};

