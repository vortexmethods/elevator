// Проект elevator 
// (c) И.К. Марчевский, 2021

/*!
\file
\brief Файл кода с описанием класса Control
\author Марчевский Илья Константинович
\version 0.6
\date 29 мая 2021 г.
*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>


#include "Control.h"
#include "Queue.h"


Control::Control(size_t numberOfFloors, size_t numberOfElevators, size_t capacityOfElevator)
	: floorButtons(new FloorButtons(numberOfFloors)), queue(new Queue(numberOfFloors)), time(0)
{
	for (size_t id = 0; id < numberOfElevators; ++id)
		elevators.emplace_back(new Elevator(numberOfFloors, capacityOfElevator, id));
}//Control(...)


Control::~Control()
{
}//~Control()


void Control::TimeIncrement()
{
	++time;
}//TimeIncrement()


void Control::FindAppearingPassengers()
{
	auto newEnd = std::partition(queue->passengers.begin(), queue->passengers.end(), \
		[=](const Passenger& p) {return p.getTimeInit() != getCurrentTime(); });

	for (auto it = newEnd; it != queue->passengers.end(); ++it)
	{
		queue->passOnFloor[it->getFloorDeparture()].push_back(*it);
		passStatBuffer.push_back("time = " + std::to_string(getCurrentTime()) \
			+ "\tPassenger #" + std::to_string(it->id) \
			+ "\tappeared on floor #" + std::to_string(it->getFloorDeparture()) \
			+ ", goes to floor #" + std::to_string(it->getFloorDestination()));
	}//for it

	queue->passengers.erase(newEnd, queue->passengers.end());
}//FindAppearingPassengers()


void Control::PressingFloorButtons()
{
	for (size_t floor = 0; floor < queue->passOnFloor.size(); ++floor)
	{		
		for (auto& p : queue->passOnFloor[floor])
			if (!isElevatorOnFloor(floor))
			{
				if (p.getFloorDestination() > p.getFloorDeparture())
					floorButtons->setUpButton(p.getFloorDeparture());
				else
					floorButtons->setDnButton(p.getFloorDeparture());
			}//if !isElevatorOnFloor
	}//for floor
}//PressingFloorButtons()


void Control::LeavingFloors()
{
	for (size_t floor = 0; floor < queue->passOnFloor.size(); ++floor)
	{
		std::vector<Passenger> continueWaiting;

		for (auto& p : queue->passOnFloor[floor])
		{
			if (getCurrentTime() - p.getTimeInit() < p.properties.criticalWaitTime)
				continueWaiting.push_back(p);
			else
			{
				p.status = PassengerStatus::leaved;
				queue->finished.push_back(p);
				passStatBuffer.push_back(\
					"time = " + std::to_string(getCurrentTime()) \
					+ "\tPassenger #" + std::to_string(p.id) \
					+ "\tfrom floor #" + std::to_string(p.getFloorDeparture()) \
					+ " to floor #" + std::to_string(p.getFloorDestination()) \
					+ "\tleaved the floor (waiting time = " + std::to_string(getCurrentTime() - p.getTimeInit()) + ")");
			}
		}
		queue->passOnFloor[floor] = std::move(continueWaiting);

	}//for floor
}

void Control::MakeStep()
{
	passStatBuffer.resize(0);

	TimeIncrement();

	//Проверка появления пассажиров на этажах и их передача 
	//в соответствующие списки ожидающих на этажах
	FindAppearingPassengers();

	//Нажатие появившимися пассажирами кнопок на этажах	
	PressingFloorButtons();

	//Пассажиры, ждавшие слишком долго, уходят с этажей, и за это начисляется большой штраф
	LeavingFloors();

	//Посадка пассажиров в лифты на этажах
	//цикл по этажам:
	for (size_t pos = 0; pos < floorButtons->dnButtons.size(); ++pos)
	{
		//std::vector<ElevatorIndicator> ind = { ElevatorIndicator::up, ElevatorIndicator::down };
		
		//for (auto& indValue : ind)
		//{
			std::vector<Elevator*> elevOnFloor;
			for (auto& e : elevators)
			{
				if ((e->position / 100 == pos) && (e->doorsStatus == ElevatorDoorsStatus::openedLoading)) 
					if (e->getNumberOfPassengers() < e->capacity)
						elevOnFloor.push_back(e.get());
					else 
					{
						if (e->timeToSelfProgramme == 0)
						{
							e->timeToSelfProgramme = waitingTime;
							e->doorsStatus = ElevatorDoorsStatus::waiting;
						}
					}
			}

			if (elevOnFloor.size() > 1)
				for (auto& pe : elevOnFloor)
					pe->timeToSelfProgramme = elevOnFloor[0]->timeToSelfProgramme;

			if (elevOnFloor.size() > 0)
				if (elevOnFloor[0]->timeToSelfProgramme == 0)
				{					
					auto& pass = queue->passOnFloor[pos];

					size_t passToUp = 0, passToDn = 0;
					for (auto& p : pass)
					{
						if (p.getFloorDestination() > pos)
							++passToUp;
						else
							++passToDn;
					}

					if (passToUp == 0)
						for (auto& e : elevOnFloor)
							if (e->getIndicator() == ElevatorIndicator::up)
							{
								e->timeToSelfProgramme = waitingTime;
								e->doorsStatus = ElevatorDoorsStatus::waiting;
							};

					if (passToDn == 0)
						for (auto& e : elevOnFloor)
							if (e->getIndicator() == ElevatorIndicator::down)
							{
								e->timeToSelfProgramme = waitingTime;
								e->doorsStatus = ElevatorDoorsStatus::waiting;
							};

					if ((passToUp == 0) && (passToDn == 0))
						for (auto& e : elevOnFloor)
							if (e->getIndicator() == ElevatorIndicator::both)
							{
								e->timeToSelfProgramme = waitingTime;
								e->doorsStatus = ElevatorDoorsStatus::waiting;
							};

					
					std::vector<Passenger> stillWaiting;

					for (auto& p : pass)
					{

						std::vector<Elevator*> elevAppropriate;
						bool inverseWay = false;

						bool inv = p.PerformInverseProbability(getCurrentTime());

						for (auto& e : elevOnFloor)
						{
							if ((p.getFloorDestination() > pos) && (e->timeToSelfProgramme == 0) && (e->doorsStatus == ElevatorDoorsStatus::openedLoading) && (e->indicator == ElevatorIndicator::up || e->indicator == ElevatorIndicator::both))
							{
								e->lastChechedPassenger = std::max(p.id, e->lastChechedPassenger);
								
								if (e->getNumberOfPassengers() < e->capacity)
									elevAppropriate.push_back(e);									
								else
								{
									e->timeToSelfProgramme = waitingTime;
									e->doorsStatus = ElevatorDoorsStatus::waiting;
								}
							}
							
							if ((p.getFloorDestination() < pos) && (e->timeToSelfProgramme == 0) && (e->doorsStatus == ElevatorDoorsStatus::openedLoading) && (e->indicator == ElevatorIndicator::down || e->indicator == ElevatorIndicator::both))
							{
								e->lastChechedPassenger = std::max(p.id, e->lastChechedPassenger);

								if (e->getNumberOfPassengers() < e->capacity)
									elevAppropriate.push_back(e);
								else
								{
									e->timeToSelfProgramme = waitingTime;
									e->doorsStatus = ElevatorDoorsStatus::waiting;
								}
							}
						}

						//Если человек готов сесть не туда
						if ((elevAppropriate.size() == 0) && (inv))
						{
							
							for (auto& e : elevOnFloor)
							{																
								if ((e->timeToSelfProgramme == 0) && (e->doorsStatus == ElevatorDoorsStatus::openedLoading))
								{
									e->lastChechedPassenger = std::max(p.id, e->lastChechedPassenger);

									if (e->lastChechedPassenger <= p.id)
									{
										if (e->getNumberOfPassengers() < e->capacity)
										{
											elevAppropriate.push_back(e);
											inverseWay = true;
										}
										else
										{
											e->timeToSelfProgramme = waitingTime;
											e->doorsStatus = ElevatorDoorsStatus::waiting;
										}
									}
								}
							}
						}


						if (elevAppropriate.size() > 0)
						{
							size_t elevWithSmallestPass = 0;
							size_t smallestPass = elevAppropriate[0]->getNumberOfPassengers();

							for (size_t numb = 1; numb < elevAppropriate.size(); ++numb)
								if (elevAppropriate[numb]->getNumberOfPassengers() < smallestPass)
								{
									elevWithSmallestPass = numb;
									smallestPass = elevAppropriate[numb]->getNumberOfPassengers();
								}
								

							Elevator* e = elevAppropriate[elevWithSmallestPass];
							
							e->passengers.push_back(p);
							e->passengers.back().status = PassengerStatus::going;
							e->passengers.back().timeStart = getCurrentTime();
							passStatBuffer.push_back("time = " + std::to_string(getCurrentTime()) \
								+ "\tPassenger #" + std::to_string(e->passengers.back().id) \
								+ "\tfrom floor #" + std::to_string(e->passengers.back().getFloorDeparture()) \
								+ " to floor #" + std::to_string(e->passengers.back().getFloorDestination()) \
								+ (inverseWay ? "*" : "") \
								+ "\tentered the elevator #" + std::to_string(e->myid)); 
							e->timeToSelfProgramme = timeEntering;
							e->buttons[e->passengers.back().getFloorDestination()] = true;
							if (e->indicator == ElevatorIndicator::both)
							{
								if (e->passengers.back().properties.floorDestination > pos)
									e->indicator = ElevatorIndicator::up;
								else
									e->indicator = ElevatorIndicator::down;
							}
								
						}
						else
							stillWaiting.push_back(p);

					}//for p

					pass.clear();
					pass = std::move(stillWaiting);

				}
		//}//for indValue
	}//for pos


	//Обработка нажатия кнопки "Ход"
	for (auto& e : elevators)
	{
		if (e->doorsStatus == ElevatorDoorsStatus::waiting)
			if (e->isGoingButtonPressed() && (e->timeToSelfProgramme > 1))
			{
				e->timeToSelfProgramme = 1;
			}
	}

	//Обработка движения лифта
	for (auto& e : elevators)
	{
		if (e->timeToSelfProgramme == 0)
		{
			auto pos = e->position / 100;
			auto& pass = queue->passOnFloor[pos];

			switch (e->status)
			{
			case ElevatorStatus::staying:
			{
				//3.1. Обрабатываем стоящий лифт
				switch (e->doorsStatus)
				{
				case ElevatorDoorsStatus::openedUnloading:
				{
					e->timeToSelfProgramme = timeLeaving - 1;
					auto it = std::find_if(e->passengers.begin(), e->passengers.end(), \
						[=](const Passenger& p) {return p.getFloorDestination() == pos; });
					if (it != e->passengers.end())
					{
						it->status = PassengerStatus::arrived;
						it->timeFinish = getCurrentTime();
						queue->finished.push_back(*it);
						passStatBuffer.push_back("time = " + std::to_string(getCurrentTime()) \
							+ "\tPassenger #" + std::to_string(it->id) \
							+ "\tfrom floor #" + std::to_string(it->getFloorDeparture()) \
							+ " to floor #" + std::to_string(it->getFloorDestination()) \
							+ "\tgot off the elevator #" + std::to_string(e->myid) \
							+ " (appeared t = " + std::to_string(it->properties.timeInit) \
							+ ", in elevator t = " + std::to_string(it->timeStart) + ")");
						e->passengers.erase(it);
						break;
					}// if it!=
										
					e->doorsStatus = ElevatorDoorsStatus::openedLoading;
					e->lastChechedPassenger = 0;

					break;
				}//case ElevatorDoorsStatus::openedUnloading:

				case ElevatorDoorsStatus::waiting:
				{
					e->doorsStatus = ElevatorDoorsStatus::closing;
					e->timeToSelfProgramme = timeClosing - 1;
					break;
				}//case ElevatorDoorsStatus::waiting:

				case ElevatorDoorsStatus::closing:
				{
					//Только что закрывший двери лифт - делаем двери закрытыми
					e->doorsStatus = ElevatorDoorsStatus::closed;
					break;
				}//case ElevatorDoorsStatus::closing:

				case ElevatorDoorsStatus::closed:
				{
					//Если лифт стоит на этаже и появляется человек снаружи //29.05.2021
					if ( ((e->position % 100) == 0 ) && (e->isEmpty()) )
					{
						if (queue->passOnFloor[e->position / 100].size() > 0)
						{
							bool isPassUp = false, isPassDn = false;
							for (auto& p : queue->passOnFloor[e->position / 100])
							{
								if (p.getFloorDestination() > pos)
									isPassUp = true;
								if (p.getFloorDestination() < pos)
									isPassDn = true;
							}
							
							if ((e->indicator == ElevatorIndicator::both)
								|| (e->indicator == ElevatorIndicator::up && isPassUp)
								|| (e->indicator == ElevatorIndicator::down && isPassDn))
							{
								e->doorsStatus = ElevatorDoorsStatus::opening;
								e->buttons[pos] = false;
								e->timeToSelfProgramme = timeOpening - 1;
								break;
							}
							
						}
					}
					
					//Если есть назначение для стоящего лифта с закрытыми дверьми - разгоняем

					//вверх
					if ((e->position / 100) < e->destinationFloor)
					{
						e->status = ElevatorStatus::movingUp;
						e->acceleration = ElevatorAcceleration::accelerating;
						e->timeToSelfProgramme = timeAccelerating - 1;
					}//if ((e->position...

					//вниз
					if (((e->position + 99) / 100) > e->destinationFloor)
					{
						e->status = ElevatorStatus::movingDn;
						e->acceleration = ElevatorAcceleration::accelerating;
						e->timeToSelfProgramme = timeAccelerating - 1;
					}//if (((e->position...

					//3.1.4. Если лифт прибыл в пункт назначения - открываем двери
					if (((e->position / 100) == e->destinationFloor) &&
						((e->position % 100) == 0))
					{
						//но открываем двери только если либо он непустой, либо снаружи нажата кнопка:
						if ((e->getNumberOfPassengers() > 0) ||
							(floorButtons->getDnButton(e->destinationFloor)) ||
							(floorButtons->getUpButton(e->destinationFloor))) 							
						{
							e->doorsStatus = ElevatorDoorsStatus::opening;
							e->buttons[pos] = false;
							e->timeToSelfProgramme = timeOpening - 1;

							if ((floorButtons->getDnButton(e->destinationFloor)) &&
								(e->indicator == ElevatorIndicator::down || e->indicator == ElevatorIndicator::both))
								floorButtons->unsetDnButton(e->destinationFloor);

							if ((floorButtons->getUpButton(e->destinationFloor)) &&
								(e->indicator == ElevatorIndicator::up || e->indicator == ElevatorIndicator::both))
								floorButtons->unsetUpButton(e->destinationFloor);
						}//if ((e->getNumberOfPassengers() > 0) ||...
					}//if (((e->position...										
					break;
				}//case ElevatorDoorsStatus::closed:

				case ElevatorDoorsStatus::opening:
				{
					//Только что открывший двери лифт - делаем двери открытыми
					if (e->isEmpty()) //29.05.2021
						e->doorsStatus = ElevatorDoorsStatus::openedLoading;
					else
						e->doorsStatus = ElevatorDoorsStatus::openedUnloading;
					break;
				}

				}//switch (e->doorsStatus)
				
				break;
			}// case ElevatorStatus::staying


			case ElevatorStatus::movingUp:
			case ElevatorStatus::movingDn:
			{
				switch (e->acceleration)
				{
				case ElevatorAcceleration::breaking:
				{
					e->acceleration = ElevatorAcceleration::uniform;
					e->status = ElevatorStatus::staying;
					break;
				}//case ElevatorAcceleration::breaking:

				case ElevatorAcceleration::accelerating:
				{
					int sign = (e->status == ElevatorStatus::movingUp) ? 1 : -1;
					e->position += sign * (int)(100*veloUniform);
					e->acceleration = ElevatorAcceleration::uniform;
					break;
				}//case ElevatorAcceleration::accelerating:

				case ElevatorAcceleration::uniform:
				{
					int sign = (e->status == ElevatorStatus::movingUp) ? 1 : -1;
					
					bool critDir = ((100 * (int)e->destinationFloor - (int)e->position) * sign) > 0;
					if (critDir)
					{
						if (abs((int)((e->position - 100 * e->destinationFloor))) != (int)(100 * veloUniform))

							e->position += sign * (int)(100 * veloUniform);
						else
						{
							e->acceleration = ElevatorAcceleration::breaking;
							e->position += sign * 12;
							e->timeToSelfProgramme = timeBreaking - 1;
						}//else
					}
					else
					{
						e->acceleration = ElevatorAcceleration::breaking;
						e->position += sign * 12;
						e->timeToSelfProgramme = timeBreaking - 1;
					}
				}//case ElevatorAcceleration::uniform:

				}//switch (e->acceleration)

				break;
			}//case ElevatorStatus::movingUp:                    
			 //case ElevatorStatus::movingDn:
			}
			

		}//if (e->timeToSelfProgramme == 0)
		else // если продолжается предыдущая операция
		{
			if ((e->status == ElevatorStatus::movingUp) ||
				(e->status == ElevatorStatus::movingDn))
			{
				int sign = (e->status == ElevatorStatus::movingUp) ? 1 : -1;
				
				if (e->acceleration == ElevatorAcceleration::accelerating)
				{
					switch (e->timeToSelfProgramme)
					{
					case 3:
						e->position += sign * 5;
						break;

					case 2:
						e->position += sign * 8;
						break;

					case 1:
						e->position += sign * 12;
						break;
					}//switch (e->timeToSelfProgramme)
				}//if (e->acceleration == ElevatorAcceleration::accelerating)
				
				if (e->acceleration == ElevatorAcceleration::breaking)
				{
					switch (e->timeToSelfProgramme)
					{
					case 2:
						e->position += sign * 8;
						break;

					case 1:
						e->position += sign * 5;
						break;
					}//switch (e->timeToSelfProgramme)
				}//if (e->acceleration == ElevatorAcceleration::breaking)
			}//if ((e->status == ElevatorStatus::movingUp) || (e->status == ElevatorStatus::movingDn))

			--(e->timeToSelfProgramme);
		}//else
	}//for e : elevators
}//MakeStep()


void Control::AddPassengerToQueue(const PassengerProperties& passProp_)
{
	queue->addPassenger(passProp_);
}//AddPassengerToQueue(...)

void Control::ReadTimeTable(const std::string& fileName_)
{
	std::ifstream fi(fileName_);

	char str[255];
	fi.getline(str, 100, '\n');
	
	PassengerProperties passProp;
	int N;	
	
	while (!fi.eof())
	{
		fi >> N;
		fi.get();
		fi >> passProp.timeInit;
		fi.get();
		fi >> passProp.floorDeparture;
		fi.get();
		fi >> passProp.floorDestination;
		fi.get();
		fi >> passProp.criticalWaitTime;
		fi.get();
		fi >> passProp.pInverseStartWaiting;
		fi.get();
		fi >> passProp.pInverseStopWaiting;
		fi.get();
		fi >> passProp.pStartGoing;

		queue->addPassenger(passProp);
	}

	fi.close();
	fi.clear();
}


void Control::PrintElevatorState(size_t i, const std::string& fname) const
{
	std::ofstream fout;
	if (fname != "")
	{
		if (getCurrentTime() <= 1)
			fout.open(fname);
		else
			fout.open(fname, std::ios_base::app);
	}//if (fname != "")
	
	std::ostream& str = (fname == "") ? std::cout : fout;

/*str << "time = " << getCurrentTime() << ", \telev[" << i << "]: " \
		<< elevators[i]->getStateString() << std::endl;*/

	printf("\033[01;37m");
	str << "time = " << getCurrentTime() << ", \telev[";
	switch (i) {
	case 0: printf("\033[01;34m"); break;
	case 1: printf("\033[01;32m"); break;
	case 2: printf("\033[22;31m"); break;
	case 3: printf("\033[01;35m"); break;
	default: printf("\033[22;33m");
	}
	str << i;
	printf("\033[01;37m");
	str << "]: ";

	//begin
	//std::string strState;

	//std::string strStatus;

	str << "level = ";
	str << std::to_string(elevators[i]->position / 100);
	str << ".";
	if ((elevators[i]->position % 100) < 10)
		str << "0";
	str << std::to_string(elevators[i]->position % 100);

	str << ", dir. = ";

	switch (elevators[i]->status)
	{
	case ElevatorStatus::movingUp:
		printf("\033[01;32m");
		str << "up,  ";
		break;
	case ElevatorStatus::movingDn:
		printf("\033[01;34m");
		str << "dn,  ";
		break;
	case ElevatorStatus::staying:
		printf("\033[01;33m");
		str << "stay,";
		break;
	}//switch (status)

	printf("\033[01;37m");
	str << " ind. = ";

	//std::string strIndicator;
	switch (elevators[i]->indicator)
	{
	case ElevatorIndicator::up:
		printf("\033[01;32m");
		str << "up,  ";
		break;
	case ElevatorIndicator::down:
		printf("\033[01;34m");
		str << "down,";
		break;
	case ElevatorIndicator::both:
		printf("\033[01;33m");
		str << "both,";
		break;
	}// switch (indicator)

	printf("\033[01;37m");
	str << " acceler. = ";

	//std::string strAccel;
	switch (elevators[i]->acceleration)
	{
	case ElevatorAcceleration::accelerating:
		printf("\033[22;32m");
		str << "acceler.,";
		break;
	case ElevatorAcceleration::breaking:
		printf("\033[22;31m");
		str << "breaking,";
		break;
	case  ElevatorAcceleration::uniform:
		printf("\033[22;33m");
		str << "uniform, ";
		break;
	}//switch (acceleration)

	printf("\033[01;37m");
	str << " doors = ";

	//std::string strDoors;
	switch (elevators[i]->doorsStatus)
	{
	case ElevatorDoorsStatus::openedUnloading:
		printf("\033[01;31m");
		str << "unloading...";
		break;
	case ElevatorDoorsStatus::openedLoading:
		printf("\033[01;31m");
		str << "loading...  ";
		break;
	case ElevatorDoorsStatus::opening:
		printf("\033[01;33m");
		str << "opening...  ";
		break;
	case ElevatorDoorsStatus::closing:
		printf("\033[01;33m");
		str << "closing...  ";
		break;
	case ElevatorDoorsStatus::closed:
		printf("\033[22;36m");
		str << "closed      ";
		break;
	case ElevatorDoorsStatus::waiting:
		printf("\033[01;31m");
		str << "waiting...  ";
		break;
	}//switch (doorsStatus)

	printf("\033[01;37m");
	str << " (pass.: ";
	for (auto& p : elevators[i]->passengers)
	{
		str << std::to_string(p.id);
		str << ", ";
	}//for p
	str << ")" << std::endl;
	//end


	if (fname != "")
		fout.close();
}//PrintElevatorState(...)


