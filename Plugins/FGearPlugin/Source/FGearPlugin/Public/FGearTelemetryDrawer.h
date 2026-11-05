// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "FGearGlobalLog.h"
#include "Kismet/KismetStringLibrary.h"
#include "FGearUtility.h"
#include "FGearTelemetryDrawer.generated.h"

UCLASS(abstract, ClassGroup = (FGear), hidecategories = (Transform, Rendering, Physics, Mobility, LOD, ComponentTick, Tags, AssetUserData, ComponentReplication, Activation, Variable, Collision, Cooking))
class FGEARPLUGIN_API UFGearTelemetryDrawer : public USceneComponent
{
	GENERATED_BODY()
private:
	int mWindowHeight;
public:
	UFGearTelemetryDrawer();
	virtual FVector2D drawTelemetry(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p);
protected:
	void setWindowHeight(int h) { mWindowHeight = h; }
	int getWindowHeight() const { return mWindowHeight; }
};