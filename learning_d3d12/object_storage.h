#pragma once

class PERObject;
class ObjectFactory;

class ObjectStorage {
public:
	ObjectStorage();
	~ObjectStorage();

	PERObject* PopObject(int type);
	void PushObject(PERObject* object);

	void AddObjectFactory(int type, ObjectFactory* factory);
	// 추가된 팩토리들로 큐 생성
	void MakeObjectQueuesWithAddedFactories();

private:
	// 기본 오브젝트 펙토리 생성
	void CreateBasicObjectFactories();
	void DeleteObjectFactories();

	// 오브젝트 큐 채우기
	void FillObjectQueues();
	void RefillObjectQueue(std::queue<PERObject*>& queue, int type);
	void ClearObjectQueues();

	std::unordered_map<int, ObjectFactory*> m_objectFactoryMap;
	std::unordered_map<int, std::queue<PERObject*>> m_objectQueueMap;
};