/****************************************************/
/*		Singleton.h									*/
/*		Auther	: Ishibashi Ryuto					*/
/*		Date	:	2018/3/01	00:01				*/
/****************************************************/

/// �p������ƌp����̃N���X��singleton������
/// �g�p����ۂ́A�p����̃N���X���e���v���[�g�N���X�Ƃ��ēn������

#pragma once

#include<memory>

template<class T>
class Singleton
{
public:
	
	// �C���X�^���X�擾
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