void Control::PrintButtonsState(const std::string& fname) const
{
	std::ofstream fout;
	if (fname != "")
	{
		if (getCurrentTime() <= 1)
			fout.open(fname);
		else
			fout.open(fname, std::ios_base::app);
	}//if (fname != "")

	std::ostream& str = (fname == "") ? std::cout : fout;

	str << "time = " << getCurrentTime() << ": " << std::endl;
	for (auto& e : elevators)
	{
		str << " in elevator #" << e->myid << ": ";
		for (size_t i = 0; i < e->buttons.size(); ++i)
			if (e->getButton(i))
				str << i << " ";
		str << std::endl;
	}//for e
	str << " on floors: ";
	for (size_t i = 0; i < floorButtons->upButtons.size(); ++i)
	{
		if (floorButtons->getUpButton(i) || floorButtons->getDnButton(i))
		{
			str << "#" << i << "(";
			if (floorButtons->getUpButton(i)) {
				printf("\033[01;32m");
				str << "up ";
			}
			if (floorButtons->getDnButton(i)) {
				printf("\033[01;34m");
				str << "dn";
			}
			printf("\033[01;37m");
			str << ")  ";
		}//if (floorButtons->...
	}//for i
	str << std::endl << std::endl;

	if (fname != "")
		fout.close();
}//PrintButtonsState(...)


