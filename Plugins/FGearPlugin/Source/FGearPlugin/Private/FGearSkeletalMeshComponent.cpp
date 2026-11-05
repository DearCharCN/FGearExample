// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearSkeletalMeshComponent.h"
#include "FGearVehicle.h"

UFGearSkeletalMeshComponent::UFGearSkeletalMeshComponent()
{
	SetCollisionProfileName(TEXT("Pawn"));
	bApplyImpulseOnDamage = 0;
	bPerBoneMotionBlur = false;
	bReceivesDecals = false;
#if ENGINE_MAJOR_VERSION != 5
	mVehicle = nullptr;
#endif
}

#if ENGINE_MAJOR_VERSION != 5
void UFGearSkeletalMeshComponent::init(AFGearVehicle* v)
{
	mVehicle = v;

	if (v->isSubstepping())
	{
		PrimaryComponentTick.bCanEverTick = true;
		PrimaryComponentTick.bStartWithTickEnabled = true;
		onCalcCustomPhysics.BindUObject(this, &UFGearSkeletalMeshComponent::SubstepTick);
	}
}

void UFGearSkeletalMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (mVehicle == nullptr) return;
	FBodyInstance* body = mVehicle->getBody();
	if (body) body->AddCustomPhysics(onCalcCustomPhysics);
}

void UFGearSkeletalMeshComponent::SubstepTick(float deltaTime, FBodyInstance* body)
{
	mVehicle->substepTick(deltaTime);
	//fire substep event
	if (OnSubstepTick.IsBound()) OnSubstepTick.Broadcast(deltaTime);
}
#endif