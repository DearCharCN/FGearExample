// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearWheel.h"
#include "FGearVehicle.h"
#include "FGearAxle.h"
#include "FGearTire.h"
#include "FGearWheelOptions.h"
#include "FGearEngine.h"
#include "FGearTransmission.h"
#include "FGearAnimInstance.h"
#include "FGearSkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include <limits>

UFGearWheel::UFGearWheel() :
	UFGearTelemetryDrawer(),
	mIsActive(true),
	mConvexShape(nullptr),
	mSpringCoeff(1.0f),
	mCompressedLength(0.0f),
	mLastCompressLength(0.0f),
	mCompressRatio(0.0f),
	mCurrentLoad(0.0f),
	mHasContact(false),

	mCurrentSteer(0.0f),
	mToeAngle(0.0f),
	mCamberAngle(0.0f),
	mPitch(0.0f),

	mLngSlipOverride(0.0f),
	mFrictionFactor(1.0f),
	mLatFrictionForce(1.0f),
	mLngFrictionForce(1.0f),

	mInherentDrag(0.0f),
	mAdditionalDrag(0.0f)
{
}

#if WITH_EDITOR
bool UFGearWheel::CanEditChange(const FProperty* Property) const
{
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearWheel, mSide))
	{
		return GetAttachParent() == nullptr;
	}
	return Super::CanEditChange(Property);
}
#endif

void UFGearWheel::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (mConvexShape != nullptr && mConvexShape->IsValidLowLevel())
	{
		mConvexShape->DestroyComponent();
		mConvexShape = nullptr;
	}
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UFGearWheel::init(AFGearVehicle* v, UFGearAxle* axle)
{
	mVehicle = v;
	mAxle = axle;
	mBody = mVehicle->getBody();
	mAnim = mVehicle->getAnim();
	refreshParameters();
	setWindowHeight(0);
	reset();
}

void UFGearWheel::refreshParameters()
{
	if (mAxle == nullptr) return;

	//get options
	mTire = mAxle->getWheelOptions()->getTireModel();
	mLatFriction = mAxle->getWheelOptions()->getLateralFriction();
	mLngFriction = mAxle->getWheelOptions()->getLongitudinalFriction();

	mLatScale = mAxle->getWheelOptions()->getLateralScale();
	mLngScale = mAxle->getWheelOptions()->getLongitudinalScale();
	mOverturnScale = mAxle->getWheelOptions()->getOverturnScale();
	mRollScale = mAxle->getWheelOptions()->getRollMomentScale();
	mAlignScale = mAxle->getWheelOptions()->getSelfAlignScale();

	mRadius = mAxle->getWheelOptions()->getRadius();
	mWidth = mAxle->getWheelOptions()->getWidth();
	mForwardOffset = mAxle->getWheelOptions()->getForwardOffset();
	mSpacerOffset = mAxle->getWheelOptions()->getSpacerOffset();
	mRimOffset = mAxle->getWheelOptions()->getRimOffset();
	mCastOffset = mAxle->getWheelOptions()->getCastOffset();
	mMass = mAxle->getWheelOptions()->getMass();
	mPressure = FMath::Max(2.0f * FGearUtility::epsilon, mAxle->getWheelOptions()->getTirePressure());

	mMaxBrakeTorque = mAxle->getWheelOptions()->getBrakeTorque();
	mSuspensionUpTravel = mAxle->getWheelOptions()->getSuspensionUpTravel();
	mSuspensionDownTravel = mAxle->getWheelOptions()->getSuspensionDownTravel();
	mSuspensionSpring = mAxle->getWheelOptions()->getSuspensionSpring();
	mCompressionDamper = mAxle->getWheelOptions()->getCompressionDamper();
	mRelaxationDamper = mAxle->getWheelOptions()->getRelaxationDamper();
	mSuspensionPreload = mAxle->getWheelOptions()->getSuspensionPreload();
	mSuspensionPreload = (mSuspensionPreload / 100.0f) * (mSuspensionUpTravel + mSuspensionDownTravel);

	mToeAngle = (mSide == FGearWheelSide::WS_LEFT ? 1.0f : -1.0f) * mAxle->getToeAngle();
	mCamberAngle = (mSide == FGearWheelSide::WS_LEFT ? 1.0f : -1.0f) * mAxle->getCamberAngle();

	//set radius & calculate dependent params
	setRadius(mRadius);

	//apply offsets & store local hub position
	//calls updateComponentTransform inside
	setLateralOffsets(mForwardOffset, mSpacerOffset, mRimOffset);
}

void UFGearWheel::initConvexCaster()
{
	//fail cases
	if (mVehicle->getQueryType() != FGearQueryType::Convex) return;
	UStaticMesh* convexAsset = mVehicle->getConvexAsset();
	if (convexAsset == nullptr) return; // return if fail

	//create
	if (mConvexShape == nullptr)
	{
		mConvexShape = NewObject<UStaticMeshComponent>(this);
		mConvexShape->RegisterComponent();
		mConvexShape->AttachToComponent(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, true));
		mConvexShape->SetStaticMesh(convexAsset);
		mConvexShape->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		mConvexShape->SetCollisionObjectType(mVehicle->getTraceChannel());
		mConvexShape->SetGenerateOverlapEvents(false);
		mConvexShape->bAlwaysCreatePhysicsState = true; //necessary for sweep test
		mConvexShape->SetHiddenInGame(!mVehicle->getConvexVisible());
	}

	//set local transform
	setConvexTransform();
}

void UFGearWheel::setConvexTransform()
{
	if (mConvexShape)
	{
		FTransform t(mVehicle->getConvexRotation().Quaternion(), (mRadius * FGearUtility::meter2cm) * FVector::UpVector);
		switch (mVehicle->getConvexUp())
		{
			case FGearUpDirection::X_UP: t.SetScale3D(FVector(mWidth, mRadius / 0.5f, mRadius / 0.5f)); break;
			case FGearUpDirection::Y_UP: t.SetScale3D(FVector(mRadius / 0.5f, mWidth, mRadius / 0.5f)); break;
			case FGearUpDirection::Z_UP: t.SetScale3D(FVector(mRadius / 0.5f, mRadius / 0.5f, mWidth)); break;
		}
		mConvexShape->SetRelativeTransform(t);
	}
}

