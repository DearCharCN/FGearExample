// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearTire96.h"
#include "FGearUtility.h"

UFGearTire96::UFGearTire96() :
	UFGearTire(),
	//nominals
	mNominalLoad(5000.0f),
	//pacejka 96 longitudinal
	PCX1(1.685f),
	PDX1(1.210f),
	PDX2(-0.037f),
	PEX1(0.344f),
	PEX2(0.095f),
	PEX3(-0.020f),
	PEX4(0.0f),
	PKX1(21.51f),
	PKX2(-0.163f),
	PKX3(0.245f),
	PHX1(0.0f),
	PHX2(0.0f),
	PVX1(0.0f),
	PVX2(0.0f),
	//pacejka 96 lateral
	PCY1(1.193f),
	PDY1(-0.990f),
	PDY2(0.145f),
	PDY3(-11.23f),
	PEY1(-1.003f),
	PEY2(-0.537f),
	PEY3(-0.083f),
	PEY4(-4.787f),
	PKY1(-14.95f),
	PKY2(2.130f),
	PKY3(-0.028f),
	PHY1(0.003f),
	PHY2(-0.001f),
	PHY3(0.075f),
	PVY1(0.045f),
	PVY2(-0.024f),
	PVY3(-0.532f),
	PVY4(0.039f),
	//pacejka 96 combined longitudinal
	RBX1(12.35f),
	RBX2(-10.77f),
	RCX1(1.092f),
	REX1(0.0f),
	REX2(0.0f),
	RHX1(0.007f),
	//pacejka 96 combined lateral
	RBY1(6.461f),
	RBY2(4.196f),
	RBY3(-0.015f),
	RCY1(1.081f),
	REY1(0.0f),
	REY2(0.0f),
	RHY1(0.009f),
	RVY1(0.053f),
	RVY2(-0.073f),
	RVY3(0.517f),
	RVY4(35.44f),
	RVY5(1.9f),
	RVY6(-10.71f)
{
	updatePreviewCurve();
}

void UFGearTire96::init()
{
	Super::init();
	mCombineMode = FGearForceCombineMode::SUM;
}

void UFGearTire96::calculate(float load, float slipRatio, float slipAngle, float camber, float pressure, float radius, float Vx)
{
	if (load <= 0.0f)
	{
		mLongitudinalForce = mLateralForce = 0.0f;
		return;
	}

	//symmetry signs
	float lngSign = 1.0f;
	float latSign = 1.0f;
	if (mForceSymmetry)
	{
		if (slipRatio < 0.0f)
		{
			slipRatio *= -1.0f;
			lngSign = -1.0f;
		}
		else if (slipRatio == 0.0f) lngSign = 0.0f;
		if (slipAngle < 0.0f)
		{
			slipAngle *= -1.0f;
			latSign = -1.0f;
		}
		else if (slipAngle == 0.0f) latSign = 0.0f;
	}

	slipAngle *= -1.0f; //sign convention
	slipAngle = FMath::Tan(slipAngle);
	camber = FMath::Sin(camber);

	float dFz = (load - mNominalLoad) / mNominalLoad;

	//longitudinal
	float Fx0 = pacejkaFx(slipRatio, load, dFz);

	//lateral
	float Dyk;
	float Fy0 = pacejkaFy(slipAngle, camber, load, dFz, Dyk);

	//combined
	float Gx = 1.0f;
	float Gy = 1.0f;
	float Svyk = 0.0f;

	if (mCombineMode == FGearForceCombineMode::SUM)
	{
		Gx = pacejkaGx(slipRatio, slipAngle, dFz);
		Gy = pacejkaGy(slipRatio, slipAngle, dFz);
		Svyk = pacejkaSvyk(slipRatio, slipAngle, camber, dFz, Dyk);
	}

	//sum up
	mLongitudinalForce = Gx * Fx0 * lngSign;
	mLateralForce = (Gy * Fy0 + Svyk) * latSign;
}

