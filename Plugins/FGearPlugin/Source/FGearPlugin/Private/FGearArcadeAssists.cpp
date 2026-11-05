// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearArcadeAssists.h"
#include "FGearVehicle.h"
#include "FGearAxle.h"
#include "FGearWheel.h"
#include "FGearWheelOptions.h"
#include "FGearEngine.h"
#include "FGearTransmission.h"
#include "FGearSkeletalMeshComponent.h"
#include "FGearStandardInput.h"

UFGearArcadeAssists::UFGearArcadeAssists() :
	UFGearTelemetryDrawer(),

	mRecoverAssists(false),
	mUnderSteerRecoverPower(0.0f),
	mOverSteerRecoverPower(0.0f),
	mMinRecoverAssistSpeed(5.0f),
	mFullRecoverAssistSpeed(50.0f),

	mAntirollBars(false),
	mFrontAntiRollPower(0.0f),
	mRearAntiRollPower(0.0f),

	mTractionAssist(false),
	mTractionAssistRatio(0.0f),
	mTractionAssistMaxSpeed(15.0f),

	mDriftAssist(false),
	mDriftAssistFrontRatio(0.0f),
	mDriftAssistRearRatio(0.0f),

	mTorqueSplitter(false),
	mTorqueSplitterRatio(0.0f),
	mTorqueSplitterResponseTime(100.0f),

	mBikeHelper(false),
	mAntiRollSpring(125000000.0f),
	mDamperSmooth(1.0f),
	mMinLeanSpeed(20.0f),
	mMaxLeanAngle(45.0f),
	mMaxPitchAngle(75.0f),
	mLastPitch(0.0f),
	mLeanInAir(true)
{
}

void UFGearArcadeAssists::init(AFGearVehicle* v)
{
	mVehicle = v;
	mBody = mVehicle->getBody();

	int axles = mVehicle->getAxleCount();
	mAxles.Init(nullptr, axles);
	for (int i = 0; i < axles; i++)
	{
		mAxles[i] = mVehicle->getAxle(i);
	}

	//grab a copy of wheel friction factors -> axleCt * 2(lng/lat)
	mFrictionCoeffs.Init(0.0f, axles * 2);
	for (int i = 0; i < axles; i++)
	{
		mFrictionCoeffs[i * 2] = mVehicle->getAxle(i)->getWheelOptions()->getLongitudinalFriction();
		mFrictionCoeffs[i * 2 + 1] = mVehicle->getAxle(i)->getWheelOptions()->getLateralFriction();
	}

	//grab a copy of axle torque shares -> axleCt
	mTorqueShares.Init(0.0f, axles);
	for (int i = 0; i < axles; i++)
	{
		mTorqueShares[i] = mVehicle->getAxle(i)->getTorqueShare();
	}
}

void UFGearArcadeAssists::substepTick(float dt)
{
	if (mRecoverAssists) updateRecoverAssists();
	if (mAntirollBars) updateAntiRollBars();
	if (mTractionAssist) updateTractionAssist();
	if (mDriftAssist) updateDriftAssist();
	if (mTorqueSplitter) updateTorqueSplitter(dt);
	if (mBikeHelper) updateBikeHelper(dt);
}

void UFGearArcadeAssists::setInstantSpeed(float kmhSpeed, int gear)
{
	//set rigidbody velocity
	float speed = kmhSpeed * FGearUtility::kmh2ms * FGearUtility::meter2cm;
	mBody->SetLinearVelocity(speed * mVehicle->getForwardDir(), false);

	//set current gear
	if (gear >= -1 && gear <= mVehicle->getTransmission()->getMaxGear())
	{
		mVehicle->getTransmission()->setCurrentGear(gear, true);
	}

	//sync engine rpm, find fastest wheel rpm
	float wheelRpm = kmhSpeed / mVehicle->getMaxWheelRPMToKMH();
	float targetRpm = wheelRpm * mVehicle->getTransmission()->getTransmissionRatio();
	mVehicle->getEngine()->setRpm(targetRpm);
}

void UFGearArcadeAssists::setCruiseControl(bool active, float kmhSpeed)
{
	mCruiseControl = active;
	mCruiseSpeed = kmhSpeed;
}