void UFGearWheel::updateComponentTransform()
{
	if (mBoneName == NAME_None) return;
	FTransform trans = mVehicle->getBoneTransform(mBoneName);
	//apply suspension up/down travel changes
	FVector upOffset = mSuspensionUpTravel * FGearUtility::meter2cm * FVector::UpVector;
	//apply lateral offset changes
	FVector fwdOffset = mForwardOffset * FGearUtility::meter2cm * FVector::ForwardVector;
	FVector sideVector = mSide == FGearWheelSide::WS_LEFT ? FVector::LeftVector : FVector::RightVector;
	FVector sideOffset = (mSpacerOffset + mRimOffset) * FGearUtility::meter2cm * sideVector;
	//apply root bone transform
	FTransform rootTrans = mVehicle->getMesh()->GetBoneTransform(0, FTransform::Identity);
	FVector rootOffset = rootTrans.GetLocation();
	//set location
	trans.SetLocation(trans.GetLocation() + upOffset + fwdOffset + sideOffset - rootOffset);
	SetRelativeTransform(trans);
	//store hub position
	mLocalHubPosition = GetRelativeTransform().GetLocation();
}

//called when vehicle is reset
//called when wheel is inactive
void UFGearWheel::reset()
{
	//forces
	mSpringForce = mLateralForce = mLongitudinalForce = mCombinedTorque = FVector::ZeroVector;
	mLocalVelocity = mGroundVelocity = FVector::ZeroVector;
	//suspension
	mSpringCoeff = 1.0f;
	mCompressedLength = mLastCompressLength = mCompressRatio = mCurrentLoad = 0.0f;
	mHasContact = false;
	//steering
	mCurrentSteer = mPitch = 0.0f;
	mSteerQuat = FQuat::Identity;
	//friction
	mLateralSlip = mLongitudinalSlip = mSlipRatio = mSlipAngle = mLngSlipOverride = 0.0f;
	mFrictionFactor = 1.0f;
	mLatFrictionForce = mLngFrictionForce = 0.0f;
	//drag
	mInherentDrag = mAdditionalDrag = 0.0f;
	//feedback
	mFeedbackTorque = 0.0f;
	mABSActive = mASRActive = mHandbrakeActive = false;
	//others
	mRealSpeed = mWheelTorque = mBrakeTorque = mBraking = mSpeedOverflow = 0.0f;
}

void UFGearWheel::myFixedUpdate(float dt, float dts, int step, FVector& force, FVector& torque)
{
	if (!mIsActive)
	{
		reset();
		force = torque = FVector::ZeroVector;
		return;
	}

	if (step == 0) //first step
	{
		updatePhysicsTransform(); //for chaos
		suspensionUpdate(dt);
		updateLocalVelocity();
		stickyTireUpdate();
	}

	updateWheelTorque();

	updateWheelSpeed(dts);

	updateSlipValues();

	updateTireFrictionValues();

	updateWheelFriction(dts);

	//combination force
	FVector frictionForce = FVector::ZeroVector;
	if (mTire->getCombineMode() == FGearForceCombineMode::SIMPLE) frictionForce = combineLegacy();
	else if (mTire->getCombineMode() == FGearForceCombineMode::GRIP) frictionForce = combinePOR();
	else frictionForce = combineSum();

	//scale tire forces when sticky tire is in effect
	if (mVehicle->getStickyTireState() < 1.0f)
	{
		mLateralForce *= mVehicle->getStickyTireState();
		if (mBraking > 0.0f || mVehicle->isHandbrakeOn())
		{
			mLongitudinalForce *= mVehicle->getStickyTireState();
		}
	}

	//sum up with suspension
	force = mSpringForce + frictionForce + calcWheelDragForce(dts);

	//torque from tire model + tire reaction torque for x component
	torque = mCombinedTorque;

	//fix rolling resistance torque sign
	if (torque.Y != 0.0f) torque.Y = FMath::Sign(mRealSpeed) * FMath::Abs(torque.Y);

	//add wheel torque feedback, when braking take brake disc radius into account(~%30)
	torque.Y += mLongitudinalForce.Size() * -FMath::Sign(mWheelTorque) * (mBraking > 0.0f ? 0.3f : 1.0f) * mRadius;

	//scale torque
	torque.Y *= mRollScale;
	torque.Z *= -mAlignScale;
	torque.X *= -mOverturnScale;

	//convert local torque to global
	FQuat qs = FGearUtility::Euler(0.0f, mCurrentSteer + mToeAngle, 0.0f);
	torque = qs * mPhysicsTrans.TransformVector(torque);
}

FVector UFGearWheel::combineLegacy()
{
	if (mLongitudinalForce.Size() > 0.0f && mCurrentLoad > 0.0f)
	{
		float maxLongitudinalForce = mTire->getMaxLongitudinalForce() * mFrictionFactor * mLngFriction * 2.0f;
		float midVal = 1.0f - FMath::Pow(mLongitudinalForce.Size() / maxLongitudinalForce, 2.0f);
		if (midVal > 0.0f)
		{
			float lateralSize = mLateralForce.Size() * FMath::Sqrt(midVal);
			mLateralForce = lateralSize * mLateralForce.GetSafeNormal();
		}
	}
	return mLongitudinalForce + mLateralForce;
}

FVector UFGearWheel::combinePOR()
{
	if (mP == 0.0f) mS = mA = mP = 1.0f; //p is zero for the first frame
	FVector totalTireForce = (mS / mP * mLongitudinalForce) + (mA / mP * mLateralForce);
	return totalTireForce;
}

FVector UFGearWheel::combineSum()
{
	return mLongitudinalForce + mLateralForce;
}

void UFGearWheel::visualsUpdate(float dt)
{
	if (mIsActive && mAnim != nullptr)
	{
		//rotate acc. to wheel speed
		mPitch += mRealSpeed * dt;
		
		//clamp mpitch
		static float pi2 = 2.0f * PI;
		if (mPitch > pi2) mPitch -= pi2; else if (mPitch < -pi2) mPitch += pi2;

		//update anim bp
		mAnim->update(getAxle()->getIndex(), mSide == FGearWheelSide::WS_LEFT, getWheelPosition(), getWheelRotation().Rotator());
	}
}

