// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearAnimInstance.h"

UFGearAnimInstance::UFGearAnimInstance()
{
	axles.Init(FGearBoneAxle(), 16);
}

void UFGearAnimInstance::init(int num)
{
	axles.Empty();
	axles.Init(FGearBoneAxle(), num);
}

void UFGearAnimInstance::update(int axle, bool isLeft, FVector p, FRotator r)
{
	if (!axles.IsValidIndex(axle)) return;

	if (isLeft)
	{
		axles[axle].left.position = p;
		axles[axle].left.rotation = r;
	}
	else
	{
		axles[axle].right.position = p;
		axles[axle].right.rotation = r;
	}
}