void UFGearArcadeAssists::updateRecoverAssists()
{
	if (!mAxles.IsValidIndex(0)) return;

	//check minumum speed
	FVector dir2 = FGearUtility::getWorldTransform(mBody).TransformVectorNoScale(FVector::ForwardVector);
	float dot1 = FVector::DotProduct(dir2, mVehicle->getMyVelocity());
	if (dot1 < mMinRecoverAssistSpeed * FGearUtility::kmh2ms * FGearUtility::meter2cm) return;

	//find speed coeff
	bool frontContact = mAxles[0]->getLeftWheel()->hasContact() && mAxles[0]->getRightWheel()->hasContact();
	bool rearContact = mAxles[1]->getLeftWheel()->hasContact() && mAxles[1]->getRightWheel()->hasContact();
	float spdCoeff = 1.0f;
	if (mFullRecoverAssistSpeed > 0.0f)
	{
		float limit = mFullRecoverAssistSpeed - mMinRecoverAssistSpeed;
		if (limit > 0.0f) spdCoeff = FMath::Min(1.0f, (mVehicle->getKMHSpeed() - mMinRecoverAssistSpeed) / limit);
	}
	bool substep = !mVehicle->isSubstepping();

	if (frontContact)
	{
		FVector dir1 = 0.5f * (mAxles[0]->getLeftWheel()->getGlobalForward() + mAxles[0]->getRightWheel()->getGlobalForward());
		float angle = FGearUtility::SignedAngle(dir1, dir2, FVector::UpVector);
		float targetAngle = 0.5f * angle;
		float delta = targetAngle - angle;
		mVehicle->addTorque(FVector(0.0f, 0.0f, 100000.0f * spdCoeff * mUnderSteerRecoverPower * delta));
	}

	if (rearContact)
	{
		float angleRatio = FMath::Min(1.0f, FMath::Abs(mVehicle->getSteerDeltaAngle(false) / 90.0f));
		mVehicle->addTorque(FVector(0.0f, 0.0f, -200000.0f * spdCoeff * mOverSteerRecoverPower * FGearUtility::getWorldAngularVelocity(mBody).Z * angleRatio));
	}
}

void UFGearArcadeAssists::updateAntiRollBars()
{
	UFGearWheel* mFlw = mAxles[0]->getLeftWheel();
	UFGearWheel* mFrw = mAxles[0]->getRightWheel();
	UFGearWheel* mRlw = mAxles[1]->getLeftWheel();
	UFGearWheel* mRrw = mAxles[1]->getRightWheel();

	float flCompress = mFlw->getSuspensionCompressRatio();
	float frCompress = mFrw->getSuspensionCompressRatio();
	float rlCompress = mRlw->getSuspensionCompressRatio();
	float rrCompress = mRrw->getSuspensionCompressRatio();

	float frontAntiRoll = FMath::Abs(flCompress - frCompress) * 0.01f * mFrontAntiRollPower;
	float rearAntiRoll = FMath::Abs(rlCompress - rrCompress) * 0.01f * mRearAntiRollPower;

	//apply front
	if (flCompress > frCompress)
	{
		float springLength = FGearUtility::meter2cm * 1000.0f * mFlw->getSuspensionTotalLength();
		mFrw->applyVerticalForce(-rearAntiRoll * springLength * mFlw->getSuspensionSpring());
	}
	else if (frCompress > flCompress)
	{
		float springLength = FGearUtility::meter2cm * 1000.0f * mFrw->getSuspensionTotalLength();
		mFlw->applyVerticalForce(-rearAntiRoll * springLength * mFrw->getSuspensionSpring());
	}

	//apply rear
	if (rlCompress > rrCompress)
	{
		float springLength = FGearUtility::meter2cm * 1000.0f * mRlw->getSuspensionTotalLength();
		mRrw->applyVerticalForce(-frontAntiRoll * springLength * mRlw->getSuspensionSpring());
	}
	else if (rrCompress > rlCompress)
	{
		float springLength = FGearUtility::meter2cm * 1000.0f * mRrw->getSuspensionTotalLength();
		mRlw->applyVerticalForce(-frontAntiRoll * springLength * mRrw->getSuspensionSpring());
	}
}