//instead of using GetComponentXXX we need physics transform for chaos
void UFGearWheel::updatePhysicsTransform()
{
	mPhysicsTrans = GetRelativeTransform() * FGearUtility::getWorldTransform(mBody);
}

void UFGearWheel::suspensionUpdate(float dt)
{
	mSpringForce = FVector::ZeroVector;
	mCurrentLoad = 0.0f;

	//default wheel pos
	float totalLength = mSuspensionUpTravel + mSuspensionDownTravel;
	mCompressedLength = totalLength;

	//common
	float castDist = FGearUtility::meter2cm * (totalLength + mRadius + mCastOffset);
	float minDist = castDist;
	float hitCount = 0;
	FVector avgNormal = FVector::ZeroVector;
	FVector upVector = mPhysicsTrans.TransformVectorNoScale(FVector::UpVector);
	mSteerQuat = FQuat(upVector, FMath::DegreesToRadians(mCurrentSteer + mToeAngle));
	FQuat wheelRot = mSteerQuat * FGearUtility::getWorldTransform(mBody).GetRotation();
	
	//trace params
	FCollisionQueryParams traceParams;
	traceParams.MobilityType = (EQueryMobilityType)mVehicle->getQueryMobilityType();
	traceParams.bReturnPhysicalMaterial = true;
	traceParams.AddIgnoredActor(mVehicle);
	traceParams.IgnoreMask = mVehicle->getIgnoreMask();
	traceParams.bTraceComplex = mVehicle->getTraceComplex();
	ECollisionChannel castMask = mVehicle->getTraceChannel();

	//line casting
	if (mVehicle->getQueryType() == FGearQueryType::Line)
	{
		//ray scan
		int rays = mVehicle->getRayCount();
		int lrays = mVehicle->getLateralRayCount();
		FVector dirFrom = mSteerQuat * -upVector;
		float maxAngle = 0.0f;
		float stepAngle = 1.0f;
		float startX = 0.0f;
		float stepX = 1.0f;
		float endX = 0.0f;

		if (rays > 1)
		{
			maxAngle = 90.0f;
			float yOffsetAngle = FMath::RadiansToDegrees(FMath::Atan2(mCompressedLength, mRadius));
			maxAngle -= yOffsetAngle;
			stepAngle = (2.0f * maxAngle) / (rays - 1);
		}

		if (lrays > 1)
		{
			startX = -0.5f * mWidth;
			endX = 0.5f * mWidth;
			stepX = mWidth / (lrays - 1);
		}

		//raycast
		for (float x = startX; x <= endX; x += stepX)
		{
			FVector rightVector = mPhysicsTrans.TransformVectorNoScale(FVector::RightVector);
			FVector addx = x * FGearUtility::meter2cm * rightVector;
			for (float angle = -maxAngle; angle <= (maxAngle + 0.001f); angle += stepAngle)
			{
				FVector cdir = dirFrom.RotateAngleAxis(angle, mSteerQuat * rightVector);
				FVector origin = getHubPosition() + addx - (mCastOffset * FGearUtility::meter2cm) * cdir;
				FHitResult hit;
				if (GetWorld() && GetWorld()->LineTraceSingleByChannel(hit, origin, origin + castDist * cdir, castMask, traceParams))
				{
					//avoid self collision / check block only
					if (hit.GetActor() == GetOwner() || hit.bBlockingHit == 0) continue;
					//check closest
					float hitDistance = hit.Distance * FGearUtility::cm2meter;
					if (hitDistance < minDist)
					{
						minDist = hitDistance;
						mRayHit = hit;
					}
					avgNormal += hit.Normal;
					hitCount++;
				}
			}
		}
	}
	//sphere casting
	else if (mVehicle->getQueryType() == FGearQueryType::Sphere)
	{
		FVector cdir = -upVector;
		FVector origin = getHubPosition() - ((mCastOffset + mRadius) * FGearUtility::meter2cm) * cdir;
		TArray<struct FHitResult> hits;
		if (GetWorld() && GetWorld()->SweepMultiByChannel(hits, origin, origin + castDist * cdir, FQuat::Identity, castMask, mColShape, traceParams))
		{
			//for each hit
			for (int i = 0; i < hits.Num(); i++)
			{
				FHitResult hit = hits[i];
				//avoid self collision / check block only
				if (hit.GetActor() == GetOwner() || hit.bBlockingHit == 0) continue;
				//prevent reverse direction hits
				if (FVector::DotProduct(cdir, hit.Normal) > 0.0f) continue;
				//check closest
				float hitDistance = hit.Distance * FGearUtility::cm2meter;
				if (hitDistance < minDist)
				{
					minDist = hitDistance;
					mRayHit = hit;
				}
				avgNormal += hit.Normal;
				hitCount++;
			}
		}
	}
	//convex casting
	else if (mVehicle->getQueryType() == FGearQueryType::Convex)
	{
		if (mConvexShape)
		{
			FVector cdir = -upVector;
			FVector origin = getHubPosition() - ((mCastOffset + mRadius) * FGearUtility::meter2cm) * cdir;
			FQuat qshape = FGearUtility::Euler(0.0f, mCurrentSteer + mToeAngle, 0.0f);
			qshape = qshape * mConvexShape->GetComponentQuat();

			FComponentQueryParams sweepParams;
			sweepParams.MobilityType = (EQueryMobilityType)mVehicle->getQueryMobilityType();
			sweepParams.bFindInitialOverlaps = false;
			sweepParams.bReturnPhysicalMaterial = true;
			sweepParams.AddIgnoredActor(mVehicle);
			sweepParams.IgnoreMask = mVehicle->getIgnoreMask();
			sweepParams.bTraceComplex = mVehicle->getTraceComplex();

			TArray<FHitResult> hits;
			if (GetWorld() && GetWorld()->ComponentSweepMulti(hits, mConvexShape, origin, origin + castDist * cdir, qshape, sweepParams))
			{
				//for each hit
				for (int i = 0; i < hits.Num(); i++)
				{
					FHitResult hit = hits[i];
					//avoid self collision / check block only
					if (hit.GetActor() == GetOwner() || hit.bBlockingHit == 0) continue;
					//prevent reverse direction hits
					if (FVector::DotProduct(cdir, hit.Normal) > 0.0f) continue;
					//check closest
					float hitDistance = hit.Distance * FGearUtility::cm2meter;
					if (hitDistance < minDist)
					{
						minDist = hitDistance;
						mRayHit = hit;
					}
					avgNormal += hit.Normal;
					hitCount++;
				}
			}
		}
	}

	mHasContact = hitCount > 0;

	if (mHasContact)
	{
		//avg. dir
		avgNormal /= hitCount;
		//eliminate lateral forces for volume casts
		if (mVehicle->getQueryType() != FGearQueryType::Line)
		{
			avgNormal = wheelRot.Inverse() * avgNormal;
			avgNormal.Y = 0.0f; avgNormal.Normalize();
			avgNormal = wheelRot * avgNormal;
		}
		//find compressed length & wheel pos
		mCompressedLength = minDist - mRadius - mCastOffset;
		mCompressedLength = FMath::Min(mCompressedLength, totalLength);
		//calc. spring force acc. to compress ratio
		mCompressRatio = (totalLength - mCompressedLength) / totalLength;
		float realCompress = mCompressRatio;
		mCompressRatio = FMath::Clamp(mCompressRatio, 0.0f, 1.0f);
		//normal spring + preload
		float compressMM = 1000.0f * (totalLength - mCompressedLength);
		float preloadMagnitude = (compressMM != 0.0f ? 1000.0f : 0.0f) * mSuspensionPreload * mSuspensionSpring;
		float springMagnitude = (preloadMagnitude + (compressMM * mSuspensionSpring)) * mSpringCoeff;
		FVector springForce = springMagnitude * avgNormal;
		//keep track of delta compress for damping
		float deltaCompress = mCompressedLength - mLastCompressLength;
		mLastCompressLength = mCompressedLength;
		//calc. damping force
		float compressSpeed = deltaCompress / dt;
		float dampingMagnitude = 0.0f;
		if (deltaCompress < 0.0f) dampingMagnitude = compressSpeed * mCompressionDamper;
		else dampingMagnitude = compressSpeed * mRelaxationDamper;
		FVector dampingForce = dampingMagnitude * avgNormal;
		//sum all
		mCurrentLoad = springMagnitude;
		mSpringForce = springForce - dampingForce;
		//avoid negative force if no RelaxationDownforce
		if (!mVehicle->getRelaxationDownforce() && FVector::DotProduct(mSpringForce, avgNormal) < 0)
		{
			mSpringForce = FVector::ZeroVector;
		}
	}
	else mLastCompressLength = totalLength;
}

