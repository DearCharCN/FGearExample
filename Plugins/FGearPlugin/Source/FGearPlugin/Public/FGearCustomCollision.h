// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "FGearCustomCollision.generated.h"

UCLASS(ClassGroup = (FGear), meta = (BlueprintSpawnableComponent))
class FGEARPLUGIN_API UFGearCustomCollision : public UStaticMeshComponent
{
	GENERATED_BODY()
public:
	UFGearCustomCollision(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

	/** If true, UPrimitiveComponent::SetCollisionObjectType will be called with the "Collision Channel" parameter. */
	UPROPERTY(EditAnywhere, Category = "FGear", DisplayName = "Override Collision Channel")
	bool mOverrideChannel;
	/** Collision channled type to be set if "Override Collision Channel" is true. */
	UPROPERTY(EditAnywhere, Category = "FGear", DisplayName = "Collision Channel", meta = (EditCondition = "mOverrideChannel"))
	TEnumAsByte<ECollisionChannel> mChannel;
};