void UFGearArcadeAssists::updateTractionAssist()
{
	//only works when drift assist is off
	if (mDriftAssistFrontRatio != 0.0f || mDriftAssistRearRatio != 0.0f) return;

	//reset friction factors
	for (int i = 0; i < mAxles.Num(); i++)
	{
		mAxles[i]->getWheelOptions()->setLongitudinalFriction(mFrictionCoeffs[i * 2]);
		mAxles[i]->getWheelOptions()->setLateralFriction(mFrictionCoeffs[i * 2 + 1]);
	}

	if (mTractionAssistRatio != 0.0f && FMath::Abs(mVehicle->getKMHSpeed()) < mTractionAssistMaxSpeed)
	{
		float magnitude = (mTractionAssistMaxSpeed - FMath::Abs(mVehicle->getKMHSpeed())) / mTractionAssistMaxSpeed;
		magnitude *= 0.01f * mTractionAssistRatio;
		for (int i = 0; i < mAxles.Num(); i++)
		{
			mAxles[i]->getWheelOptions()->setLongitudinalFriction((1.0f + magnitude) * mFrictionCoeffs[i * 2]);
			mAxles[i]->getWheelOptions()->setLateralFriction((1.0f + magnitude) * mFrictionCoeffs[i * 2 + 1]);
		}
	}

	//apply
	for (int i = 0; i < mAxles.Num(); i++)
	{
		mAxles[i]->applyWheelOptions();
	}
}

void UFGearArcadeAssists::updateDriftAssist()
{
	//only works when traction assist is off
	if (mTractionAssistRatio != 0.0f) return;

	//reset assist factors
	for (int i = 0; i < mAxles.Num(); i++)
	{
		mAxles[i]->getWheelOptions()->setLateralFriction(mFrictionCoeffs[i * 2 + 1]);
	}

	if (mVehicle->getForwardSpeed() < 1.0f) return;

	if (mDriftAssistFrontRatio != 0.0f || mDriftAssistRearRatio != 0.0f)
	{
		float angleRatio = FMath::Min(1.0f, FMath::Abs(mVehicle->getSteerDeltaAngle(false) / 90.0f));
		mAxles[0]->getWheelOptions()->setLateralFriction((1.0f + 0.01f * mDriftAssistFrontRatio * angleRatio) * mFrictionCoeffs[1]);
		mAxles[1]->getWheelOptions()->setLateralFriction((1.0f + 0.01f * mDriftAssistRearRatio * angleRatio) * mFrictionCoeffs[3]);
	}

	//apply
	for (int i = 0; i < mAxles.Num(); i++)
	{
		mAxles[i]->applyWheelOptions();
	}
}

void UFGearArcadeAssists::updateTorqueSplitter(float dt)
{
	if (mTorqueSplitterRatio == 0.0f) return;

	float frontSpeed = FMath::Abs(mVehicle->getAxle(0)->getRPM());
	float rearSpeed = FMath::Abs(mVehicle->getAxle(1)->getRPM());
	float splitRatio = mTorqueSplitterRatio / 100.0f;
	float frontShare = mTorqueShares[0];
	float rearShare = mTorqueShares[1];

	if (frontSpeed > rearSpeed)
	{
		float frontRatio = rearSpeed / frontSpeed;
		float rearRatio = 1.0f - frontRatio;
		frontShare = mTorqueShares[0] - splitRatio * rearRatio * mTorqueShares[0];
		rearShare = 1.0f - frontShare;
	}
	else if (rearSpeed > frontSpeed)
	{
		float rearRatio = frontSpeed / rearSpeed;
		float frontRatio = 1.0f - rearRatio;
		rearShare = mTorqueShares[1] - splitRatio * frontRatio * mTorqueShares[1];
		frontShare = 1.0f - rearShare;
	}

	float curFrontShare = mVehicle->getAxle(0)->getTorqueShare();
	float curRearShare = mVehicle->getAxle(1)->getTorqueShare();
	float time = FMath::Clamp(dt / (0.001f * mTorqueSplitterResponseTime), 0.0f, 1.0f);
	mVehicle->getAxle(0)->setTorqueShare(FMath::Lerp(curFrontShare, frontShare, time));
	mVehicle->getAxle(1)->setTorqueShare(FMath::Lerp(curRearShare, rearShare, time));
}

