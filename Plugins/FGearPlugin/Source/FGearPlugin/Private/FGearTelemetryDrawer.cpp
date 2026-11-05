// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearTelemetryDrawer.h"

UFGearTelemetryDrawer::UFGearTelemetryDrawer() :
	USceneComponent()
{
	SetActive(true);
	PrimaryComponentTick.bCanEverTick = false;
}

FVector2D UFGearTelemetryDrawer::drawTelemetry(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p)
{
	if (p.Y + getWindowHeight() > Canvas->SizeY)
	{
		p.Y = 0;
		p.X += 190;
	}
	return p;
}