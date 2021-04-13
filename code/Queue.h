// Проект elevator 
// (c) И.К. Марчевский, 2021

/*!
\file
\brief Заголовочный файл с описанием класса Queue
\author Марчевский Илья Константинович
\version 0.4
\date 14 апреля 2021 г.
*/

#pragma once

#include <vector>

#include "Passenger.h"

/*!
\brief Класс --- очередь пассажиров 
*/
class Queue
{
	friend class Control;

private:
	std::vector<Passenger> passengers;               
	std::vector<std::vector<Passenger>> passOnFloor; 
	std::vector<Passenger> finished;

public:
	/// \brief Инициализирующий конструктор
	/// 
	/// \param[in] numberOfFloors_ число этажей, включая подвал
	Queue(size_t numberOfFloors_)
		: passOnFloor(numberOfFloors_)
	{};

	/// \brief Функция добавки пассажира в очередь
	///
	/// \param[in] passProp_ константная ссылка на параметры добавляемого пассажира
	void addPassenger(const PassengerProperties& passProp_)
	{
		passengers.emplace_back(passengers.size(), passProp_);
	}
};

