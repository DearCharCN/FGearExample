// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearCustomCollision.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodySetup.h"

//WARNING : custom collision requires CTF_UseSimpleAsComplex mode
UFGearCustomCollision::UFGearCustomCollision(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer),
	mOverrideChannel(false),
	mChannel(ECollisionChannel::ECC_WorldStatic)
{
	Mobility = EComponentMobility::Movable;
	CastShadow = false;
	bHiddenInGame = true;
	bCastHiddenShadow = false;
	bReceiveMobileCSMShadows = false;

	if (!HasAnyFlags(RF_ClassDefaultObject)) SetMassScale(NAME_None, 0.0f);

	static ConstructorHelpers::FObjectFinder<UObject> wireframeMaterial(TEXT("Material'/Engine/EngineDebugMaterials/WireframeMaterial.WireframeMaterial'"));
	if (wireframeMaterial.Succeeded())
	{
		SetMaterial(0, (UMaterialInterface*)wireframeMaterial.Object);
	}
}

void UFGearCustomCollision::BeginPlay()
{
	Super::BeginPlay();
	//this line is added for UE5
	//somehow WorldDynamic mode does not collide with others
	if (mOverrideChannel) SetCollisionObjectType(mChannel);
}