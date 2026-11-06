// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearAxle.h"
#include "FGearWheelOptions.h"
#include "FGearWheel.h"
#include "FGearVehicle.h"
#include "FGearEngine.h"
#include "FGearTransmission.h" 
#include "FGearTire.h"

UFGearAxle::UFGearAxle() :
	UFGearTelemetryDrawer(),
	mDiffStrength(0.25f),
	mTorqueShare(0.5f),
	mFinalGearRatio(3.5f),
	mMaxSteerAngle(0.0f),
	mAckermanCoeff(1.0f),
	mAckermanReferenceIndex(1),
	mHasHandbrake(false),
	mCamberAngle(0.0f),
	mToeAngle(0.0f),
	mIndex(0),
	mAxleTorque(0.0f),
	mLeftTorque(0.0f),
	mRightTorque(0.0f),
	mCurrentSteerLeft(0.0f),
	mCurrentSteerRight(0.0f),
	mTargetSteer(0.0f)
{
}

void UFGearAxle::init(AFGearVehicle * v, int index)
{
	mIndex = index;
	mVehicle = v;
	mEngine = v->getEngine();
	mTransmission = v->getTransmission();
	mWheelOptions->checkNoTireState(); //check before wheels
	mWheelLeft->init(v, this);
	mWheelRight->init(v, this);
	setWindowHeight(140);
}

//called from Vehicle.Reset
void UFGearAxle::reset()
{
	mWheelOptions->reset();
	mTorqueShare = 0.5f;
	mMaxSteerAngle = 0.0f;
	mAckermanCoeff = 1.0f;
	mAckermanReferenceIndex = 1;
	mHasHandbrake = false;
	mDiffStrength = 0.25f;
	mCamberAngle = 0.0f;
	mToeAngle = 0.0f;
}

void UFGearAxle::applyWheelOptions()
{
	if (mWheelLeft != nullptr) mWheelLeft->refreshParameters();
	if (mWheelRight != nullptr) mWheelRight->refreshParameters();
}

void UFGearAxle::preWheelupdate(bool firstStep)
{
	if (firstStep) steeringUpdate();
	calculateAxleTorque();
}

void UFGearAxle::postWheelupdate()
{
	//force same speed of dominating wheel
	if (mDifferentialType == FGearDifferentialType::DF_LOCKED)
	{
		float leftFeed = mWheelLeft->getFeedbackTorque();
		float rightFeed = mWheelRight->getFeedbackTorque();
		if (leftFeed > rightFeed) mWheelRight->setSpeed(mWheelLeft->getSpeed());
		else if (leftFeed < rightFeed) mWheelLeft->setSpeed(mWheelRight->getSpeed());
	}
}

void UFGearAxle::steeringUpdate()
{
	if (hasSteering())
	{
		mCurrentSteerLeft = mTargetSteer;
		mCurrentSteerRight = mTargetSteer;

		//ackerman
		bool outerLeft = mCurrentSteerLeft != 0.0f && FGearUtility::Sign(mWheelLeft->getLocalHubPosition().Y) != FGearUtility::Sign(mCurrentSteerLeft);
		bool outerRight = mCurrentSteerRight != 0.0f && FGearUtility::Sign(mWheelRight->getLocalHubPosition().Y) != FGearUtility::Sign(mCurrentSteerRight);
		if (mAckermanCoeff != 0.0f && (outerLeft || outerRight))
		{
			//reversed case
			if (mMaxSteerAngle < 0.0f)
			{
				outerLeft = !outerLeft;
				outerRight = !outerRight;
			}
			float localTargetSteer = outerLeft ? mCurrentSteerLeft : mCurrentSteerRight;
			float d = getTrackWidth();
			float l = mVehicle->getAxleDistance(mIndex, mAckermanReferenceIndex);
			float absTarget = FMath::DegreesToRadians(FMath::Abs(localTargetSteer));
			float turnRadius = l / FMath::Tan(absTarget);
			float Y = FMath::RadiansToDegrees(FMath::Atan2(l, d + turnRadius));
			localTargetSteer = FGearUtility::Sign(localTargetSteer) * FMath::Abs(Y);
			if (outerLeft) mCurrentSteerLeft = FMath::Lerp(mTargetSteer, localTargetSteer, mAckermanCoeff);
			else mCurrentSteerRight = FMath::Lerp(mTargetSteer, localTargetSteer, mAckermanCoeff);
		}

		//apply
		mWheelLeft->setCurrentSteer(mCurrentSteerLeft);
		mWheelRight->setCurrentSteer(mCurrentSteerRight);
	}
}

