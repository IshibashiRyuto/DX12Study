/****************************************************/
/*		Singleton.h									*/
/*		Auther	: Ishibashi Ryuto					*/
/*		Date	:	2018/3/01	00:01				*/
/****************************************************/

/// 継承すると継承先のクラスをsingleton化する
/// 使用する際は、継承先のクラスをテンプレートクラスとして渡すこと

#pragma once

#include<memory>

template<class T>
class Singleton
{
public:
	
	// インスタンス取得
	static T* Instance()
	{
		static T _instance;
		return &_instance;
	}

	virtual ~Singleton() 
	{
	}


private:
	Singleton() {}
	Singleton(const Singleton&) {}
	const Singleton& operator=(const Singleton&) {}
};