//low speed sticky tire hack
//when below minSpeed zero out lateral components of suspension
void UFGearWheel::stickyTireUpdate()
{
	if (!mHasContact) return;

	if (mVehicle->getStickyTireState() < 1.0f)
	{
		//eliminate lateral spring force
		FVector localSpring = mSpringForce;
		float lateralSize = 0.0f;
		//inverse rotate
		float rz = FGearUtility::getWorldTransform(mBody).Rotator().Euler().Z;
		FQuat q(FVector::UpVector, FMath::DegreesToRadians(rz));
		localSpring = q.Inverse() * localSpring;
		//zero out laterals (close to zero, some magic numbers here)
		lateralSize += FMath::Abs(localSpring.Y);
		localSpring.Y = 0.0f;
		if (mBraking > 0.0f || mVehicle->isHandbrakeOn())
		{
			lateralSize += FMath::Abs(localSpring.X);
			localSpring.X = 0.0f;
		}
		//back to original rotation
		mSpringForce = q * localSpring;
		//add slope factor
		FVector upVector = mPhysicsTrans.TransformVectorNoScale(FVector::UpVector);
		float angle = FGearUtility::Angle(upVector, FVector::UpVector);
		angle = FMath::Clamp(angle, -80.0f, 80.0f); //clamp for tangent
		float tanAngle = FMath::Tan(FMath::DegreesToRadians(angle));
		float verticalSize = tanAngle * lateralSize;
		mSpringForce.Z += verticalSize;
	}
}

void UFGearWheel::updateLocalVelocity()
{
	mGroundVelocity = FGearUtility::getWorldVelocityAtPoint(mBody, getHubPosition());
	//detect ground is enabled and sitting on a moving object
	if (mVehicle->getDetectGround() && mRayHit.GetComponent() && mRayHit.GetComponent()->GetBodyInstance())
	{
		FBodyInstance* hitBody = mRayHit.GetComponent()->GetBodyInstance();
		mGroundVelocity -= FGearUtility::getWorldVelocityAtPoint(hitBody, mRayHit.Location);
	}
	mLocalVelocity = mPhysicsTrans.InverseTransformVector(mGroundVelocity * FGearUtility::cm2meter);
	if ((mCurrentSteer + mToeAngle) != 0.0f)
	{
		FQuat qs = FGearUtility::Euler(0.0f, -mCurrentSteer - mToeAngle, 0.0f);
		mLocalVelocity = qs * mLocalVelocity;
	}
}

void UFGearWheel::updateSlipValues()
{
	//lateral slip
	mLateralSlip = mHasContact ? -mLocalVelocity.Y : 0.0f;

	//longitudinal slip
	float wheelSpeed = mRealSpeed * mRadius;
	mLongitudinalSlip = mHasContact ? wheelSpeed - mLocalVelocity.X : 0.0f;

	//lng slip override
	if (FMath::Abs(mLongitudinalSlip) < FMath::Abs(mLngSlipOverride))
	{
		mLongitudinalSlip = mLngSlipOverride;
	}

	//slip ratio
	float absForwardVelocity = FMath::Abs(mLocalVelocity.X);
	mSlipRatio = mLongitudinalSlip / (absForwardVelocity + FGearUtility::epsilon);

	//slip angle
	mSlipAngle = FMath::Atan(mLocalVelocity.Y / (absForwardVelocity + FGearUtility::epsilon));

	//s - a - p
	if (mTire->getCombineMode() == FGearForceCombineMode::GRIP)
	{
		mS = FMath::Abs(mSlipRatio / mTire->getMaxLongitudinalForceSlip());
		mA = FMath::Abs(mSlipAngle / mTire->getMaxLateralForceAngle());
		mP = FMath::Sqrt(mS * mS + mA * mA);
		mSlipRatio = FMath::Sign(mSlipRatio) * mP * mTire->getMaxLongitudinalForceSlip();
		mSlipAngle = FMath::Sign(mSlipAngle) * mP * mTire->getMaxLateralForceAngle();
	}

	//limit slip ratio/angle
	mSlipRatio = FMath::Clamp(mSlipRatio, -1.0f, 1.0f);
	mSlipAngle = FMath::Clamp(mSlipAngle, -0.475f * PI, 0.475f * PI); //avoid extremas
}

