// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearTireMF61.h"
#include "FGearUtility.h"

UFGearTireMF61::UFGearTireMF61() : 
	UFGearTire(),
	//nominals
	mNominalLoad(4000.0f),
	mNominalPressure(2.2f),
	//pacejka longitudinal
	PCX1(1.579f),
	PDX1(1.0422f),
	PDX2(-0.08285f),
	PDX3(0.0f),
	PEX1(0.11113f),
	PEX2(0.3143f),
	PEX3(0.0f),
	PEX4(0.001719f),
	PKX1(21.687f),
	PKX2(13.728f),
	PKX3(-0.4098f),
	PHX1(0.00021615f),
	PHX2(0.0011598f),
	PVX1(0.000020283f),
	PVX2(0.00010568f),
	PPX1(-0.3485f),
	PPX2(0.37824f),
	PPX3(-0.09603f),
	PPX4(0.06518f),
	//pacejka lateral
	PCY1(1.338f),
	PDY1(0.8785f),
	PDY2(-0.06452f),
	PDY3(0.0f),
	PEY1(-0.8057f),
	PEY2(-0.6046f),
	PEY3(0.09854f),
	PEY4(-6.697f),
	PEY5(0.0f),
	PKY1(-15.324f),
	PKY2(1.715f),
	PKY3(0.3695f),
	PKY4(2.0005f),
	PKY5(0.0f),
	PKY6(-0.8987f),
	PKY7(-0.23303f),
	PHY1(-0.001806f),
	PHY2(0.0035f),
	PVY1(-0.00661f),
	PVY2(0.03592f),
	PVY3(-0.162f),
	PVY4(-0.4864f),
	PPY1(-0.6255f),
	PPY2(-0.06523f),
	PPY3(-0.16666f),
	PPY4(0.2811f),
	PPY5(0.0f),
	//pacejka combined longitudinal
	RBX1(13.046f),
	RBX2(9.718f),
	RBX3(0.0f),
	RCX1(0.9995f),
	REX1(-0.4403f),
	REX2(-0.4663f),
	RHX1(-0.00009968f),
	//pacejka combined lateral
	RBY1(10.622f),
	RBY2(7.82f),
	RBY3(0.002037f),
	RBY4(0.0f),
	RCY1(1.0587f),
	REY1(0.3148f),
	REY2(0.004867f),
	RHY1(0.009472f),
	RHY2(0.009754f),
	RVY1(0.05187f),
	RVY2(0.0004853f),
	RVY3(0.0f),
	RVY4(94.63f),
	RVY5(1.8914f),
	RVY6(23.8f),
	//pacejka overturning moment
	QSX1(-0.007764f),
	QSX2(1.1915f),
	QSX3(0.013948f),
	QSX4(4.912f),
	QSX5(1.02f),
	QSX6(22.83f),
	QSX7(0.7104f),
	QSX8(-0.023393f),
	QSX9(0.6581f),
	QSX10(0.2824f),
	QSX11(5.349f),
	QSX12(0.0f),
	QSX13(0.0f),
	QSX14(0.0f),
	PPMX1(0.0f),
	//pacejka rolling resistance moment
	QSY1(0.00702f),
	QSY2(0.0f),
	QSY3(0.001515f),
	QSY4(0.00008514f),
	QSY5(0.0f),
	QSY6(0.0f),
	QSY7(0.9008f),
	QSY8(-0.4089f),
	VREF(16.67f),
	//pacejka aligning moment
	QBZ1(12.035f),
	QBZ2(-1.33f),
	QBZ3(0.0f),
	QBZ4(0.176f),
	QBZ5(-0.14853f),
	QBZ9(34.5f),
	QBZ10(0.0f),
	QCZ1(1.2923f),
	QDZ1(0.09068f),
	QDZ2(-0.00565f),
	QDZ3(0.3778f),
	QDZ4(0.0f),
	QDZ6(0.0017015f),
	QDZ7(-0.002091f),
	QDZ8(-0.1428f),
	QDZ9(0.00915f),
	QDZ10(0.0f),
	QDZ11(0.0f),
	QEZ1(-1.7924f),
	QEZ2(0.8975f),
	QEZ3(0.0f),
	QEZ4(0.2895f),
	QEZ5(-0.6786f),
	QHZ1(0.0014333f),
	QHZ2(0.0024087f),
	QHZ3(0.24973f),
	QHZ4(-0.21205f),
	PPZ1(-0.4408f),
	PPZ2(0.0f),
	//pacejka combined aligning moment
	SSZ1(0.00918f),
	SSZ2(0.03869f),
	SSZ3(0.0f),
	SSZ4(0.0f)
{
	updatePreviewCurve();
}

void UFGearTireMF61::init()
{
	Super::init();
	mCombineMode = FGearForceCombineMode::SUM;
}

