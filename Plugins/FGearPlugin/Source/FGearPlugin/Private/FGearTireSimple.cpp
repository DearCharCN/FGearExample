// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearTireSimple.h"

UFGearTireSimple::UFGearTireSimple() :
	UFGearTire(),
	Lnb(15.0f),
	Lnc(2.0f),
	Lnd(1.0f),
	Lne(0.95f),
	Ltb(5.0f),
	Ltc(2.0f),
	Ltd(1.0f),
	Lte(0.9f)
{
	updatePreviewCurve();
}

void UFGearTireSimple::init()
{
	Super::init();
	mCombineMode = FGearForceCombineMode::GRIP;
}

void UFGearTireSimple::calculate(float load, float slipRatio, float slipAngle, float camber, float pressure, float radius, float Vx)
{
	mLongitudinalForce = load * (Lnd * FMath::Sin(Lnc * FMath::Atan(Lnb * (1 - Lne) * slipRatio + Lne * FMath::Atan(Lnb * slipRatio))));
	mLateralForce = load * (Ltd * FMath::Sin(Ltc * FMath::Atan(Ltb * (1 - Lte) * slipAngle + Lte * FMath::Atan(Ltb * slipAngle))));
}