// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearSuspensionConstraint.h"
#include "FGearVehicle.h"
#include "FGearAxle.h"
#include "FGearWheel.h"
#include "FGearSkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"

UFGearSuspensionConstraint::UFGearSuspensionConstraint() :
	UFGearTelemetryDrawer(),
	mIsEnabled(true),
	mVehicle(nullptr)
{
    mJTS.Empty();
}

void UFGearSuspensionConstraint::init(AFGearVehicle* v)
{
    if (!mIsEnabled) return;

	mVehicle = v;
	if (mVehicle == nullptr) return;

	FTransform rootTrans = mVehicle->getMesh()->GetBoneTransform(0, FTransform::Identity);
	FVector rootOffset = rootTrans.GetLocation();

    //add joint for each axle
    int axleCount = mVehicle->getAxleCount();
    mJTS.Init(nullptr, axleCount * 2);
    for (int i = 0; i < axleCount; i++)
    {
        FVector ptL = mVehicle->getAxle(i)->getLeftWheel()->getLocalHubPosition() + rootOffset;
        FVector ptR = mVehicle->getAxle(i)->getRightWheel()->getLocalHubPosition() + rootOffset;

        UPhysicsConstraintComponent* jtL = NewObject<UPhysicsConstraintComponent>(mVehicle, UPhysicsConstraintComponent::StaticClass());
        jtL->RegisterComponent();
        jtL->SetRelativeLocation(ptL);
        jtL->AttachToComponent(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
        jtL->SetConstrainedComponents(Cast<UPrimitiveComponent>(mVehicle->getMesh()), NAME_None, nullptr, NAME_None);

        UPhysicsConstraintComponent* jtR = NewObject<UPhysicsConstraintComponent>(mVehicle, UPhysicsConstraintComponent::StaticClass());
        jtR->RegisterComponent();
        jtR->SetRelativeLocation(ptR);
        jtR->AttachToComponent(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
        jtR->SetConstrainedComponents(Cast<UPrimitiveComponent>(mVehicle->getMesh()), NAME_None, nullptr, NAME_None);

        float limit = mVehicle->getAxle(i)->getWheelOptions()->getSuspensionUpTravel() * FGearUtility::meter2cm;

        jtL->ConstraintInstance.SetLinearXLimit(ELinearConstraintMotion::LCM_Free, limit);
        jtL->ConstraintInstance.SetLinearYLimit(ELinearConstraintMotion::LCM_Free, limit);
        jtL->ConstraintInstance.SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, limit);
        jtL->ConstraintInstance.UpdateLinearLimit();

        jtR->ConstraintInstance.SetLinearXLimit(ELinearConstraintMotion::LCM_Free, limit);
        jtR->ConstraintInstance.SetLinearYLimit(ELinearConstraintMotion::LCM_Free, limit);
        jtR->ConstraintInstance.SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, limit);
        jtR->ConstraintInstance.UpdateLinearLimit();

        // store in list
        mJTS[i * 2] = jtL;
        mJTS[i * 2 + 1] = jtR;
    }
}

void UFGearSuspensionConstraint::substepTick()
{
    if (!mIsEnabled) return;
    if (mJTS.Num() == 0) return;

    //update limits when wheels have contact
    for (int i = 0; i < mVehicle->getAxleCount(); i++)
    {
        int iL = i * 2;
        int iR = i * 2 + 1;

        UFGearAxle* iA = mVehicle->getAxle(i);
        UFGearWheel* wL = iA->getLeftWheel();
        UFGearWheel* wR = iA->getRightWheel();
        float upTravel = iA->getWheelOptions()->getSuspensionUpTravel() * FGearUtility::meter2cm;

        if (wL->hasContact())
        {
            mJTS[iL]->ConstraintInstance.SetLinearZMotion(ELinearConstraintMotion::LCM_Limited);
            //anchor limit pos
			FVector wLP = wL->getHubPosition();
            FVector wLPLimit = wL->getRayHit().ImpactPoint + (wL->getRadius() * FGearUtility::meter2cm + upTravel) * wL->GetUpVector();
            //limit condition
			FVector delta = wLP - wLPLimit;
			delta = delta.ProjectOnTo(wL->GetUpVector());
			if (FVector::DotProduct(delta, wL->GetUpVector()) < 0.0f) wLP -= delta;
			//apply
            mJTS[iL]->ConstraintInstance.SetRefPosition(EConstraintFrame::Type::Frame2, wLP);
        }
        else mJTS[iL]->ConstraintInstance.SetLinearZMotion(ELinearConstraintMotion::LCM_Free);

        if (wR->hasContact())
        {
            mJTS[iR]->ConstraintInstance.SetLinearZMotion(ELinearConstraintMotion::LCM_Limited);
			//anchor limit pos
			FVector wRP = wR->getHubPosition();
            FVector wRPLimit = wR->getRayHit().ImpactPoint + (wR->getRadius() * FGearUtility::meter2cm + upTravel) * wR->GetUpVector();
			//limit condition
			FVector delta = wRP - wRPLimit;
			delta = delta.ProjectOnTo(wR->GetUpVector());
			if (FVector::DotProduct(delta, wR->GetUpVector()) < 0.0f) wRP -= delta;
			//apply
            mJTS[iR]->ConstraintInstance.SetRefPosition(EConstraintFrame::Type::Frame2, wRP);
        }
        else mJTS[iR]->ConstraintInstance.SetLinearZMotion(ELinearConstraintMotion::LCM_Free);
    }
}

void UFGearSuspensionConstraint::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	for (int i = 0; i < mJTS.Num(); i++)
	{
		if (mJTS[i] != nullptr && mJTS[i]->IsValidLowLevel())
		{
			mJTS[i]->DestroyComponent();
			mJTS[i] = nullptr;
		}
	}
    mJTS.Empty();
    Super::OnComponentDestroyed(bDestroyingHierarchy);
}