#pragma once

#include "Modules/ModuleManager.h"

class FNineSliceEditor : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};