void UFGearWheel::updateTireFrictionValues()
{
	if (mSlipRatio == 0.0f) mSlipRatio = mTire->getMaxLongitudinalForceSlip(); //vehicle may not move at first
	mTire->calculate(mCurrentLoad, mSlipRatio, mSlipAngle, FMath::DegreesToRadians(mCamberAngle), mPressure, mRadius, mLocalVelocity.X);
	mLngFrictionForce = mTire->getLongitudinalForce();
	mLatFrictionForce = mTire->getLateralForce();
	mCombinedTorque = mTire->getCombinedTorque();

	//additional factors : direct friction coeff, lng/lat friction coeff, 2x magic
	mLngFrictionForce *= mFrictionFactor * mLngFriction * 2.0f;
	mLatFrictionForce *= mFrictionFactor * mLatFriction * 2.0f;
}

void UFGearWheel::updateWheelTorque()
{
	mWheelTorque = 0.0f;
	mBrakeTorque = 0.0f;
	mASRActive = false;
	mABSActive = false;
	mHandbrakeActive = false;

	//sign of initial speed
	float iSign = FMath::Sign(mRealSpeed);

	//fall back to velocity sign if zero
	if (iSign == 0.0f) iSign = FMath::Sign(mLocalVelocity.X);

	//apply brake torque
	float brakeTorque = mBraking * mMaxBrakeTorque;
	if (mBraking > 0.0f)
	{
		//ABS?
		mHandbrakeActive = mVehicle->isHandbrakeOn() && mAxle->hasHandBrake();
		mABSActive = mHasContact && !mHandbrakeActive && mVehicle->getABS() > 0.0f && FMath::Abs(getKMHSpeed()) >= mVehicle->getMinAbsSpeed();

		//apply
		mBrakeTorque -= iSign * brakeTorque;
	}

	//apply engine torque
	float axleTorque = mAxle->getTorque(this) / mRadius;
	if (axleTorque != 0.0f)
	{
		//asr is set to inactive for engine brake case, this avoids irrelevant asr activation at low speeds
		bool engineBrake = mVehicle->getEngine()->getThrottle() == 0.0f;
		mASRActive = mHasContact && !engineBrake && mVehicle->getASR() > 0.0f;

		//apply
		mWheelTorque += axleTorque;
	}

	//apply rolling friction
	float frictionTorque = mAxle->getDrivetrainFriction(); //constant friction
	frictionTorque += 0.1f * mVehicle->getRollingResistanceCoeff() * FMath::Abs(mRealSpeed);
	if (frictionTorque != 0.0f)
	{
		mBrakeTorque -= iSign * frictionTorque;
	}

	//sum all
	mWheelTorque += mBrakeTorque;
}

void UFGearWheel::updateWheelSpeed(float dt)
{
	//acceleration coeff. instead of (torque / momentum)
	float inertia = 0.5f * (mMass * (mRadius * mRadius)) + mVehicle->getDrivetrainInertia();
	float accCoeff = 1.0f / inertia;

	//take engine inertia into account
	float engineInertia = mVehicle->getEngine()->getEngineInertia() * mVehicle->getTransmission()->getTransmissionRatio();
	engineInertia = engineInertia * mAxle->getTorqueShare() / mRadius;
	float engineCoeff = 1.0f / (inertia + FMath::Abs(engineInertia));

	//acting forces
	float absLngFrictionTorque = FMath::Abs(mLngFrictionForce) * mRadius;
	float absWheelTorque = FMath::Abs(mWheelTorque);
	float netTorque = absWheelTorque - absLngFrictionTorque;

	//ground correction
	if (mHasContact)
	{
		float groundDelta = (getGroundSpeed() - mRealSpeed) * mVehicle->getSimCoeff();
		//engine coeff is only effective when slowing down
		bool skipEngineCoeff = FGearUtility::Sign(groundDelta) == FGearUtility::Sign(mRealSpeed);
		float angularAcc = absLngFrictionTorque * (skipEngineCoeff ? accCoeff : engineCoeff);
		float deltaV = FGearUtility::Sign(groundDelta) * FMath::Min(angularAcc * dt, FMath::Abs(groundDelta));
		mRealSpeed += deltaV;
	}
	//just set to target speed when off ground
	else if (mAxle->getTorque(this) != 0.0f)
	{
		//lerp to zero when breaking
		if (mBraking > 0.0f)
		{
			mRealSpeed = FMath::Lerp(mRealSpeed, 0.0f, 10.0f * dt);
			return;
		}
		//keep at rpm speed
		else if (mVehicle->getEngine()->getRpm() > 10.0f + mVehicle->getEngine()->getIdleRpm())
		{
			mRealSpeed = mAxle->getCurRpm() * FGearUtility::rpm2Rads;
			return;
		}
	}

	//override slip with max force slip
	float optimalSlip = FMath::Abs(mLocalVelocity.X) * mTire->getMaxLongitudinalForceSlip();
	float optimalSlipState = mVehicle->getOptimumSlipState();
	if (mASRActive || mABSActive) //asr & abs requires optimal slip
	{
		optimalSlipState = FMath::Max(optimalSlipState, FMath::Max(mVehicle->getASR(), mVehicle->getABS()));
		mVehicle->setOptimumSlipState(optimalSlipState);
	}
	mLngSlipOverride = FMath::Lerp(0.0f, FGearUtility::Sign(mWheelTorque) * optimalSlip, optimalSlipState);

	//torque rotation
	if (netTorque > 0.0f)
	{
		//target speed, accelerate is false when moving backward with forward gear + throttle
		bool accelerate = FMath::Sign(mWheelTorque) != FMath::Sign(mBrakeTorque); //signum
		float targetSpeed = accelerate ? mAxle->getCurRpm() * FGearUtility::rpm2Rads : 0.0f;

		//update target speed
		if (mWheelTorque > 0.0f && mRealSpeed > targetSpeed)
		{
			targetSpeed = mRealSpeed;
		}
		else if (mWheelTorque < 0.0f && mRealSpeed < targetSpeed)
		{
			targetSpeed = mRealSpeed;
		}

		//calc. wheel acceleration
		//take engine inertia into account while engine braking
		accCoeff = accelerate || mBraking > 0.0f ? accCoeff : engineCoeff;
		float angularAcc = FMath::Sign(mWheelTorque) * netTorque * accCoeff; //signum
		float deltaV = angularAcc * dt;

		//ABS, ASR, handbrake overrides
		if (accelerate)
		{
			//this check avoids unnecessary slips over low friction surfaces at low speeds
			bool engineBrake = mVehicle->getEngine()->getThrottle() == 0.0f;
			bool lowSpeedCase = engineBrake && FMath::Abs(mRealSpeed) < 1.0f;
			bool activate = lowSpeedCase || mASRActive;

			//set actual asr state
			mASRActive &= FMath::Abs(deltaV * mVehicle->getASR()) > optimalSlip;

			//scale deltaV
			if (activate)
			{
				float scaler = 1.0f - (lowSpeedCase ? 1.0f : mVehicle->getASR());
				//no zero. reason: failing high engine friction case
				deltaV *= FMath::Max(scaler, 0.001f);
			}
		}
		else if (!accelerate)
		{
			//set actual abs state
			bool activate = mABSActive;
			mABSActive &= FMath::Abs(deltaV * mVehicle->getABS()) > optimalSlip;

			if (activate)
			{
				float scaler = 1.0f - mVehicle->getABS();
				deltaV *= FMath::Max(scaler, 0.001f); //no zero
			}
			else if (mHandbrakeActive)
			{
				deltaV *= mVehicle->getHandbrakePower();
			}
		}

		//apply deltaV, clamp with target speed
		mRealSpeed += deltaV;
		if ((deltaV > 0.0f && mRealSpeed > targetSpeed) || (deltaV < 0.0f && mRealSpeed < targetSpeed))
		{
			mRealSpeed = targetSpeed;
			//if we don't do this slip override will also generate optimal forces
			//that means we can't reduce acceleration by using high engine inertia
			mLngSlipOverride = 0.0f;
		}
	}
	else
	{
		//asr & abs requires positive net torque
		mASRActive = false;
		mABSActive = false;
	}

	//rpm limit to rotation speed
	if (hasDrive()) applyRpmLimit(dt);
}

