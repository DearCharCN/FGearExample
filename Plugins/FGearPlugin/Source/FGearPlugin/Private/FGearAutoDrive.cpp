// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearAutoDrive.h"
#include "FGearVehicle.h"
#include "FGearEngine.h"
#include "FGearTransmission.h"
#include "FGearSpline.h"
#include "FGearUtility.h"
#include "FGearStandardInput.h"
#include "Engine/World.h"
#include "Curves/CurveFloat.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#if ENGINE_MAJOR_VERSION == 5
#include "Engine/HitResult.h"
#endif

AFGearAutoDrive::AFGearAutoDrive() :
	AAIController(),
	mVehicle(nullptr),
	mActive(false),
	mSpline(nullptr),
	mStuckResetTime(5.0f),
	mResetZOffset(0.5f),
	mSpeedByAngleCurve(nullptr),
	mSpeedCoeffCurve(nullptr),
	mSteeringDistance(10.0f),
	mBrakingDistance(12.0f),
	mSteeringLimit(60.0f),
	mForwardCheckDistance(2.5f),
	mSideCheckDistance(2.5f),
	mCrossCheckDistance(3.5f),
	mAvoidanceDistance(5.0f),
	mDebug(false),
	mDebugThickness(3.0f),
	mSteeringDebugColor(FColor::Green),
	mBrakingDebugColor(FColor::Red),
	mAvoidanceDebugColor(FColor::Purple),
	mThrottleInput(0.0f),
	mBrakeInput(0.0f),
	mSteeringInput(0.0f),
	mNoVelocityTime(0.0f),
	mLateralDelta(0.0f),
	mCurrentSplineParam(-1.0f)
{

}

void AFGearAutoDrive::BeginPlay()
{
	Super::BeginPlay();

	//speed by steer curve default values
	if (mSpeedByAngleCurve == nullptr) mSpeedByAngleCurve = NewObject<UCurveFloat>();
	if (mSpeedByAngleCurve->FloatCurve.GetNumKeys() <= 1)
	{
		mSpeedByAngleCurve->FloatCurve.AddKey(0.0f, 20.0f);
		mSpeedByAngleCurve->FloatCurve.AddKey(1.0f, 20.0f);
	}

	//speed coeff curve default values
	if (mSpeedCoeffCurve == nullptr) mSpeedCoeffCurve = NewObject<UCurveFloat>();
	if (mSpeedCoeffCurve->FloatCurve.GetNumKeys() <= 1)
	{
		mSpeedCoeffCurve->FloatCurve.AddKey(0.0f, 1.0f);
		mSpeedCoeffCurve->FloatCurve.AddKey(1.0f, 1.0f);
	}

	mTraceParams.MobilityType = EQueryMobilityType::Dynamic;
	mTraceParams.bReturnPhysicalMaterial = false;
}

void AFGearAutoDrive::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	mVehicle = Cast<AFGearVehicle>(InPawn);
	if (mVehicle)
	{
		//trace ignore
		mTraceParams.ClearIgnoredActors();
		mTraceParams.AddIgnoredActor(mVehicle);
		//reset inputs
		mVehicle->getStandardInput()->resetInputs();
		//pause input reading if active
		mVehicle->getStandardInput()->setReadInputs(!mActive);
		//should tick here so that we can properly override inputs
		mVehicle->onOverrideInput.AddUniqueDynamic(this, &AFGearAutoDrive::myTick);
	}
}

void AFGearAutoDrive::OnUnPossess()
{
	Super::OnUnPossess();

	if (mVehicle)
	{
		//reset inputs
		mVehicle->getStandardInput()->resetInputs();
		//unpause input reading
		mVehicle->getStandardInput()->setReadInputs(true);
		//stop input override
		mVehicle->onOverrideInput.RemoveDynamic(this, &AFGearAutoDrive::myTick);
		//clear vehicle ref
		mVehicle = nullptr;
	}
}

void AFGearAutoDrive::setActive(bool b)
{
	mActive = b;
	//pause input reading if active
	if (mVehicle)
	{
		mVehicle->getStandardInput()->setReadInputs(!mActive);
	}
}

