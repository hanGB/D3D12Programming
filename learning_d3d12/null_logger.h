#pragma once
#include "per_logger.h"

class NullLogger : public PERLogger {
public:
	virtual void Update() {}

	virtual void Info(const char* text) {}
	virtual void Warnning(const char* text) {}
	virtual void Error(const char* text) {}

	virtual void SaveLogs() {}
	virtual void SaveWarnnings() {}
	virtual void SaveErrors() {}
	virtual void SaveAll() {}
};