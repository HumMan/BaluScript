# BaluScript

Platform | Windows | Linux
---------|---------|------
Build status | [![Build status](https://ci.appveyor.com/api/projects/status/584yr2mob5jwnuk9?svg=true)](https://ci.appveyor.com/project/HumMan/baluscript)<br>![Test status](http://teststatusbadge.azurewebsites.net/api/status/HumMan/baluscript) | [![Build Status](https://travis-ci.org/HumMan/BaluScript.svg?branch=master)](https://travis-ci.org/HumMan/BaluScript)

Простой, интерпретируемый язык, похожий на C++ и C#. Все примеры кода можно посмотреть в тестах здесь [GitHub](Test/unittest1.cpp). Модель памяти как в C++, нет неявных выделений/удалений, используются конструкторы и деструкторы.

Указатели отсутствуют. Есть ссылки, но имеется runtime механизм проверки ссылок на объекты в деструкторе (отслеживать use after free).
Методы/поля можно использовать до объявления.

```
class TestClass
{
	class Vec2<T,Size>
	{
		TStaticArray<T,Size> value;
		copy(T v0, T v1)
		{
			value[0]=v0;value[1]=v1;
		}
		copy(Vec2 l)
		{
			value[0]=l[0];value[1]=l[1];
		}
		operator +(Vec2 l, Vec2 r):Vec2
		{
			return new Vec2(l[0]+r[0],l[1]+r[1]);
		}
		operator -(Vec2 l, Vec2 r):Vec2
		{
			return new Vec2(l[0]-r[0],l[1]-r[1]);
		}
		operator [](Vec2& l, int i):T
		{
			return l.value[i];
		}
		func Dot(Vec2 r):T
		{
			return value[0]*r[0]+value[1]*r[1];
		}
	}
	func static Test:int
	{
		Vec2<int, 2> v(3,5),r(-2,8);
		Vec2<int, 2> s=(v+r);
		return s.Dot(v-r);
	}
}
```

## Что поддерживает
- [X] Классы
- [X] Вложенные классы
- [X] Шаблонные классы (параметр класс и int)
- [X] Поля классов (могут быть статичные, с доступом private/public)
- [X] Статичные локальные переменные
- [X] Перегрузка конструктора по умолчанию(default)/копии(copy), деструктора(dest), операторов, преобразований(conversion)
- [X] Стандартные операторы цикла while, for
- [X] Условный оператор if else
- [X] Перечисления enum
- [X] Набор встроенных классов: int float bool vec2 vec2i char. Сами классы тут [GitHub](Source/NativeTypes/base_types.h)
- [X] Динамические массивы TDynArray<T>
- [X] Статические массивы TStaticArray<T,Size>
- [X] Класс string - обёртка над std::string
- [X] Автоматический вывод типа локальной переменной через *var*
- [X] Имеется TPtr<T> - аналог std::shared_ptr<T> (т.к. указателей нет, а иметь общие объекты нужно)
- [X] Операторные скобки, выражения - тут как обычно
- [X] Перегрузка методов
- [X] Имеются extern методы для вызова метода C++ из скрипта

Т.к. BaluScript разрабатывался для конструктора BaluEngine, то имеются функции для взаимодействия с внешним миром
- [X] Вызов метода скрипта из C++ (тут вручную преобразуем параметры)
- [X] Генерации биндингов для скрипта для вызова его методов из C++
