#pragma once

template<typename T>
class Singleton {
public:
	static T* InstancePtr() {// get pointer
		return &getInstance();
	}

	static T& getInstance() {// get reference
		static T _instance;// local static variable
		return _instance;
	}

	Singleton(Singleton const&) = delete;
	Singleton& operator=(Singleton const&) = delete;
protected:
	Singleton() = default;
};