void UFGearWheel::applyRpmLimit(float dt)
{
	//two types of limit needed
	//1 that only limits mRealSpeed
	//1 that applies braking to limit speed
	float maxSpeedKMH = mVehicle->getWheelSpeedLimitKMH();
	if (maxSpeedKMH == 0.0f) return;
	float realSpeedKMH = getKMHSpeed();
	float clutch = mVehicle->getTransmission()->getClutchState();
	bool allowReverse = mVehicle->getAllowReverse();

	//if overflow is the same sign as limit, then set tire speed to limit
	if (FGearUtility::Sign(maxSpeedKMH) == FGearUtility::Sign(realSpeedKMH))
	{
		if (FMath::Abs(realSpeedKMH) > FMath::Abs(maxSpeedKMH))
		{
			float targetRealSpeedKMH = FMath::Lerp(realSpeedKMH, maxSpeedKMH, clutch);
			mRealSpeed = targetRealSpeedKMH / (FGearUtility::rads2Rpm * mWheelRPMToKMH);
		}
	}
	//if overflow is not the same sign as limit, then set tire speed to 0
	else if (!allowReverse) mRealSpeed = FMath::Lerp(mRealSpeed, 0.0f, clutch);

	//any need for a braking limit
	mSpeedOverflow = 0.0f;
	float absSpeedKMH = FMath::Abs(mLocalVelocity.X) * FGearUtility::ms2kmh;
	
	if (mHasContact && absSpeedKMH > dt && maxSpeedKMH != 0.0f)
	{
		if (FGearUtility::Sign(maxSpeedKMH) == FGearUtility::Sign(mLocalVelocity.X))
		{
			mSpeedOverflow = absSpeedKMH - FMath::Abs(maxSpeedKMH);
		}
		else if (!allowReverse) mSpeedOverflow = 1.0f;

		if (mSpeedOverflow > 0.0f)
		{
			mSpeedOverflow = clutch * FMath::Clamp(mSpeedOverflow, 0.0f, 1.0f);
		}
	}
}