void UFGearArcadeAssists::updateBikeHelper(float dt)
{
	float antiRollTorque = 0.0f;
	float rollDamperTorque = 0.0f;
	float pitchDamperTorque = 0.0f;

	bool frontContact = mAxles[0]->getLeftWheel()->hasContact() && mAxles[0]->getRightWheel()->hasContact();
	bool rearContact = mAxles[1]->getLeftWheel()->hasContact() && mAxles[1]->getRightWheel()->hasContact();
	bool wheelContact = frontContact && rearContact;

	//anti roll torque
	FTransform trans = FGearUtility::getWorldTransform(mBody);
	FRotator rotation = trans.GetRotation().Rotator();
	float roll = rotation.Roll;
	float pitch = rotation.Pitch;
	if (wheelContact || mLeanInAir)
	{
		float speedCoeff = FMath::Clamp(mVehicle->getKMHSpeed() / mMinLeanSpeed, 0.0f, 1.0f);
		float rollOffset = mVehicle->getNormalizedSteering() * speedCoeff * mMaxLeanAngle;
		roll -= rollOffset;
	}
	
	//fix pitch/roll limits
	if (roll > 180.0f) roll = -360.0f + roll;
	if (pitch > 180.0f) pitch = -360.0f + pitch;
	if (FMath::Abs(roll) >= 90.0f && (frontContact || rearContact)) roll += 180.0f;
	if (roll > 180.0f) roll = -360.0f + roll;
	antiRollTorque = roll / 90.0f * mAntiRollSpring;

	//anti roll damper
	FVector avelocity = FGearUtility::getWorldAngularVelocity(mBody);
	float xVelocity = trans.InverseTransformVector(avelocity).X;
	float ra = xVelocity / dt;
	float rollCoeff = FMath::Clamp(FMath::Abs(roll / mDamperSmooth), 0.0f, 1.0f);
	//we need inertiaX but UE5 gives incorrect results so we take the smallest
	float inertiaX = mVehicle->getLocalInertiaTensor().GetAbsMin();
	rollDamperTorque = inertiaX * ra * rollCoeff;
	
	if (!wheelContact)
	{
		//pitch damper
		mLastPitch = pitchDamperTorque;
		if ((pitch >= 0.0f && pitch > mMaxPitchAngle && pitch > mLastPitch) ||
			(pitch < 0.0f && pitch < -mMaxPitchAngle && pitch < mLastPitch))
		{
			float yVelocity = trans.InverseTransformVector(avelocity).Y;
			float pa = yVelocity / dt;
			float pitchCoeff = FMath::Clamp(FMath::Abs(pitch / mDamperSmooth), 0.0f, 1.0f);
			//we need inertiaY but UE5 gives incorrect results so we take the largest
			float inertiaY = mVehicle->getLocalInertiaTensor().GetAbsMax();
			pitchDamperTorque = inertiaY * pa * pitchCoeff;
		}
	}

	//apply torques
	float xTorque = antiRollTorque - rollDamperTorque;
	float yTorque = -pitchDamperTorque;
	FVector torque(xTorque, yTorque, 0.0f);
	torque = trans.TransformVector(torque);
	mVehicle->addTorque(torque);
}

void UFGearArcadeAssists::updateCruiseControl(float dt)
{
	//cancel cases
	if (!mCruiseControl || mVehicle->getIsSleeping() || mVehicle->getTransmission()->getCurGear() <= 0.0f)
	{
		return;
	}

	//cruise control input
	float error = mCruiseSpeed - mVehicle->getKMHSpeed();
	float output = FMath::Clamp(mCruiseControlPID.getOutput(error, dt), -1.0f, 1.0f);
	float throttle = output > 0.0f ? output : 0.0f;
	float brake = output < 0.0f ? -output : 0.0f;

	//user input
	float iThrottle = mVehicle->getStandardInput()->getThrottleFinalInput();
	float iBrake = mVehicle->getStandardInput()->getBrakeFinalInput();
	float steer = mVehicle->getStandardInput()->getSteerFinalInput();
	float clutch = mVehicle->getStandardInput()->getClutchFinalInput();
	int gear = mVehicle->getStandardInput()->getGearboxInput();
	bool hb = mVehicle->getStandardInput()->getHandbrakeInput();

	//user input overrides cruise control
	if (iThrottle != 0.0f || iBrake != 0.0f)
	{
		bool invertVertical = mVehicle->getTransmission()->isAutoChange() &&
							  mVehicle->getTransmission()->isAutoReverse() &&
							  mVehicle->getTransmission()->getCurGear() < 0;
		throttle = invertVertical ? iBrake : iThrottle;
		brake = invertVertical ? iThrottle : iBrake;
	}

	//override inputs
	mVehicle->getStandardInput()->overrideFinalInputs(throttle, brake, steer, clutch, gear, hb);
}