void UFGearAxle::calculateAxleTorque()
{
	float engineTorque = mEngine->getTorque();
	float transRatio = mTransmission->getTransmissionRatio(this) * mTransmission->getClutchState();
	mAxleTorque = transRatio * engineTorque * mTorqueShare;
	mLeftTorque = mRightTorque = 0.0f;
	
	if (mAxleTorque == 0.0f) return;

	if (mDifferentialType == FGearDifferentialType::DF_OPEN)
	{
		mLeftTorque = mRightTorque = getOpenDiffTorque();
	}
	else if (mDifferentialType == FGearDifferentialType::DF_LSD)
	{
		float openTorque = getOpenDiffTorque();
		mLeftTorque = FMath::Lerp(openTorque, getLockedDiffTorque(mWheelLeft), mDiffStrength);
		mRightTorque = FMath::Lerp(openTorque, getLockedDiffTorque(mWheelRight), mDiffStrength);
	}
	else if (mDifferentialType == FGearDifferentialType::DF_LOCKED)
	{
		mLeftTorque = getLockedDiffTorque(mWheelLeft);
		mRightTorque = getLockedDiffTorque(mWheelRight);
	}
}

float UFGearAxle::getOpenDiffTorque()
{
	float leftFeed = mWheelLeft->hasContact() ? mWheelLeft->getFeedbackTorque() : 1.0f;
	float rightFeed = mWheelRight->hasContact() ? mWheelRight->getFeedbackTorque() : 1.0f;

	if (mAxleTorque > 0.0f) return FMath::Min(FMath::Min(leftFeed, rightFeed), 0.5f * mAxleTorque);
	else return FMath::Max(FMath::Max(-leftFeed, -rightFeed), 0.5f * mAxleTorque);
}

float UFGearAxle::getLockedDiffTorque(UFGearWheel* w)
{
	float leftFeed = mWheelLeft->hasContact() ? mWheelLeft->getFeedbackTorque() : 1.0f;
	float rightFeed = mWheelRight->hasContact() ? mWheelRight->getFeedbackTorque() : 1.0f;
	float totalFeed = leftFeed + rightFeed;
	float leftRatio = 0.5f;
	float rightRatio = 0.5f;
	if (totalFeed != 0.0f)
	{
		leftRatio = leftFeed / totalFeed;
		rightRatio = rightFeed / totalFeed;
		//limit torque bias ratio (9:1)
		if (mDifferentialType == FGearDifferentialType::DF_LSD)
		{
			leftRatio = FMath::Clamp(leftFeed / totalFeed, 0.1f, 0.9f);
			rightRatio = FMath::Clamp(rightFeed / totalFeed, 0.1f, 0.9f);
		}
	}

	if (mAxleTorque > 0.0f)
	{
		if (w == mWheelLeft) return FMath::Min(leftFeed, leftRatio * mAxleTorque);
		if (w == mWheelRight) return FMath::Min(rightFeed, rightRatio * mAxleTorque);
	}
	else if (mAxleTorque < 0.0f)
	{
		if (w == mWheelLeft) return FMath::Max(-leftFeed, leftRatio * mAxleTorque);
		if (w == mWheelRight) return FMath::Max(-rightFeed, rightRatio * mAxleTorque);
	}

	return 0.0f;
}

float UFGearAxle::getTorque(UFGearWheel* w)
{
	if (w == mWheelLeft) return mLeftTorque;
	if (w == mWheelRight) return mRightTorque;
	return 0.0f;
}

float UFGearAxle::getSteeringInput()
{
	if (hasSteering()) return mTargetSteer / mMaxSteerAngle;
	return 0.0f;
}

float UFGearAxle::getCurRpm() const
{
	float transRatio = mTransmission->getTransmissionRatio(this);
	if (mTorqueShare == 0.0f || transRatio == 0.0f) return 0.0f;
	return mEngine->getRpm() / transRatio;
}

float UFGearAxle::getMaxRpm() const
{
	float transRatio = mTransmission->getTransmissionRatio(this);
	if (mTorqueShare == 0.0f || transRatio == 0.0f) return 0.0f;
	return mEngine->getLimitRpm() / transRatio;
}

float UFGearAxle::getFeedbackTorque() const
{
	float transRatio = mTransmission->getTransmissionRatio(this);
	if (mTorqueShare == 0.0f || transRatio == 0.0f) return 0.0f;
	return (mWheelLeft->getFeedbackTorque() + mWheelRight->getFeedbackTorque()) / transRatio;
}

float UFGearAxle::getWheelRPMToKMH() const
{
	if (mFinalGearRatio == 0.0f) return 0.0f;
	return 0.5f * (mWheelLeft->getWheelRPMToKMH() + mWheelRight->getWheelRPMToKMH()) / mFinalGearRatio;
}

