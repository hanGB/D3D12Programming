#pragma once
#include "stdafx.h"

class PERObject;

class PERComponent {
public:
	PERComponent() {}
	virtual ~PERComponent() { if (m_next) delete m_next; }

	virtual void Initialize() { if (m_next) m_next->Initialize(); }
	
	PERObject* GetOwner() { return m_owner; }
	PERComponent* GetNext() { return m_next; }
	void SetOwner(PERObject* owner) { m_owner = owner; if (m_next) m_next->SetOwner(owner); }
	void SetNext(PERComponent* next) { m_next = next; }

private:
	PERObject* m_owner = nullptr;
	PERComponent* m_next = nullptr;
};

struct ComponentVectors {
	std::vector<int> inputs;
	std::vector<int> ais;
	std::vector<int> physicses;
	std::vector<int> graphicses;
	std::vector<int> others;
};


// 종류
// parent
#define PER_NON_COMPONENT		0x00
#define PER_BASE_COMPONENT		0x01
// input
#define PER_PLAYER_INPUT		0x11
// ai
#define PER_ROTATING_AI			0x21
// physics
#define PER_PLAYER_PHYSICS		0x31
// graphics
#define PER_PLAYER_GRAPHICS		0x41
// other
#define PER_CAMERA_COMPONENT	0x51