void AFGearAutoDrive::myTick(float dt)
{
	if (mActive && mSpline != nullptr && mVehicle != nullptr)
	{
		if (mVehicle->GetLocalRole() == ENetRole::ROLE_Authority)
		{
			updateInputs(dt);
			applyInputs(dt);
		}
	}
}

void AFGearAutoDrive::updateInputs(float dt)
{
	//reset all
	mThrottleInput = 0.0f;
	mBrakeInput = 0.0f;
	mSteeringInput = 0.0f;

	//find target point
	float speedCoeff = mSpeedCoeffCurve->GetFloatValue(mVehicle->getKMHSpeed());
	mCurrentSplineParam = mSpline->getClosestParam(mVehicle->GetActorLocation());
	float targetParam = mCurrentSplineParam;
	FVector target = mSpline->moveParam(targetParam, speedCoeff * mSteeringDistance * FGearUtility::meter2cm);
	if (mDebug)
	{
		DrawDebugLine(GetWorld(), mVehicle->GetActorLocation(), target, mSteeringDebugColor, false, -1.0f, 0, mDebugThickness);
		DrawDebugSphere(GetWorld(), target, 15.0f, 16, mSteeringDebugColor, false, -1.0f, 0, mDebugThickness);
	}

	//update target if any opponent nearby
	updateRaycasts(dt, speedCoeff);
	target += mLateralDelta * mVehicle->GetActorRightVector();

	//find steering input
	FVector tdir = (target - mVehicle->GetActorLocation()).GetSafeNormal();
	FVector dir = mVehicle->GetActorForwardVector();
	float sign = FMath::Sign(FVector::CrossProduct(dir, tdir).Z);
	float tsteer = sign * FGearUtility::Angle(dir, tdir);
	mSteeringInput = FMath::Max(-1.0f, FMath::Min(1.0f, tsteer / mSteeringLimit));

	//find a target speed
	FVector side = mVehicle->GetActorRightVector();
	targetParam = mCurrentSplineParam;
	FVector brakingTarget = mSpline->moveParam(targetParam, mBrakingDistance * FGearUtility::meter2cm * speedCoeff); //for braking
	FVector futureSide = FQuat(FVector::UpVector, FMath::DegreesToRadians(90.0f)) * mSpline->getTangent(targetParam).GetSafeNormal();
	float angle = FGearUtility::Angle(side, futureSide);
	float targetSpeed = mSpeedByAngleCurve->GetFloatValue(angle);
	float deltaSpeed = targetSpeed - mVehicle->getKMHSpeed();
	if (mDebug)
	{
		DrawDebugLine(GetWorld(), mVehicle->GetActorLocation(), brakingTarget, mBrakingDebugColor, false, -1.0f, 0, mDebugThickness);
		DrawDebugSphere(GetWorld(), brakingTarget, 20.0f, 32, mBrakingDebugColor, false, -1.0f, 0, mDebugThickness);
	}

	//other inputs
	mBrakeInput = deltaSpeed < -10.0f ? ((-deltaSpeed - 10.0f) / 10.0f) : 0.0f;
	if (mBrakeInput > 1.0f) mBrakeInput = 1.0f;
	mThrottleInput = deltaSpeed > 0.0f ? 1.0f : 0.0f;
}

