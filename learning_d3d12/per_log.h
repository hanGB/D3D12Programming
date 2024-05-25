#pragma once
#include "per_logger.h"

class PERLog {
public:
	~PERLog();

	static PERLogger& Logger();
	static void SetLogger(PERLogger* logger);

private:
	static PERLogger* m_logger;
};