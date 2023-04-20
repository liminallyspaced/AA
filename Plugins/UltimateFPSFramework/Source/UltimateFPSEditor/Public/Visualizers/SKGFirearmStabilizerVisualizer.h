﻿// Copyright 2022, Dakota Dawe, All rights reserved

#pragma once

#include "ComponentVisualizer.h"

class FSKGFirearmStabilizerVisualizer : public FComponentVisualizer
{
public:
	FSKGFirearmStabilizerVisualizer();

	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};
