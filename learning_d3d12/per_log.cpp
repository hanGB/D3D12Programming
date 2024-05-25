#include "stdafx.h"
#include "per_log.h"
#include "null_logger.h"

PERLogger* PERLog::m_logger;

PERLog::~PERLog()
{
    delete m_logger;
}

PERLogger& PERLog::Logger()
{
    return *m_logger;
}

void PERLog::SetLogger(PERLogger* logger)
{
    if (logger == nullptr) m_logger = new NullLogger();
    else m_logger = logger;
}
