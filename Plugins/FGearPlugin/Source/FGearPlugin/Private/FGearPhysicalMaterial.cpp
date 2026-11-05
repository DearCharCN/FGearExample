// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearPhysicalMaterial.h"

UFGearPhysicalMaterial::UFGearPhysicalMaterial() :
	UPhysicalMaterial(),
	mReceiveSkidmark(true),
	mFriction(1.0f),
	mRoughness(0.0f)
{
}