void UFGearWheel::updateWheelFriction(float dt)
{
	static float gravitySize = FMath::Abs(FGearUtility::cm2meter * GetWorld()->GetGravityZ());
	static FVector gravity = gravitySize * FVector::UpVector;

	mLateralForce = FVector::ZeroVector;
	mLongitudinalForce = FVector::ZeroVector;

	if (mHasContact)
	{
		float m = mCurrentLoad / gravitySize;

		//lateral force
		//automatically avoids low speed jitters
		{
			float absLatFriction = FMath::Abs(mLatFrictionForce);
			//required acc. to stop dv/dt
			float a = mLateralSlip / dt;
			//required f for acc. f=ma
			float f = m * a * mVehicle->getSimCoeff();
			//clamp
			f = FMath::Clamp(f, -absLatFriction, absLatFriction);
			//apply
			mLateralForce = mLatScale * f * getGlobalRight();
		}

		//longitudinal force
		//limit wheel torque with max-friction
		float absLngFriction = FMath::Abs(mLngFrictionForce);
		float currentForce = FMath::Clamp(mWheelTorque, -absLngFriction, absLngFriction);

		//for powered wheels, tire friction will override on sign mismatches
		//this solves a set of problems where vehicle moves against the tire direction
		if (mAxle->getTorqueShare() > 0.0f && FMath::Sign(mWheelTorque) != FMath::Sign(mLngFrictionForce)) //signum
		{
			//some limitations needed, friction force is unstable when speed-slip approaches zero
			float limitAlpha = FMath::Clamp(FMath::Abs(0.01f * getKMHSpeed()), 0.0f, 1.0f);
			float slipLimit = FMath::Lerp(1.0f, 0.1f, limitAlpha);
			if (FMath::Abs(getKMHSpeed()) > 1.0f && FMath::Abs(mSlipRatio) > slipLimit)
			{
				currentForce = mLngFrictionForce;
			}
			//left out for rare cases (very large rear gear ratio)
			//if enabled, will break 8wd full steer launch case
			//else currentForce = 0.0f;
		}

		//keeps vehicle from going faster than engine rpm limit
		//this is an artificial force so take clutch power into account here
		//mSpeedOverflow is multiplied with clutch power above in applyRpmLimit
		if (mSpeedOverflow > 0.0f)
		{
			currentForce = FMath::Sign(-mLocalVelocity.X) * absLngFriction * mSpeedOverflow;
		}

		//automatically avoids low speed jitters
		//this smooths the lng. force but using this for engine torque may result low traction
		//so this is only activated when braking
		if (mBraking > 0.0f)
		{
			float absCurrentForce = FMath::Abs(currentForce);
			//required acc. to stop dv/dt
			float a = mLongitudinalSlip / dt;
			//required f for acc. f=ma
			float f = m * a * mVehicle->getSimCoeff();
			//clamp
			f = FMath::Clamp(f, -absCurrentForce, absCurrentForce);
			//apply
			mLongitudinalForce = mLngScale * f * getGlobalForward();
			//set feedback torque
			mFeedbackTorque = (FMath::Abs(currentForce) + absLngFriction) * mRadius;
		}
		else
		{
			mLongitudinalForce = mLngScale * currentForce * getGlobalForward();
			//set feedback torque
			mFeedbackTorque = FMath::Max(absLngFriction, mFrictionFactor * mCurrentLoad) * mRadius;
		}
	}
	else mFeedbackTorque = FMath::Abs(mWheelTorque);
}

//calculate a pseudo force that can be applied to simulate a dragging wheel
FVector UFGearWheel::calcWheelDragForce(float dt)
{
	if (mHasContact && (mGroundVelocity.X != 0.0f || mGroundVelocity.Y != 0.0f))
	{
		FVector horizontalVel = mGroundVelocity;
		horizontalVel.Z = 0.0f;
		float dragFactor = mInherentDrag + mAdditionalDrag;
		if (dragFactor < 0.0f) dragFactor = 0.0f;
		float widthFactor = mWidth / 0.3f; //30 cm width assumed as average
		float loadFactor = mCurrentLoad / 3000.0f; //300kg load assumed as average
		return -dragFactor * widthFactor * loadFactor * mGroundVelocity.Size() * horizontalVel.GetUnsafeNormal();
	}
	return FVector::ZeroVector;
}

bool UFGearWheel::hasDrive() const
{
	return mAxle->getTorqueShare() > 0.0f;
}

FVector UFGearWheel::getHubPosition() const
{
	return FGearUtility::getWorldTransform(mBody).TransformPosition(mLocalHubPosition);
}

FVector UFGearWheel::getWheelPosition() const
{
	//update wheel z-pos acc. to compress length
	float compress = mVehicle->getTirePenetration() ? FMath::Max(0.0f, mCompressedLength) : mCompressedLength;
	FVector wheelPos = -FGearUtility::meter2cm * (compress - mSuspensionUpTravel) * FVector::UpVector;
	//update wheel x-pos acc. to forward offset
	if (mForwardOffset != 0.0f) wheelPos += FGearUtility::meter2cm * mForwardOffset * FVector::ForwardVector;
	//update wheel y-pos acc. to spacer offset
	if (mSpacerOffset != 0.0f) wheelPos += FGearUtility::meter2cm * (mSide == FGearWheelSide::WS_LEFT ? -mSpacerOffset : mSpacerOffset) * FVector::RightVector;
	//ret
	return wheelPos;
}

FQuat UFGearWheel::getWheelRotation() const
{
	//rolling angle
	FQuat qp = FGearUtility::Euler(-FMath::RadiansToDegrees(mPitch), 0.0f, 0.0f);
	//rotate acc. to camber angle
	FQuat qc = FGearUtility::Euler(0.0f, 0.0f, mCamberAngle);
	//rotate acc. to steering & toe
	FQuat qst = FGearUtility::Euler(0.0f, mCurrentSteer + mToeAngle, 0.0f);
	//ret
	return qst * qc * qp;
}

//used for slip angle calculation
FVector UFGearWheel::getGlobalVelocity() const
{
	FQuat qs = FGearUtility::Euler(0.0f, mCurrentSteer + mToeAngle, 0.0f);
	FVector gvel = qs * mLocalVelocity;
	return mPhysicsTrans.TransformVector(gvel);
}

//used from esp calculation, in vehicle space
FVector UFGearWheel::getLocalVelocity() const
{
	FQuat qs = FGearUtility::Euler(0.0f, mCurrentSteer + mToeAngle, 0.0f);
	return qs * mLocalVelocity;
}

//used for slip angle and longitudinal force calculation
FVector UFGearWheel::getGlobalForward() const
{
	FQuat qs = FGearUtility::Euler(0.0f, mCurrentSteer + mToeAngle, 0.0f);
	FVector forwardVector = mPhysicsTrans.TransformVectorNoScale(FVector::ForwardVector);
	return qs * forwardVector;
}

//used for lateral force calculation, camber is ignored
FVector UFGearWheel::getGlobalRight() const
{
	FQuat qs = FGearUtility::Euler(0.0f, mCurrentSteer + mToeAngle, 0.0f);
	FVector rightVector = mPhysicsTrans.TransformVectorNoScale(FVector::RightVector);
	return qs * rightVector;
}

//used for antirollbars
//warning : if called from a substep update, set allowSubstep to false
void UFGearWheel::applyVerticalForce(float force)
{
	if (hasContact())
	{
		FVector upVector = mPhysicsTrans.TransformVectorNoScale(FVector::UpVector);
		mVehicle->addForceAtPosition(force * upVector, getHubPosition());
	}
}