void AFGearAutoDrive::updateRaycasts(float dt, float speedCoeff)
{
	bool forwardObstacle = false;
	bool leftObstacle = false;
	bool rightObstacle = false;

	FVector fwd = mVehicle->GetActorForwardVector();
	FVector left = -mVehicle->GetActorRightVector();
	FVector right = mVehicle->GetActorRightVector();
	FVector crossLeft = (left + 1.5f * fwd).GetSafeNormal();
	FVector crossRight = (right + 1.5f * fwd).GetSafeNormal();

	float fwdDist = (mForwardCheckDistance * (1.0f + speedCoeff)) * FGearUtility::meter2cm;
	float sideDist = mSideCheckDistance * FGearUtility::meter2cm;
	float crossDist = mCrossCheckDistance * FGearUtility::meter2cm;

	FHitResult hit;
	FVector start = mVehicle->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	//forward
	if (mDebug) DrawDebugLine(GetWorld(), start, start + fwd * fwdDist, mAvoidanceDebugColor, false, -1.0f, 0, mDebugThickness);
	if (GetWorld()->LineTraceSingleByChannel(hit, start, start + fwd * fwdDist, ECollisionChannel::ECC_Pawn, mTraceParams))
	{
		if (hit.GetActor() != nullptr && hit.GetActor()->IsA(AFGearVehicle::StaticClass())) forwardObstacle = true;
	}

	//left
	if (mDebug) DrawDebugLine(GetWorld(), start, start + left * sideDist, mAvoidanceDebugColor, false, -1.0f, 0, mDebugThickness);
	if (GetWorld()->LineTraceSingleByChannel(hit, start, start + left * sideDist, ECollisionChannel::ECC_Pawn, mTraceParams))
	{
		if (hit.GetActor() != nullptr && hit.GetActor()->IsA(AFGearVehicle::StaticClass())) leftObstacle = true;
	}

	//cross left
	if (mDebug) DrawDebugLine(GetWorld(), start, start + crossLeft * crossDist, mAvoidanceDebugColor, false, -1.0f, 0, mDebugThickness);
	if (GetWorld()->LineTraceSingleByChannel(hit, start, start + crossLeft * crossDist, ECollisionChannel::ECC_Pawn, mTraceParams))
	{
		if (hit.GetActor() != nullptr && hit.GetActor()->IsA(AFGearVehicle::StaticClass())) leftObstacle = true;
	}

	//right
	if (mDebug) DrawDebugLine(GetWorld(), start, start + right * sideDist, mAvoidanceDebugColor, false, -1.0f, 0, mDebugThickness);
	if (GetWorld()->LineTraceSingleByChannel(hit, start, start + right * sideDist, ECollisionChannel::ECC_Pawn, mTraceParams))
	{
		if (hit.GetActor() != nullptr && hit.GetActor()->IsA(AFGearVehicle::StaticClass())) rightObstacle = true;
	}

	//cross right
	if (mDebug) DrawDebugLine(GetWorld(), start, start + crossRight * crossDist, mAvoidanceDebugColor, false, -1.0f, 0, mDebugThickness);
	if (GetWorld()->LineTraceSingleByChannel(hit, start, start + crossRight * crossDist, ECollisionChannel::ECC_Pawn, mTraceParams))
	{
		if (hit.GetActor() != nullptr && hit.GetActor()->IsA(AFGearVehicle::StaticClass())) rightObstacle = true;
	}

	if (forwardObstacle || leftObstacle) mLateralDelta = FMath::Lerp(mLateralDelta, mAvoidanceDistance * FGearUtility::meter2cm, 1.5f * dt);
	else if (rightObstacle) mLateralDelta = FMath::Lerp(mLateralDelta, -mAvoidanceDistance * FGearUtility::meter2cm, 1.5f * dt);
	else mLateralDelta = FMath::Lerp(mLateralDelta, 0.0f, 0.5f * dt);
}

void AFGearAutoDrive::applyInputs(float dt)
{
	//apply inputs
	mVehicle->getStandardInput()->setInputs(mThrottleInput, mBrakeInput, mSteeringInput, 0.0f, 0, false);

	//simple reset check : if stuck for 5 secs.
	if (mVehicle->getKMHSpeed() < 10.0f) mNoVelocityTime += dt; else mNoVelocityTime = 0.0f;
	if (mNoVelocityTime > mStuckResetTime)
	{
		mNoVelocityTime = 0.0f;
		FVector groundOffset(0.0f, 0.0f, mResetZOffset * FGearUtility::meter2cm);
		mVehicle->reset(mSpline->getPoint(mCurrentSplineParam) + groundOffset, mSpline->getDirection(mCurrentSplineParam).Rotation());
	}
}

#if WITH_EDITOR
bool AFGearAutoDrive::CanEditChange(const FProperty* Property) const
{
	const bool ParentVal = Super::CanEditChange(Property);
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(AFGearAutoDrive, mDebugThickness)) return mActive && mDebug;
	else if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(AFGearAutoDrive, mSteeringDebugColor)) return mActive && mDebug;
	else if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(AFGearAutoDrive, mBrakingDebugColor)) return mActive && mDebug;
	else if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(AFGearAutoDrive, mAvoidanceDebugColor)) return mActive && mDebug;
	return ParentVal;
}
#endif