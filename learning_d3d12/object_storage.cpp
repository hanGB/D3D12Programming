#include "stdafx.h"
#include "object_storage.h"
#include "object_factory.h"
#include "object_type.h"
#include "per_object.h"
#include "resource_type.h"

ObjectStorage::ObjectStorage()
{
	CreateBasicObjectFactories();
	FillObjectQueues();
}

ObjectStorage::~ObjectStorage()
{
	ClearObjectQueues();
	DeleteObjectFactories();
}

PERObject* ObjectStorage::PopObject(int type)
{
	auto it = m_objectQueueMap.find(type);

	if (it == m_objectQueueMap.end()) return nullptr;

	auto& objectQueue = it->second;
	if (objectQueue.empty()) RefillObjectQueue(objectQueue, type);
	PERObject* newObject = objectQueue.front();
	objectQueue.pop();

	return newObject;
}

void ObjectStorage::PushObject(PERObject* object)
{
	int type = object->GetObjectType();
	object->Initialize();

	auto it = m_objectQueueMap.find(type);
	if (it == m_objectQueueMap.end())
	{
		PERLog::Logger().WarnningWithFormat("%d에 해당하는 오브젝트 타입이 오브젝트 스토리지에 없습니다.", type);
		// 일단 안 쓰는 거니까 삭제
		delete object;
		return;
	}

	it->second.push(object);
}

void ObjectStorage::AddObjectFactory(int type, ObjectFactory* factory)
{
	m_objectFactoryMap.insert(std::pair<int, ObjectFactory*>(type, factory));
}

void ObjectStorage::MakeObjectQueuesWithAddedFactories()
{
	for (auto it = m_objectFactoryMap.begin(); it != m_objectFactoryMap.end(); ++it) 
	{
		auto queueIt = m_objectQueueMap.find(it->first);
		// 큐가 없을 경우 추가
		if (queueIt == m_objectQueueMap.end())
		{
			std::queue<PERObject*> queue;
			ObjectFactory* factory = it->second;
			for (int i = 0; i < PER_OBJECT_STORAGE_BASIC_QUEUE_SIZE; ++i) {
				queue.push(factory->CreateObject<PERObject>());
			}
			m_objectQueueMap.insert(std::pair<int, std::queue<PERObject*>>(it->first, queue));
		}
	}
}

void ObjectStorage::CreateBasicObjectFactories()
{
	ObjectFactory* factory = new ObjectFactory(PER_FIXED, PER_BASE_COMPONENT, PER_ROTATING_AI, PER_BASE_COMPONENT, PER_BASE_COMPONENT);
	factory->SetResourceType(PER_CUBE);
	AddObjectFactory(PER_FIXED, factory);
}

void ObjectStorage::DeleteObjectFactories()
{
	for (auto& pair : m_objectFactoryMap)
	{
		delete pair.second;
	}
	m_objectFactoryMap.clear();
}

void ObjectStorage::FillObjectQueues()
{
	for (auto it = m_objectFactoryMap.begin(); it != m_objectFactoryMap.end(); ++it) 
	{
		std::queue<PERObject*> queue;
		ObjectFactory* factory = it->second;
		for (int i = 0; i < PER_OBJECT_STORAGE_BASIC_QUEUE_SIZE; ++i) {
			queue.push(factory->CreateObject<PERObject>());
		}
		m_objectQueueMap.insert(std::pair<int, std::queue<PERObject*>>(it->first, queue));
	}
}

void ObjectStorage::RefillObjectQueue(std::queue<PERObject*>& queue, int type)
{
	// 큐가 존재한다는 건 팩토리가 있다 것이 확정되어 있으니 있는 지 여부 검사는 넘어감
	ObjectFactory* factory = m_objectFactoryMap[type];

	for (int i = 0; i < PER_OBJECT_STORAGE_BASIC_QUEUE_SIZE; ++i) {
		queue.push(factory->CreateObject<PERObject>());
	}
}

void ObjectStorage::ClearObjectQueues()
{
	for (auto& pair : m_objectQueueMap)
	{
		auto& queue = pair.second;

		while (!queue.empty()) {
			PERObject* object = queue.front();
			queue.pop();
			delete object;
		}
	}
	m_objectQueueMap.clear();
}
