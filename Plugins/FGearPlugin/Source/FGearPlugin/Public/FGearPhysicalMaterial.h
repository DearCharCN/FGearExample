// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "FGearPhysicalMaterial.generated.h"

UCLASS()
class FGEARPLUGIN_API UFGearPhysicalMaterial : public UPhysicalMaterial
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FGearPhysicalMaterial", DisplayName = "Receive Skidmark")
	bool mReceiveSkidmark;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FGearPhysicalMaterial", DisplayName = "Friction")
	float mFriction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FGearPhysicalMaterial", DisplayName = "Roughness")
	float mRoughness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FGearPhysicalMaterial", DisplayName = "Smoke Particle")
	UNiagaraSystem* mSmokeParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FGearPhysicalMaterial", DisplayName = "Skid Clip")
	class USoundWave* mSkidClip;

	UFGearPhysicalMaterial();
};