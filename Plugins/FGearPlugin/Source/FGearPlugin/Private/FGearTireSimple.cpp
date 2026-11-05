// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearTireSimple.h"

UFGearTireSimple::UFGearTireSimple() :
	UFGearTire(),
	Lnb(15.0),
	Lnc(2.0),
	Lnd(1.0),
	Lne(0.95),
	Ltb(5.0),
	Ltc(2.0),
	Ltd(1.0),
	Lte(0.9)
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