float UFGearAxle::getNormalizedSteering() const
{
	if (mCurrentSteerLeft + mCurrentSteerRight > 0.0f) return mCurrentSteerRight / mMaxSteerAngle;
	else return mCurrentSteerLeft / mMaxSteerAngle;
}

FVector UFGearAxle::getLocalCenter() const
{
	return 0.5f * (mWheelLeft->getLocalHubPosition() + mWheelRight->getLocalHubPosition());
}

FVector UFGearAxle::getGlobalCenter() const
{
	return 0.5f * (mWheelLeft->getHubPosition() + mWheelRight->getHubPosition());
}

float UFGearAxle::getKMHSpeed() const
{
	if (!mWheelLeft->isEnabled()) return mWheelRight->getKMHSpeed();
	if (!mWheelRight->isEnabled()) return mWheelLeft->getKMHSpeed();
	return 0.5f * (mWheelLeft->getKMHSpeed() + mWheelRight->getKMHSpeed());
}

float UFGearAxle::getRPM() const
{
	if (!mWheelLeft->isEnabled()) return mWheelRight->getRpm();
	if (!mWheelRight->isEnabled()) return mWheelLeft->getRpm();
	return 0.5f * (mWheelLeft->getRpm() + mWheelRight->getRpm());
}

float UFGearAxle::getGroundRPM() const
{
	if (!mWheelLeft->isEnabled()) return mWheelRight->getGroundRpm();
	if (!mWheelRight->isEnabled()) return mWheelLeft->getGroundRpm();
	return 0.5f * (mWheelLeft->getGroundRpm() + mWheelRight->getGroundRpm());
}

float UFGearAxle::getTrackWidth() const
{
	return FMath::Abs(mWheelLeft->getLocalHubPosition().Y - mWheelRight->getLocalHubPosition().Y);
}

//used for wheel friction calculations
float UFGearAxle::getDrivetrainFriction() const
{
	if (mAxleTorque == 0.0f) return 0.0f;
	float absAxleSpeed = FMath::Abs(getCurRpm()) * FGearUtility::rpm2Rads;
	return absAxleSpeed * mVehicle->getDrivetrainFriction() * mTransmission->getClutchState();
}

bool UFGearAxle::hasSteering() const
{
	return mMaxSteerAngle != 0.0f;
}

void UFGearAxle::setTargetSteer(float s)
{
	if (hasSteering()) mTargetSteer = s * mMaxSteerAngle;
}

void UFGearAxle::saveState(FFGearVehicleState& state)
{
	mWheelLeft->saveState(state);
	mWheelRight->saveState(state);
}

void UFGearAxle::loadState(FFGearVehicleState& state)
{
	mWheelLeft->loadState(state);
	mWheelRight->loadState(state);
}

#if WITH_EDITOR
bool UFGearAxle::CanEditChange(const FProperty* Property) const
{
	const bool ParentVal = Super::CanEditChange(Property);
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearAxle, mDiffStrength)) return mDifferentialType == FGearDifferentialType::DF_LSD;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearAxle, mFinalGearRatio)) return mTorqueShare > 0.0f;
	return ParentVal;
}
#endif

FVector2D UFGearAxle::drawTelemetry(UCanvas * Canvas, UFont * Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p)
{
	p = Super::drawTelemetry(Canvas, Font, textColor, renderInfo, p);
	Canvas->SetLinearDrawColor(textColor);
	Canvas->DrawText(Font, "Axle" + FString::FromInt(mIndex), p.X, p.Y, 1.0f, 1.0f, renderInfo);
	FGearUtility::drawLine(Canvas, p.X, p.Y + 18, p.X + 32, p.Y + 18, textColor);
	Canvas->DrawText(Font, FString::Printf(TEXT("TireModel : %s"), *mWheelOptions->getTireModel()->GetClass()->GetName()), p.X, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Differential : %s"), *FGearUtility::EnumToString(stringify(FGearDifferentialType), mDifferentialType)), p.X, p.Y + 35, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "Has Power: " + UKismetStringLibrary::Conv_BoolToString(mTorqueShare != 0.0f), p.X, p.Y + 50, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "Has Steering: " + UKismetStringLibrary::Conv_BoolToString(hasSteering()), p.X, p.Y + 65, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Torque Share: %%%.0f"), 100.0f * mTorqueShare), p.X, p.Y + 80, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Camber Angle: %.2f"), mCamberAngle), p.X, p.Y + 95, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Toe Angle: %.2f"), mToeAngle), p.X, p.Y + 110, 1.0f, 1.0f, renderInfo);
	p.Y += getWindowHeight();
	return p;
}