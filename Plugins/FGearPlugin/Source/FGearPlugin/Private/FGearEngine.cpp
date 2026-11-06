// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearEngine.h"
#include "FGearVehicle.h"
#include "FGearTransmission.h"

UFGearEngine::UFGearEngine() :
	UFGearTelemetryDrawer(),
	mTorqueCurve(nullptr),
	mTorqueScale(1.0f),
	mFrictionTorque(50.0f),
	mIdleRpm(1000.0f),
	mLimitRpm(6500.0f),
	mLimiterTime(100.0f),
	mEngineInertia(0.25f),
	mEngineStallRpm(600.0f),
	mEngineStallTime(-1.0f),
	mRunning(true),
	mRpm(0.0f),
	mThrottle(0.0f),
	mLimiter(0.0f),
	mTorque(0.0f),
	mFeedbackRpm(0.0f),
	mFeedbackTorque(0.0f),
	mPeakTorqueRpm(0.0f),
	mMatchRpm(0.0f),
	mMatchTime(0.0f),
	mMatchDelta(0.0f),
	mStallTime(0.0f)
{
}

void UFGearEngine::init(AFGearVehicle* v)
{
	if (mTorqueCurve == nullptr)
	{
		mTorqueCurve = NewObject<UCurveFloat>();
		mTorqueCurve->FloatCurve.AddKey(0.0f, 0.0f);
		mTorqueCurve->FloatCurve.AddKey(1000.0f, 75.0f);
		mTorqueCurve->FloatCurve.AddKey(4000.0f, 150.0f);
		mTorqueCurve->FloatCurve.AddKey(6000.0f, 100.0f);
	}

	mVehicle = v;
	mTransmission = v->getTransmission();
	mRpm = mIdleRpm;
	mIdleRpmTorque = mTorqueCurve->GetFloatValue(mIdleRpm);
	calculatePeakTorqueRpm();
	setWindowHeight(125);
}

void UFGearEngine::myFixedUpdate(float dt)
{
	//check engine stall
	updateStallState(dt);

	//rev matching
	if (mMatchTime > 0.0f)
	{
		if (mMatchTime < dt) mRpm = mMatchRpm;
		else mRpm += dt * mMatchDelta;
		mMatchTime -= dt;
	}

	//pedal acceleration
	float angularAcc = 0.0f;
	bool hasThrottle = mThrottle > 0.0f;
	float frictionTorque = (mRunning ? getRpmRatio() : 1.0f) * mFrictionTorque;
	if (mRunning && mRpm < mIdleRpm) angularAcc = mTorqueCurve->GetFloatValue(mIdleRpm) / mEngineInertia;
	else if (!hasThrottle || mLimiter > 0.0f) angularAcc = -frictionTorque / mEngineInertia;
	else angularAcc = (mTorque - frictionTorque) / mEngineInertia;
	float angularVel = angularAcc * dt;
	float rpmVel1 = angularVel * FGearUtility::rads2Rpm;

	//update rpm
	if (mMatchTime <= 0.0f) mRpm += rpmVel1;

	//apply limits
	if (mRunning && mRpm < mIdleRpm && rpmVel1 < 0.0f) mRpm -= rpmVel1;
	if (mRpm > mLimitRpm) { mRpm = mLimitRpm; mLimiter = mLimiterTime / 1000.0f; }
	else if (mRpm < 0.0f) mRpm = 0.0f;
	if (mLimiter > 0.0f) mLimiter -= dt;

	//torque with pedal
	mTorque = hasThrottle ? mThrottle * mTorqueCurve->GetFloatValue(mRpm) : -frictionTorque;

	//no throttle state
	if (mTorque == -frictionTorque)
	{
		//stalling
		if (canStall() && mFeedbackRpm < mIdleRpm && mFeedbackTorque < mIdleRpmTorque)
		{
			mTorque = mIdleRpmTorque;
		}
		//scale friction torque
		//this prevents jittery behavior when wheels are close to 0 rpm
		else
		{
			bool transOn = mTransmission->getCurGear() != 0 && mTransmission->getClutchState() != 0.0f;
			if (transOn && mFeedbackRpm != 0.0f)
			{
				mTorque *= FMath::Clamp(mFeedbackRpm / mIdleRpm, -1.0f, 1.0f);
			}
		}
	}

	//finalize
	mTorque *= mTorqueScale;
}

void UFGearEngine::setFeedback(float dt, float rpm, float torque)
{
	//override FeedbackTorque acc. to clutch
	float clutchPower = mTransmission->getClutchState();
	mFeedbackTorque = torque * clutchPower;

	//feedback drive
	float clampedRpm = FMath::Clamp(rpm, mRunning ? mIdleRpm : 0.0f, mLimitRpm);
	float sign = FMath::Sign(clampedRpm - mRpm);
	float totalTorque = sign * FMath::Abs(mFeedbackTorque);
	float angularAcc = totalTorque / mEngineInertia;
	float angularVel = angularAcc * dt;
	float rpmVel2 = angularVel * FGearUtility::rads2Rpm;
	if (mLimiter > 0 || mMatchTime > 0.0f) rpmVel2 = 0.0f;

	//alter feedback drive to avoid overshoots and take clutch into account
	float deltaRpm = FMath::Abs(clampedRpm - mRpm);
	if (FMath::Abs(rpmVel2) > deltaRpm)
	{
		rpmVel2 = FGearUtility::Sign(rpmVel2) * deltaRpm * clutchPower;
	}

	//update rpm
	mRpm += rpmVel2;
	mFeedbackRpm = rpm;
}

