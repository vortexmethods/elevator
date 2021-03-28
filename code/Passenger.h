// Проект elevator 
// (c) И.К. Марчевский, 2021

/*!
\file
\brief Заголовочный файл с описанием класса Passenger и сопутствующих структур
\author Марчевский Илья Константинович
\version 0.3
\date 29 марта 2021 г.
*/

#pragma once

/*! \enum PassengerStatus
 \brief Статусы пассажиров (не может быть использовано напрямую в системе управления, внутренний параметр)
*/
enum class PassengerStatus { 	
	waiting,  ///< Пассажир ждет лифта на этаже отправления
	going,    ///< Пассажир нажодится в лифте	
	arrived,  ///< Пассажир успешно прибыл на нужный этаж
	leaved    ///< Пассажир не дождался лифта и ушел
};

/*! \struct PassengerProperties
 \brief Параметры пассажиров
*/
struct PassengerProperties
{
	/// Время появления пассажира на этаже
	size_t timeInit;

	/// Этаж, с которого пассажир отправляется
	size_t floorDeparture;

	/// Этаж, на который пассажир едет
	size_t floorDestination;

	/// Время ожидания, после которого пассажир уходит
	size_t criticalWaitTime;

	/// Вероятность сесть в лифт, едущий в неверном направлении в начале ожидания
	double pInverseStartWaiting;

	/// Вероятность сесть в лифт, едущий в неверном направлении в конце ожидания
	double pInverseStopWaiting;

	/// Вероятность нажать кнопку "ход", не дожидаясь закрытия дверей
	double pStartGoing;
};

/*!
\brief Класс --- пассажир
*/
class Passenger
{
	friend class Control;	
	friend class Elevator;

private:
	/// Порядковый номер
	size_t id;
		
	/// Характеристики человека
	PassengerProperties properties;

	/// Время посадки пассажира в лифт
	size_t timeStart;

	/// Время прибытия пассажира на требуемый этаж
	size_t timeFinish;

	/// Статус пассажира
	PassengerStatus status;

	size_t getTimeInit() const { return properties.timeInit; };
	size_t getFloorDeparture() const { return properties.floorDeparture; };
	size_t getFloorDestination() const { return properties.floorDestination; };
	bool PerformInverseProbability(size_t curTime) const
	{
		const PassengerProperties& pr = properties;

		double curProb = pr.pInverseStartWaiting \
			+ (double)(curTime - pr.timeInit) / pr.criticalWaitTime * (pr.pInverseStopWaiting - pr.pInverseStartWaiting);

		double rnd = (double)rand() / (RAND_MAX + 1);

		return (rnd < curProb);
	};

	bool PerformNoWaitingProbability() const
	{
		const PassengerProperties& pr = properties;
		double rnd = (double)rand() / (RAND_MAX + 1);
		return (rnd < pr.pStartGoing);
	}

public:
	/// \brief Инициализирующий конструктор
	///
	/// \param[in] id_ порядковый номер пассажира
	/// \param[in] properties_ параметры пассажира
	Passenger(size_t id_, const PassengerProperties& properties_)
		: id(id_), properties(properties_), timeStart(-1), timeFinish(-1), status(PassengerStatus::waiting) 
		{};

	/// \brief Оператор сравнения для возможности сортировки пассажиров по порядковому номеру
	bool operator<(const Passenger& ps) const
	{
		return id < ps.id;
	}
};

