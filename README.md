# BaluScript

Platform | Windows | Linux
---------|---------|------
Build status | [![Build status](https://ci.appveyor.com/api/projects/status/584yr2mob5jwnuk9?svg=true)](https://ci.appveyor.com/project/HumMan/baluscript) | [![Build Status](https://travis-ci.org/HumMan/BaluScript.svg?branch=master)](https://travis-ci.org/HumMan/BaluScript)

Simple C++ like interpreted language with static types and operators overloading

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

#Supported features
- [X] classes
- [X] nested classes
- [X] static class fields
- [X] static local variables
- [X] constructors, destructors, copy constructor, move constructor
- [X] operators overloading
- [X] implicit conversion overloading
- [X] template classes
- [X] if, while, for
- [X] enumerations
- [X] base types: int float vec2 bool string char
- [X] native static and dynamic arrays
- [X] inheritance
- [X] methods and fields visibility

#TODO
* lambda
* template specialization (simple add new classes in realization)
* default parameters like some_func(cutted_name: "tests")
* *var* or *auto* word like in c# or c++
* template methods
* *const* parameters (like const by ref)
* maybe *out* parameters
* properties
* multiple return values (like in Go)
* partial class
