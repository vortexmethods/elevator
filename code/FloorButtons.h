// Проект elevator 
// (c) И.К. Марчевский, 2021

/*!
\file
\brief Заголовочный файл с описанием класса FloorButtons
\author Марчевский Илья Константинович
\version 0.2
\date 25 марта 2021 г.
*/

#pragma once

#include <vector>

/*!
\brief Класс --- кнопки на этажах
*/
class FloorButtons
{

	friend class Control;

private:
	/// Вектор состояний кнопок вверх
	std::vector<bool> upButtons;

	/// Вектор состояний кнопок вниз
	std::vector<bool> dnButtons;

	void setUpButton(size_t i)
	{
		upButtons[i] = true;
	}

	void setDnButton(size_t i)
	{
		dnButtons[i] = true;
	}

	void unsetUpButton(size_t i)
	{
		upButtons[i] = false;
	}

	void unsetDnButton(size_t i)
	{
		dnButtons[i] = false;
	}

	const std::vector<bool>& getUpButtons() const 
	{ 
		return upButtons; 
	}
	
	const std::vector<bool>& getDnButtons() const 
	{ 
		return dnButtons; 
	}

	bool getUpButton(size_t i) const
	{
		return upButtons[i];
	}

	bool getDnButton(size_t i) const		
	{
		return dnButtons[i];
	}

public:
	/// Инициализирующий конструктор
	///
	/// \param[in] numberOfFloors количество этажей (включая подвал)
	FloorButtons(size_t numberOfFloors)
		: upButtons(numberOfFloors, false)
		, dnButtons(numberOfFloors, false)
	{}
};

