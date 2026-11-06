// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearAeroDynamics.h"
#include "FGearVehicle.h"

UFGearAeroDynamics::UFGearAeroDynamics() :
	UFGearTelemetryDrawer(),
	mDragCoefficient(0.8f),
	mDownForceCoefficient(0.2f),
	mDownForceXOffset(0.0f)
{
}

void UFGearAeroDynamics::init(AFGearVehicle * v)
{
	mVehicle = v;
	mBody = v->getBody();
	setWindowHeight(65);
}

void UFGearAeroDynamics::myFixedUpdate()
{
	if (mBody == nullptr) return;

	float v = FGearUtility::getWorldVelocity(mBody).Size() * FGearUtility::cm2meter;
	bool allowSubstep = !mVehicle->isSubstepping();

	//frontal drag
	mDragForce = -0.5f * mAirDensity * (v * v) * mDragCoefficient;
	FVector velDir = FGearUtility::getWorldVelocity(mBody).GetSafeNormal();
	mVehicle->addForce(mDragForce * FGearUtility::meter2cm * velDir);

	//down force
	mDownForce = 0.5f * mAirDensity * (v * v) * mDownForceCoefficient;
	FTransform trans = FGearUtility::getWorldTransform(mBody);
	FVector dir = trans.TransformVectorNoScale(FVector::ForwardVector);
	FVector pos = trans.GetLocation() + (mDownForceXOffset * FGearUtility::meter2cm * dir);
	mVehicle->addForceAtPosition(mDownForce * FGearUtility::meter2cm * FVector::DownVector, pos);
}

FVector2D UFGearAeroDynamics::drawTelemetry(UCanvas * Canvas, UFont * Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p)
{
	p = Super::drawTelemetry(Canvas, Font, textColor, renderInfo, p);
	Canvas->SetLinearDrawColor(textColor);
	Canvas->DrawText(Font, "AeroDynamics", p.X, p.Y, 1.0f, 1.0f, renderInfo);
	FGearUtility::drawLine(Canvas, p.X, p.Y + 18, p.X + 87, p.Y + 18, textColor);
	Canvas->DrawText(Font, FString::Printf(TEXT("Drag Force: %.0f Nm"), mDragForce), p.X, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Down Force: %.0f Nm"), mDownForce), p.X, p.Y + 35, 1.0f, 1.0f, renderInfo);
	p.Y += getWindowHeight();
	return p;
}