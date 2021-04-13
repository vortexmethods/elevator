// Проект elevator 
// (c) И.К. Марчевский, 2021

/*!
\file
\brief Файл кода с описанием класса Elevator
\author Марчевский Илья Константинович
\version 0.4
\date 14 апреля 2021 г.
*/

#include <iostream>

#include "Elevator.h"


Elevator::Elevator(size_t numberOfFloors, size_t maxCapacity, size_t id)
	: buttons(numberOfFloors, false)
	, capacity(maxCapacity)
	, myid(id)
	, timeToSelfProgramme(0)
	, position(0)
	, acceleration(ElevatorAcceleration::uniform)
	, destinationFloor(0)
	, doorsStatus(ElevatorDoorsStatus::closed)
	, indicator(ElevatorIndicator::both)
	, status(ElevatorStatus::staying)
{	
}


void Elevator::setDestination(size_t i) 
{ 
	try
	{
		if ((i < 0) || (i >= buttons.size()))
			throw std::exception("incorrect floor");
	}
	catch(...)
	{ 
		std::cout << "Exception: incorrect floor = " << i << std::endl;
		exit(-1);
	}

	destinationFloor = i; 
}//setDestination(...) 


void Elevator::setIndicator(ElevatorIndicator indicator)
{
	this->indicator = indicator;
}//setIndicator(...)


bool Elevator::isGoingButtonPressed() const
{	
	for (auto& p : passengers)
		if (p.PerformNoWaitingProbability())
			return true;
	return false;
}//isGoingButtonPressed


std::string Elevator::getStateString() const
{
	std::string strState;
		
	std::string strStatus;
	switch (status)
	{
	case ElevatorStatus::movingUp:
		strStatus = "up,  ";
		break;
	case ElevatorStatus::movingDn:
		strStatus = "dn,  ";
		break;
	case ElevatorStatus::staying:
		strStatus = "stay,";
		break;
	}//switch (status)

	std::string strIndicator;
	switch (indicator)
	{
	case ElevatorIndicator::up:
		strIndicator = "up,  ";
		break;
	case ElevatorIndicator::down:
		strIndicator = "down,";
		break;
	case ElevatorIndicator::both:
		strIndicator = "both,";
		break;
	}// switch (indicator)

	std::string strAccel;
	switch (acceleration)
	{
	case ElevatorAcceleration::accelerating:
		strAccel = "acceler.,";
		break;
	case ElevatorAcceleration::breaking:
		strAccel = "breaking,";
		break;
	case  ElevatorAcceleration::uniform:
		strAccel = "uniform, ";
		break;
	}//switch (acceleration)

	std::string strDoors;
	switch (doorsStatus)
	{
	case ElevatorDoorsStatus::openedUnloading:
		strDoors = "unloading...";
		break;
	case ElevatorDoorsStatus::openedLoading:
		strDoors = "loading...  ";
		break;
	case ElevatorDoorsStatus::opening:
		strDoors = "opening...  ";
		break;
	case ElevatorDoorsStatus::closing:
		strDoors = "closing...  ";
		break;
	case ElevatorDoorsStatus::closed:
		strDoors = "closed      ";
		break;
	case ElevatorDoorsStatus::waiting:
		strDoors = "waiting...  ";
		break;
	}//switch (doorsStatus)

	strState += "level = ";
	strState += std::to_string(position / 100);
	strState += ".";
	if ((position % 100) < 10) 
		strState += "0";
	strState += std::to_string(position % 100);
		
	strState += ", dir. = ";
	strState += strStatus.c_str();

	strState += " ind. = ";
	strState += strIndicator.c_str();

	strState += " acceler. = ";
	strState += strAccel.c_str();
	strState += " doors = ";
	strState += strDoors.c_str();
		
	strState += " (pass.: ";
	for (auto& p : passengers)
	{
		strState += std::to_string(p.id);
		strState += ", ";
	}//for p
	strState += ")";
	
	return strState;
}//getStateString()


bool Elevator::isDestinationAchieved(const std::vector<bool>& upButtons, const std::vector<bool>& dnButtons) const
{
	bool b1 = ((position / 100 == destinationFloor) && (position % 100 == 0));
	bool b2 = ((status == ElevatorStatus::staying) && (!isEmpty()) && (doorsStatus == ElevatorDoorsStatus::openedUnloading || doorsStatus == ElevatorDoorsStatus::openedLoading || doorsStatus == ElevatorDoorsStatus::waiting));
	bool b3 = ((status == ElevatorStatus::staying) && (isEmpty()) && ( upButtons[position/100] || dnButtons[position / 100] ) && (doorsStatus == ElevatorDoorsStatus::openedUnloading || doorsStatus == ElevatorDoorsStatus::openedLoading || doorsStatus == ElevatorDoorsStatus::waiting));
	bool b4 = ((status == ElevatorStatus::staying) && (isEmpty()) && (!upButtons[position / 100] && !dnButtons[position / 100]));
	
	return (b1 && (b2 || b3 || b4));
}//isDestinationAchieved(...)