float UFGearTire96::pacejkaFx(float slipRatio, float Fz, float dFz)
{
	float ux = PDX1 + PDX2 * dFz; //simplified, lambda_uV is zero by default 
	float Dx = FMath::Max(ux * Fz, 0.0f); //>0
	float Cx = FMath::Max(PCX1, 0.0f); //>0
	float Kxk = Fz * (PKX1 + PKX2 * dFz) * FMath::Exp(PKX3 * dFz);
	float Bx = Kxk / (Cx * Dx + FGearUtility::epsilon);
	float Shx = PHX1 + PHX2 * dFz;
	float Kx = slipRatio + Shx;
	float Ex = (PEX1 + PEX2 * dFz + PEX3 * dFz * dFz) * (1.0f - PEX4 * FMath::Sign(Kx));
	Ex = FMath::Min(Ex, 1.0f); //<=1
	float Svx = Fz * (PVX1 + PVX2 * dFz);
	float Fx0 = Dx * FMath::Sin(Cx * FMath::Atan(Bx * Kx - Ex * (Bx * Kx - FMath::Atan(Bx * Kx)))) + Svx;
	return Fx0;
}

float UFGearTire96::pacejkaFy(float slipAngle, float camber, float Fz, float dFz, float& Dyk)
{
	float Fz0 = mNominalLoad;
	float uy = (PDY1 + PDY2 * dFz) * (1.0f - PDY3 * camber * camber); //simplified, lambda_uV is zero by default 
	float Dy = uy * Fz;
	float Cy = FMath::Max(PCY1, 0.0f); //>0
	float Kya = PKY1 * Fz0 * FMath::Sin(2.0f * FMath::Atan(Fz / (PKY2 * Fz0)));
	float Ky = Kya * (1.0f - PKY3 * camber * camber);
	float By = Ky / (Cy * Dy + FGearUtility::epsilon);
	float Shy = (PHY1 + PHY2 * dFz) + (PHY3 * camber);
	float Ay = slipAngle + Shy;
	float Ey = (PEY1 + PEY2 * dFz) * (1.0f - (PEY3 + PEY4 * camber) * FMath::Sign(Ay));
	Ey = FMath::Min(Ey, 1.0f); //<=1
	float Svy = Fz * ((PVY1 + PVY2 * dFz) + (PVY3 + PVY4 * dFz) * camber);
	float Fy0 = Dy * FMath::Sin(Cy * FMath::Atan(By * Ay - Ey * (By * Ay - FMath::Atan(By * Ay)))) + Svy;

	//for PacejkaSvy
	Dyk = Dy;

	return Fy0;
}

float UFGearTire96::pacejkaGx(float slipRatio, float slipAngle, float dFz)
{
	float Bx = FMath::Max(RBX1 * FMath::Cos(FMath::Atan(RBX2 * slipRatio)), 0.0f); //>0
	float Cx = RCX1;
	float Shx = RHX1;
	float As = slipAngle + Shx;
	float Ex = FMath::Min(REX1 + REX2 * dFz, 1.0f); // <=1
	float G0 = FMath::Cos(Cx * FMath::Atan(Bx * Shx - Ex * (Bx * Shx - FMath::Atan(Bx * Shx))));
	float Gx = FMath::Cos(Cx * FMath::Atan(Bx * As - Ex * (Bx * As - FMath::Atan(Bx * As)))) / G0;
	return FMath::Max(Gx, 0.0f); //>0
}

float UFGearTire96::pacejkaGy(float slipRatio, float slipAngle, float dFz)
{
	float By = FMath::Max(RBY1 * FMath::Cos(FMath::Atan(RBY2 * (slipAngle - RBY3))), 0.0f); //>0
	float Cy = RCY1;
	float Shy = RHY1;
	float Ks = slipRatio + Shy;
	float Ey = FMath::Min(REY1 + REY2 * dFz, 1.0f); //<=1
	float G0 = FMath::Cos(Cy * FMath::Atan(By * Shy - Ey * (By * Shy - FMath::Atan(By * Shy))));
	float Gy = FMath::Cos(Cy * FMath::Atan(By * Ks - Ey * (By * Ks - FMath::Atan(By * Ks)))) / G0;
	return FMath::Max(Gy, 0.0f); //>0
}

float UFGearTire96::pacejkaSvyk(float slipRatio, float slipAngle, float camber, float dFz, float Dyk)
{
	float Dvyk = Dyk * (RVY1 + RVY2 * dFz + RVY3 * camber) * FMath::Cos(FMath::Atan(RVY4 * slipAngle));
	float Svyk = Dvyk * FMath::Sin(RVY5 * FMath::Atan(RVY6 * slipRatio));
	return Svyk;
}