//individual setter for radius
void UFGearWheel::setRadius(float f)
{
	mRadius = f;
	//extra params
	mCircumference = 2.0f * PI * mRadius;
	mWheelRPMToKMH = mCircumference * 60.0f / 1000.0f;
	//create sphere collider
	mColShape = FCollisionShape::MakeSphere(mRadius * FGearUtility::meter2cm);
	initConvexCaster();
}

//individual setter for forward/spacer/rim offset
//recalculates local hub postion
void UFGearWheel::setLateralOffsets(float forward, float spacer, float rim)
{
	mForwardOffset = forward;
	mSpacerOffset = spacer;
	mRimOffset = rim;
	updateComponentTransform();
}

void UFGearWheel::setBoneName(const FString& NewName)
{
	mBoneName = FName(NewName);
	updateComponentTransform();
}

//individual setter for suspension preload
//given percentage% value is converted to actual length
//make sure you have set suspension travel lengths before setting this
void UFGearWheel::setSuspensionPreload(float f)
{
	mSuspensionPreload = f;
	mSuspensionPreload = (mSuspensionPreload / 100.0f) * (mSuspensionUpTravel + mSuspensionDownTravel);
}

void UFGearWheel::saveState(FFGearVehicleState& state)
{
	if (state.mActive.Num() == 0)
	{
		int ct = 2 * mVehicle->getAxleCount();
		state.mActive.SetNum(ct);
		state.mLastCompressLength.SetNum(ct);
		state.mPitch.SetNum(ct);
		state.mRealSpeed.SetNum(ct);
	}
	int i = mAxle->getIndex();
	int j = mSide == FGearWheelSide::WS_LEFT ? 0 : 1;
	int index = i * 2 + j;
	state.mActive[index] = mIsActive;
	state.mLastCompressLength[index] = mLastCompressLength;
	state.mPitch[index] = mPitch;
	state.mRealSpeed[index] = mRealSpeed;
}

void UFGearWheel::loadState(FFGearVehicleState& state)
{
	if (state.mActive.Num() > 0)
	{
		int i = mAxle->getIndex();
		int j = mSide == FGearWheelSide::WS_LEFT ? 0 : 1;
		int index = i * 2 + j;
		mIsActive = state.mActive[index];
		mLastCompressLength = state.mLastCompressLength[index];
		mPitch = state.mPitch[index];
		mRealSpeed = state.mRealSpeed[index];
	}
}

FVector2D UFGearWheel::drawTelemetry(UCanvas * Canvas, UFont * Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p)
{
	Canvas->DrawText(Font, FString::Printf(TEXT("%d%s"), mAxle->getIndex(), mSide == FGearWheelSide::WS_LEFT ? TEXT("L") : TEXT("R")), p.X, p.Y + 15, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("%.1f"), getKMHSpeed()), p.X + 23, p.Y + 15, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("%.0f"), mWheelTorque), p.X + 70, p.Y + 15, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("%.0f"), mCurrentLoad), p.X + 120, p.Y + 15, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("%.2f"), mCompressedLength * 100.0f), p.X + 180, p.Y + 15, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("%.0f"), mCurrentSteer), p.X + 263, p.Y + 15, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("%.1f/%.1f"), mLatFriction, mLngFriction), p.X + 335, p.Y + 15, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("%.2f"), mFrictionFactor), p.X + 440, p.Y + 15, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("%.0f"), mLongitudinalForce.Size()), p.X + 515, p.Y + 15, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("%.2f"), mLongitudinalSlip), p.X + 580, p.Y + 15, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("%.2f"), mSlipRatio), p.X + 630, p.Y + 15, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("%.0f"), mLateralForce.Size()), p.X + 690, p.Y + 15, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("%.2f"), mLateralSlip), p.X + 750, p.Y + 15, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("%.2f"), mSlipAngle), p.X + 800, p.Y + 15, 1.0f, 1.0f, renderInfo);
	p.Y += 15;
	return p;
}

void UFGearWheel::drawGizmo(FColor color)
{
	FVector v = mVehicle->getMesh()->GetBoneLocation(mBoneName, EBoneSpaces::WorldSpace);
	FVector rimOffset = (mSide == FGearWheelSide::WS_LEFT ? -mRimOffset : mRimOffset) * FGearUtility::meter2cm * GetRightVector();
	float halfWidth = mWidth * 0.5f * FGearUtility::meter2cm;
	DrawDebugCylinder(	GetWorld(),
						v + rimOffset - mSteerQuat * GetRightVector() * halfWidth,
						v + rimOffset + mSteerQuat * GetRightVector() * halfWidth,
						mRadius * FGearUtility::meter2cm,
						32,
						color,
						false,
						-1.0f,
						SDPG_MAX);
	
	//ray scan
	if (mVehicle->getQueryType() != FGearQueryType::Line) return;
	int rays = mVehicle->getRayCount();
	int lrays = mVehicle->getLateralRayCount();
	float castDist = FGearUtility::meter2cm * (mSuspensionUpTravel + mSuspensionDownTravel + mRadius + mCastOffset);
	FVector dirFrom = mSteerQuat * -GetUpVector();
	float maxAngle = 0.0f;
	float stepAngle = 1.0f;
	float startX = 0.0f;
	float stepX = 1.0f;
	float endX = 0.0f;

	if (rays > 1)
	{
		maxAngle = 90.0f;
		float yOffsetAngle = FMath::RadiansToDegrees(FMath::Atan2(mCompressedLength, mRadius));
		maxAngle -= yOffsetAngle;
		stepAngle = (2.0f * maxAngle) / (rays - 1);
	}

	if (lrays > 1)
	{
		startX = -0.5f * mWidth;
		endX = 0.5f * mWidth;
		stepX = mWidth / (lrays - 1);
	}

	//raycast
	for (float x = startX; x <= endX; x += stepX)
	{
		FVector addx = x * FGearUtility::meter2cm * GetRightVector();
		for (float angle = -maxAngle; angle <= (maxAngle + 0.001f); angle += stepAngle)
		{
			FVector cdir = dirFrom.RotateAngleAxis(angle, mSteerQuat * GetRightVector());
			FVector origin = getHubPosition() + addx - (mCastOffset * FGearUtility::meter2cm) * cdir;
			DrawDebugLine(GetWorld(), origin, origin + castDist * cdir, color, false, -1.0f, SDPG_MAX);
		}
	}
}
