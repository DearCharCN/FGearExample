// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearSpline.h"
#include "Runtime/Engine/Classes/Components/SplineComponent.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "FGearUtility.h"

AFGearSpline::AFGearSpline() :
	mSplineComp(nullptr),
	mDebug(false),
	mDebugColor(FColor::Blue),
	mDebugThickness(3.0f),
	mDebugIterator(3.0f)
{
	PrimaryActorTick.bCanEverTick = false;

	mSplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("Spline Component"));
	mSplineComp->SetClosedLoop(true);
	SetRootComponent(mSplineComp);
}

float AFGearSpline::getClosestParam(FVector p) const
{
	const float inputKey = mSplineComp->FindInputKeyClosestToWorldLocation(p);
	const int pointIndex = static_cast<int32>(inputKey);

	float lowDistBound = mSplineComp->GetDistanceAlongSplineAtSplinePoint(pointIndex);
	float highDistBound = mSplineComp->GetDistanceAlongSplineAtSplinePoint(pointIndex + 1);
	float middleDistEstimate = (lowDistBound + highDistBound) * 0.5f;

	const FInterpCurveFloat& distToInputMapping = mSplineComp->SplineCurves.ReparamTable;
	for (int i = 0; i < 10; ++i)
	{
		const float middleInputKey = distToInputMapping.Eval(middleDistEstimate);

		if (inputKey < middleInputKey) highDistBound = middleDistEstimate;
		else lowDistBound = middleDistEstimate;
		middleDistEstimate = (lowDistBound + highDistBound) * 0.5f;
	}
	return middleDistEstimate;
}

FVector AFGearSpline::moveParam(float & p, float distance)
{
	p = FMath::Fmod(p + distance, mSplineComp->GetSplineLength());
	return mSplineComp->GetLocationAtDistanceAlongSpline(p, ESplineCoordinateSpace::World);
}

FVector AFGearSpline::getPoint(float p) const
{
	return mSplineComp->GetLocationAtDistanceAlongSpline(p, ESplineCoordinateSpace::World);
}

FVector AFGearSpline::getTangent(float p) const
{
	return mSplineComp->GetTangentAtDistanceAlongSpline(p, ESplineCoordinateSpace::World);
}

FVector AFGearSpline::getDirection(float p) const
{
	return mSplineComp->GetDirectionAtDistanceAlongSpline(p, ESplineCoordinateSpace::World);
}

void AFGearSpline::BeginPlay()
{
	Super::BeginPlay();
	if (!mDebug) return;
	float len = mSplineComp->GetSplineLength();
	for (float i = 0.0f; i < len; i += mDebugIterator * FGearUtility::meter2cm)
	{
		DrawDebugLine(GetWorld(),
			mSplineComp->GetLocationAtDistanceAlongSpline(i, ESplineCoordinateSpace::World),
			mSplineComp->GetLocationAtDistanceAlongSpline(FMath::Min(len, i + mDebugIterator * FGearUtility::meter2cm), ESplineCoordinateSpace::World),
			mDebugColor,
			true,
			-1.0f,
			0,
			mDebugThickness);
	}
}