void UFGearEngine::setRevMatching(float time, float rpm)
{
	if (time > 0.0f)
	{
		mMatchTime = time;
		mMatchRpm = FMath::Clamp(rpm, mIdleRpm, mLimitRpm);
		mMatchDelta = (mMatchRpm - mRpm) / time;
	}
	else mRpm = rpm;
}

bool UFGearEngine::canStall()
{
	return mRunning && mEngineStallTime >= 0.0f && !mVehicle->getTransmission()->isAutoClutch();
}

void UFGearEngine::updateStallState(float dt)
{
	if (canStall())
	{
		if (mFeedbackRpm < mEngineStallRpm && mFeedbackTorque > mIdleRpmTorque)
		{
			mStallTime += dt;
			if (mStallTime > mEngineStallTime)
			{
				setRunning(false);
			}
			return;
		}
	}

	if (mStallTime > 0.0f)
	{
		mStallTime -= dt;
		if (mStallTime < 0.0f) mStallTime = 0.0f;
	}
}

void UFGearEngine::setThrottle(float pedal)
{
	mThrottle = mRunning ? pedal : 0.0f;
	//non zero throttle wakes up
	if (mThrottle != 0.0f && mVehicle->getIsSleeping() && mVehicle->canWakeByThrottle())
	{
		mVehicle->wakeUp();
	}
}

//set engine on/off
void UFGearEngine::setRunning(bool b)
{
	if (b != mRunning)
	{
		if (OnStartStop.IsBound()) OnStartStop.Broadcast(b);
		mRunning = b;
	}
	
	mThrottle = 0.0f;
	mMatchRpm = 0.0f;
	mMatchTime = 0.0f;
	mMatchDelta = 0.0f;
	mStallTime = 0.0f;
}

//used for clutch assist
void UFGearEngine::calculatePeakTorqueRpm()
{
	mPeakTorqueRpm = mIdleRpm;
	if (mTorqueCurve == nullptr) return;
	float maxTorque = 0.0f;
	for (float rpm = mIdleRpm; rpm <= mLimitRpm; rpm++)
	{
		float torque = mTorqueCurve->GetFloatValue(rpm);
		if (torque > maxTorque)
		{
			maxTorque = torque;
			mPeakTorqueRpm = rpm;
		}
	}
}

//return max power in kW
void UFGearEngine::calculateMaxPower(float& maxPower, float& maxPowerRpm)
{
	maxPower = maxPowerRpm = 0.0f;
	if (mTorqueCurve == nullptr) return;
	for (float rpm = mIdleRpm; rpm <= mLimitRpm; rpm++)
	{
		float torque = mTorqueScale * mTorqueCurve->GetFloatValue(rpm);
		float power = torque * rpm / 9548.8f;
		if (power > maxPower)
		{
			maxPower = power;
			maxPowerRpm = rpm;
		}
	}
}

void UFGearEngine::saveState(FFGearVehicleState& state)
{
	state.mRpm = mRpm;
	state.mLimiter = mLimiter;
	state.mFeedbackRpm = mFeedbackRpm;
}

void UFGearEngine::loadState(FFGearVehicleState& state)
{
	mRpm = state.mRpm;
	mLimiter = state.mLimiter;
	mFeedbackRpm = state.mFeedbackRpm;
}

FVector2D UFGearEngine::drawTelemetry(UCanvas * Canvas, UFont * Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p)
{
	float maxPower, maxPowerRpm;
	calculateMaxPower(maxPower, maxPowerRpm);
	maxPower *= FGearUtility::kw2hp;
	float curPower = (mTorque * mRpm / 9548.8f) * FGearUtility::kw2hp;

	p = Super::drawTelemetry(Canvas, Font, textColor, renderInfo, p);
	Canvas->SetLinearDrawColor(textColor);
	Canvas->DrawText(Font, "Engine", p.X, p.Y, 1.0f, 1.0f, renderInfo);
	FGearUtility::drawLine(Canvas, p.X, p.Y + 18, p.X + 40, p.Y + 18, textColor);
	Canvas->DrawText(Font, FString::Printf(TEXT("Rpm: %.0f"), mRpm), p.X, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("LimitRpm: %.0f"), mLimitRpm), p.X, p.Y + 35, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "Limiter: " + UKismetStringLibrary::Conv_BoolToString(isLimiterOn()), p.X, p.Y + 50, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Torque: %.0f Nm"), mTorque), p.X, p.Y + 65, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Power: %.0fhp"), curPower), p.X, p.Y + 80, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Max Power: %.0fhp @%.0f"), maxPower, maxPowerRpm), p.X, p.Y + 95, 1.0f, 1.0f, renderInfo);
	p.Y += getWindowHeight();
	return p;
}