void UFGearTireMF61::calculate(float load, float slipRatio, float slipAngle, float camber, float pressure, float radius, float Vx)
{
	if (load <= 0.0f)
	{
		mLongitudinalForce = mLateralForce = 0.0f;
		mCombinedTorque = FVector::ZeroVector;
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

	float a = -slipAngle; //sign convention
	float ai = FMath::Tan(a);
	float k = slipRatio;
	float y = camber;
	float yi = FMath::Sin(y);
	float Fz = load;
	float Fz0 = mNominalLoad;
	float dFz = (load - Fz0) / Fz0;
	float Pi = pressure;
	float Pi0 = mNominalPressure;
	float dPi = (Pi - Pi0) / Pi0;

	//pure longitudinal
	float Fx0, Kxk;
	{
		float ux = (PDX1 + PDX2 * dFz) * (1.0f + PPX3 * dPi + PPX4 * dPi * dPi) * (1.0f - PDX3 * y * y);
		float Dx = FMath::Max(ux * Fz, 0.0f); //>0
		float Cx = FMath::Max(PCX1, 0.0f); //>0
		Kxk = Fz * (PKX1 + PKX2 * dFz) * FMath::Exp(PKX3 * dFz) * (1.0f + PPX1 * dPi + PPX2 * dPi * dPi);
		float Bx = Kxk / (Cx * Dx + FGearUtility::epsilon);
		float Shx = PHX1 + PHX2 * dFz;
		float Kx = k + Shx;
		float Ex = (PEX1 + PEX2 * dFz + PEX3 * dFz * dFz) * (1.0f - PEX4 * FMath::Sign(Kx));
		Ex = FMath::Min(Ex, 1.0f); //<=1
		float Svx = Fz * (PVX1 + PVX2 * dFz);
		Fx0 = Dx * FMath::Sin(Cx * FMath::Atan(Bx * Kx - Ex * (Bx * Kx - FMath::Atan(Bx * Kx)))) + Svx;
	}

	//pure lateral
	float Fy0, By, Cy, Kya, Shy, Svy, Dyk;
	{
		float uy = (PDY1 + PDY2 * dFz) * (1.0f + PPY3 * dPi + PPY4 * dPi * dPi) * (1.0f - PDY3 * yi * yi);
		float Dy = uy * Fz;
		Cy = FMath::Max(PCY1, 0.0f); //>0
		Kya = PKY1 * Fz0 * (1.0f + PPY1 * dPi) * (1.0f - PKY3 * FMath::Abs(yi)) * FMath::Sin(PKY4 * FMath::Atan((Fz / Fz0) / ((PKY2 + PKY5 * yi * yi) * (1.0f + PPY2 * dPi))));
		float Kyy = Fz * (PKY6 + PKY7 * dFz) * (1.0f + PPY5 * dPi);
		By = Kya / (Cy * Dy + FGearUtility::epsilon);
		float Svyy = Fz * (PVY3 + PVY4 * dFz) * yi;
		Shy = (PHY1 + PHY2 * dFz) + (Kyy * yi - Svyy) / (Kya + FGearUtility::epsilon);
		float Ay = ai + Shy;
		float Ey = (PEY1 + PEY2 * dFz) * (1.0f + (PEY5 * yi * yi) - (PEY3 + PEY4 * yi) * FMath::Sign(Ay));
		Ey= FMath::Min(Ey, 1.0f); //<=1
		Svy = Fz * (PVY1 + PVY2 * dFz) + Svyy;
		Fy0 = Dy * FMath::Sin(Cy * FMath::Atan(By * Ay - Ey * (By * Ay - FMath::Atan(By * Ay)))) + Svy;
		Dyk = Dy;
	}

	//combined Gx
	float Gx;
	{
		float Bxa = FMath::Max((RBX1 + RBX3 * yi * yi) * FMath::Cos(FMath::Atan(RBX2 * k)), 0.0f); //>0
		float Cxa = RCX1;
		float Shxa = RHX1;
		float As = ai + Shxa;
		float Exa = FMath::Min(REX1 + REX2 * dFz, 1.0f); //<=1
		float G0 = FMath::Cos(Cxa * FMath::Atan(Bxa * Shxa - Exa * (Bxa * Shxa - FMath::Atan(Bxa * Shxa))));
		Gx = FMath::Cos(Cxa * FMath::Atan(Bxa * As - Exa * (Bxa * As - FMath::Atan(Bxa * As)))) / G0;
		Gx = FMath::Max(Gx, 0.0f); //>0
	}

	//combined Gy
	float Gy;
	{
		float Byk = FMath::Max((RBY1 + RBY4 * yi * yi) * FMath::Cos(FMath::Atan(RBY2 * (ai - RBY3))), 0.0f); //>0
		float Cyk = RCY1;
		float Shyk = RHY1 + RHY2 * dFz;
		float Ks = k + Shyk;
		float Eyk = FMath::Min(REY1 + REY2 * dFz, 1.0f); //<=1
		float G0 = FMath::Cos(Cyk * FMath::Atan(Byk * Shyk - Eyk * (Byk * Shyk - FMath::Atan(Byk * Shyk))));
		Gy = FMath::Cos(Cyk * FMath::Atan(Byk * Ks - Eyk * (Byk * Ks - FMath::Atan(Byk * Ks)))) / G0;
		Gy = FMath::Max(Gy, 0.0f); //>0
	}

	//combined Svyk
	float Svyk;
	{
		float Dvyk = Dyk * (RVY1 + RVY2 * dFz + RVY3 * yi) * FMath::Cos(FMath::Atan(RVY4 * ai));
		Svyk = Dvyk * FMath::Sin(RVY5 * FMath::Atan(RVY6 * k));
	}

	//sum up forces
	float Fx = Gx * Fx0;
	float Fy = Gy * Fy0 + Svyk;
	mLongitudinalForce = Fx * lngSign;
	mLateralForce = Fy * latSign;

	//overturning couple
	float Mx;
	{
		float l1 = QSX1 - QSX2 * y * (1.0f + PPMX1 * dPi) + (QSX3 * Fy / Fz0);
		float iv = FMath::Atan(QSX6 * Fz / Fz0);
		float l2 = QSX4 * FMath::Cos(QSX5 * iv * iv) * FMath::Sin(QSX7 * y + QSX8 * FMath::Atan(QSX9 * Fy / Fz0));
		float l3 = QSX10 * FMath::Atan(QSX11 * Fz / Fz0) * y;
		Mx = Fz * radius * (l1 + l2 + l3);
	}

	//rolling resistance
	float My;
	{
		float l1 = QSY1 + QSY2 * Fx / Fz0 + QSY3 * FMath::Abs(Vx / VREF) + QSY4 * FMath::Pow(Vx / VREF, 4) + (QSY5 + QSY6 * Fz / Fz0) * y * y;
		float l2 = FMath::Pow(Fz / Fz0, QSY7) * FMath::Pow(Pi / Pi0, QSY8);
		My = Fz * radius * l1 * l2;
	}

	//combined Mz
	float Mz;
	{
		float signVcx = FMath::Sign(Vx) * -1.0f; //sign convention

		//pneumatic trail
		float Sht = QHZ1 + QHZ2 * dFz + (QHZ3 + QHZ4 * dFz) * yi;
		float at = ai + Sht;
		float eq = (Kxk / Kya) * (Kxk / Kya) * k * k;
		float ateq = FMath::Sqrt(at * at + eq) * FMath::Sign(at);
		float t = pneumaticTrail(ateq, a, yi, radius, Fz, dFz, dPi, signVcx);

		//residual moment
		float Shf = Shy + Svy / (Kya + FGearUtility::epsilon);
		float ar = ai + Shf;
		float areq = FMath::Sqrt(ar * ar + eq) * FMath::Sign(ar);
		float Mzr = residualMoment(areq, a, yi, radius, Fz, dFz, By, Cy, dPi, signVcx);

		float s = radius * (SSZ1 + SSZ2 * (Fy / Fz0) + (SSZ3 + SSZ4 * dFz) * yi);

		float F1y = Fy - Svyk;
		float M1z = -t * F1y;
		Mz = (M1z + Mzr + s * Fx) * latSign;
	}

	//combine torques
	mCombinedTorque.Set(Mx, My, Mz);
}

float UFGearTireMF61::pneumaticTrail(float at, float slipAngle, float yi, float radius, float Fz, float dFz, float dPi, float signVcx)
{
	float Fz0 = mNominalLoad;
	float Bt = (QBZ1 + QBZ2 * dFz + QBZ3 * dFz * dFz) * (1.0f + QBZ4 * FMath::Abs(yi) + QBZ5 * yi * yi);
	Bt = FMath::Max(Bt, 0.0f); //>0
	float Ct = FMath::Max(QCZ1, 0.0f); //>0
	float R0 = radius;
	float Dt0 = Fz * (R0 / Fz0) * (QDZ1 + QDZ2 * dFz) * (1.0f - PPZ1 * dPi) * signVcx;
	float Dt = Dt0 * (1.0f + QDZ3 * FMath::Abs(yi) + QDZ4 * yi * yi);
	float Et = (QEZ1 + QEZ2 * dFz + QEZ3 * dFz * dFz) * (1.0f + (QEZ4 + QEZ5 * yi) * (2.0f / PI) * FMath::Atan(Bt * Ct * at));
	Et = FMath::Min(Et, 1.0f); //<=1
	float t0 = Dt * FMath::Cos(Ct * FMath::Atan(Bt * at - Et * (Bt * at - FMath::Atan(Bt * at)))) * FMath::Cos(slipAngle);
	return t0;
}

float UFGearTireMF61::residualMoment(float ar, float slipAngle, float yi, float radius, float Fz, float dFz, float By, float Cy, float dPi, float signVcx)
{
	float Br = QBZ9 + QBZ10 * By * Cy;
	float Cr = 1.0f;
	float R0 = radius;
	float Dr = Fz * R0 * ((QDZ6 + QDZ7 * dFz) + ((QDZ8 + QDZ9 * dFz) * (1.0f + PPZ2 * dPi) + (QDZ10 + QDZ11 * dFz) * FMath::Abs(yi)) * yi) * signVcx * FMath::Cos(slipAngle);
	float Mzr = Dr * FMath::Cos(Cr * FMath::Atan(Br * ar));
	return Mzr;
}