void Control::PrintPassengerState(const std::string& fname) const
{
	std::ofstream fout;
	if (fname != "")
	{
		if (getCurrentTime() <= 1)
			fout.open(fname);
		else
			fout.open(fname, std::ios_base::app);
	}//if (fname != "")

	std::ostream& str = (fname == "") ? std::cout : fout;
	printf("\033[01;33m");
	if (passStatBuffer.size() > 0)
		for (auto& st : passStatBuffer)
			str << st << std::endl;

	if (fname != "")
		fout.close();
	printf("\033[01;37m");
}//PrintPassengerState(...)


bool Control::isElevatorOnFloor(size_t stage) const
{	
	for (auto& e : elevators)
		if ((e->position / 100.0 == stage) && (e->position % 100 == 0))
			return true;
	return false;
}//isElevatorOnFloor(...)


void Control::PrintStatistics(bool passengersDetails, const std::string& fname) const
{
	std::ofstream fout;
	if (fname != "")
		fout.open(fname);	
	
	std::ostream& str = (fname == "") ? std::cout : fout;

	std::vector<Passenger> allPass(queue->passengers);
	for (auto& pf : queue->passOnFloor)
		std::copy(pf.begin(), pf.end(), std::back_inserter(allPass));			
	for (auto& e : elevators)
		std::copy(e->passengers.begin(), e->passengers.end(), std::back_inserter(allPass));
	std::copy(queue->finished.begin(), queue->finished.end(), std::back_inserter(allPass));
	
	std::sort(allPass.begin(), allPass.end());

	size_t numInElevetor = 0, numOnFloors = 0, numLeaved = 0;
	size_t penaltyFinished = 0, penaltyInElevetor = 0, penaltyOnFloors = 0, penaltyLeaved = 0;

	if (passengersDetails)
		str << "Passangers:" << std::endl;

	for (auto& p : allPass)
	{
		switch (p.status)
		{
		case PassengerStatus::arrived:
		{
			penaltyFinished += p.timeFinish - p.getTimeInit();
			if (passengersDetails)
				str << "#" << p.id << ", penalty = " << p.timeFinish - p.getTimeInit() \
				<< " (init = " << p.getTimeInit() << ", started = " << p.timeStart << ", finished = " << p.timeFinish << ")" \
				<< std::endl;
			break;
		}//case PassengerStatus::arrived:

		case PassengerStatus::going:
		{
			++numInElevetor;
			penaltyInElevetor += getCurrentTime() - p.getTimeInit();
			if (passengersDetails)
				str << "#" << p.id << ", penalty = " << getCurrentTime() - p.getTimeInit() \
				<< " (init = " << p.getTimeInit() << ", started = " << p.timeStart << ", STILL IN ELEVATOR!!!" << ")" \
				<< std::endl;
			break;
		}//case PassengerStatus::going:

		case PassengerStatus::waiting:
		{
			++numOnFloors;
			penaltyOnFloors += getCurrentTime() - p.getTimeInit();
			if (passengersDetails)
				str << "#" << p.id << ", penalty = " << getCurrentTime() - p.getTimeInit() \
				<< " (init = " << p.getTimeInit() << ", STILL WAITING FOR ELEVATOR!!!" << ")" \
				<< std::endl;
			break;
		}//case PassengerStatus::waiting:

		case PassengerStatus::leaved:
		{
			++numLeaved;
			penaltyLeaved += p.properties.criticalWaitTime * 5;
			if (passengersDetails)
				str << "#" << p.id << ", penalty = " << p.properties.criticalWaitTime * 5 \
				<< " (init = " << p.getTimeInit() << ", LEAVED THE FLOOR!!!" << ")" \
				<< std::endl;
			break;
		}//case PassengerStatus::leaved:
		}//switch (p.status)
	}//for p

	if (passengersDetails)
		str << std::endl;

	size_t waitingTime = 0, goingTime = 0, totalTime = 0;
	size_t num_finished_notLeaved = 0;
	for (auto& p : queue->finished)
	{		
		if (p.status != PassengerStatus::leaved)
		{
			++num_finished_notLeaved;
			waitingTime += p.timeStart - p.getTimeInit();
			goingTime += p.timeFinish - p.timeStart;
			totalTime += p.timeFinish - p.getTimeInit();
		}
		//else
		//{
		//	waitingTime += p.properties.criticalWaitTime;
		//	totalTime += p.properties.criticalWaitTime;
		//}
	}//for p

	str << "Number of passengers, that have finished the trip: " << num_finished_notLeaved << std::endl;
	if (num_finished_notLeaved > 0)
	{
		str << " average waiting time = " << 1.0 * waitingTime / num_finished_notLeaved << std::endl;
		str << " average going time =   " << 1.0 * goingTime / num_finished_notLeaved << std::endl;
		str << " average total time =   " << 1.0 * totalTime / num_finished_notLeaved << std::endl;
	}
	str << "Penalty for them = " << penaltyFinished << std::endl;
	str << std::endl;
	

	str << "Still waiting on floors = " << numOnFloors << std::endl;
	str << "Penalty for them = " << penaltyOnFloors << std::endl;
	str << std::endl;

	str << "Still in elevator = " << numInElevetor << std::endl;
	str << "Penalty for them = " << penaltyInElevetor << std::endl;
	str << std::endl;

	str << "Leaved the floors, because of too large waiting time = " << numLeaved << std::endl;
	str << "Penalty for them = " << penaltyLeaved << std::endl;
	str << std::endl;

	str << "TOTAL PENALTY = " << penaltyFinished + penaltyInElevetor + penaltyOnFloors + penaltyLeaved << std::endl;

	if (fname != "")
		fout.close();
}//PrintStatistics(...)

