// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Components/SkeletalMeshComponent.h"
#include "FGearSkeletalMeshComponent.generated.h"

class AFGearVehicle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGearSubstepTickSignature, float, deltaTime);

UCLASS(ClassGroup = (FGear), hidecategories = (Clothing, "Master Pose Component", "Clothing Simulation", Tags, "Component Replication", Activation, "Material Parameters", HLOD, "Asset User Data"))
class FGEARPLUGIN_API UFGearSkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()
public:
	UFGearSkeletalMeshComponent();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FGearSubstepTickSignature OnSubstepTick;

#if ENGINE_MAJOR_VERSION != 5
	void init(AFGearVehicle* v);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SubstepTick(float deltaTime, FBodyInstance* body);

private:
	AFGearVehicle* mVehicle;
	FCalculateCustomPhysics onCalcCustomPhysics;
#endif
};