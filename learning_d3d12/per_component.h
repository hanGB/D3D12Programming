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