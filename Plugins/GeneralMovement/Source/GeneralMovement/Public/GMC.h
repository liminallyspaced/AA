// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#include "Modules/ModuleManager.h"

class FGeneralMovementModule : public IModuleInterface
{
public:

  virtual void StartupModule() override;
  virtual void ShutdownModule() override;
};
