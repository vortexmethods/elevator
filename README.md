Введение в специальность 
========================

(кафедра ФН2 МГТУ им. Н.Э. Баумана, 1 курс)

Симулятор пассажирского лифта
-----------------------------

<p align="left"><img src="https://www.unitalm.ru/blog/wp-content/uploads/ekspluatacia-liftov.png"></p>

![Repo Size](https://img.shields.io/github/repo-size/vortexmethods/elevator.svg)

Copyright (C) 2021 Марчевский И.К.


СОДЕРЖАНИЕ РЕПОЗИТОРИЯ
----------------------

* В папке **Code** - исходный код симулятора

* В папке **Отчет** - бланк отчета

### Ваша задача 

* Реализовать функцию **CONTROLSYSTEM** вместо реализованного там сравнительно простого примера 
* При необходимости можно модифицировать структуру **myParams**.
* Остальные функции "трогать" ***запрещается***.

### Примечания
* Если будут находиться "баги" в данном симуляторе - прошу писать о них в раздел [**Issues**](https://github.com/vortexmethods/elevator/issues), будем исправлять!

### Документация
* Документация разработчика доступна по [ссылке](https://vortexmethods.github.io/elevator)

      https://vortexmethods.github.io/elevator

* Также сохранены подробные комментарии в файле **main.cpp**
* Приватные члены классов прокомментированы пока лишь фрагментарно
* Кое-что запрограммировано "кривовато", прошу очень сильно не возмущаться :-)

### Обновления
* 24.03.2021 - добавлен вывод в файл либо на экран статистики по пассажирам
* 25.03.2021 - добавлена документация в формате [doxygen](http://www.doxygen.org) и исправлены мелкие ошибки
* 29.03.2021 - реализованы функции ухода пассажира, не дождавшегося лифта, посадки пассажира в лифт в другую сторону, нажатия пассажирами кнопки "ход" и, самое главное, ***возможности работы с несколькими лифтами***
* 14.04.2021 - добавлены константы (в public-секции класса **control**), задающие времена выполнения основных операций, а также скорость движения лифта (изменять их нельзя)
* 25.05.2021 - исправлены замеченные ошибки; реализована возможность считывания расписания появления пассажиров из файла; добавлен тестовый файл расписания, результаты работы для которого следует включить в отчет; выложен макет отчета 