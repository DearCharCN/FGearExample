// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearTransmission.h"
#include "FGearVehicle.h"
#include "FGearEngine.h"
#include "FGearAxle.h"
#include <limits>

#if WITH_EDITOR
#include "FGearCustomPanel.h"
#endif

UFGearTransmission::UFGearTransmission() :
	UFGearTelemetryDrawer(),
	mAutoChange(true),
	mAutoReverse(true),
	mAutoClutch(true),
	mAutoClutchActive(true),
	mRevMatching(true),
	mChangeTime(100.0f),
	mGearUpRatio(90.0f),
	mGearDownRatio(80.0f),
	mGearUpVelocityBias(1.0f),
	mGearDownVelocityBias(1.0f),
	mClutchTime(100.0f),
	mClutchScale(100.0f),
#if WITH_EDITOR
	mLayout(nullptr),
#endif
	mVehicle(nullptr),

	mCurGear(0),
	mNextGear(0),
	mMaxGear(0),
	mThrottleState(0),
	mPauseTime(0.0f),
	mUpSpeed(0.0f),
	mDownSpeed(0.0f),
	mClutchState(1.0f)
{
	mGearRatios.Init(0, 2);
	mGearRatios[0] = 5.0f;
	mGearRatios[1] = 3.0f;
}

void UFGearTransmission::init(AFGearVehicle * v)
{
	//check nulls
	if (mGearRatios.Num() == 0)
	{
		mGearRatios.Init(0, 2);
		mGearRatios[0] = 5.0f;
		mGearRatios[1] = 3.0f;
	}

	//start at 1st gear if auto
	if (mAutoChange)
	{
		mCurGear = mNextGear = 1;
	}

	mVehicle = v;
	refreshParameters();
	setWindowHeight(140);
}

//called by ui to get real time parameter update for gear ratios
void UFGearTransmission::refreshParameters()
{
	if (mGearRatios.Num() == 0) { mMaxSpeeds.SetNum(0); return; }

	if (mVehicle == nullptr) mVehicle = Cast<AFGearVehicle>(GetOwner());

	mMaxGear = mGearRatios.Num() - 1; //-1 rear
	mMaxSpeeds.SetNum(mMaxGear + 1); //+1 rear

	//calculate max speed for each gear
	UFGearEngine* e = mVehicle->getEngine();
	float limitRpm = e != nullptr ? e->getLimitRpm() : 5000.0f;
	for (int i = 0; i < mMaxSpeeds.Num(); i++)
	{
		float gearRatio = (i == mMaxSpeeds.Num() - 1) ? getGearRatio(-1) : getGearRatio(i + 1);
		float wheelRpm = limitRpm / gearRatio;
		mMaxSpeeds[i] = wheelRpm * mVehicle->getMaxWheelRPMToKMH();
	}
}

void UFGearTransmission::myFixedUpdate(float dt)
{
	if (mPauseTime > 0.0f)
	{
		mPauseTime -= dt;
		if (mPauseTime < 0.0f) mPauseTime = 0.0f;
		return;
	}
	else if (mCurGear != mNextGear)
	{
		gearChanged(mCurGear, mNextGear);
		mCurGear = mNextGear;
	}

	//manage clutch
	updateClutchState(dt);

	//calc. up/down speeds
	mUpSpeed = std::numeric_limits<float>::max();
	mDownSpeed = 0.0f;
	if (mCurGear > 0 && mCurGear < mMaxGear)
	{
		mUpSpeed = mMaxSpeeds[mCurGear - 1] * (0.01f * mGearUpRatio);
	}
	if (mCurGear > 1)
	{
		mDownSpeed = mMaxSpeeds[mCurGear - 2] * (0.01f * mGearDownRatio);
	}
	if (mCurGear < 0)
	{
		mUpSpeed = 0.0f;
		mDownSpeed = std::numeric_limits<float>::min();
	}

	//skip the rest if no auto gearing
	if (!mAutoChange) return;

	//do not stay N if auto
	if (mCurGear == 0 && mPauseTime == 0.0f)
	{
		gearUp(true);
		return;
	}

	//speeds
	float wspeed = mVehicle->getMinWheelKMHSpeed(true);
	float velocity = mVehicle->getKMHSpeed();
	float speedUp = FMath::Lerp(wspeed, velocity, mGearUpVelocityBias);
	float speedDown = FMath::Lerp(wspeed, velocity, mGearDownVelocityBias);

	//R -> 1
	if (mAutoReverse && mCurGear < 0 && wspeed > -1.0f && speedUp > -1.0f && mThrottleState > 0)
	{
		gearUp(true);
	}
	//1 -> R
	else if (mAutoReverse && mCurGear == 1 && wspeed < 1.0f && speedDown < 1.0f && mThrottleState < 0)
	{
		gearDown(true);
	}
	//X -> Y
	else if (speedUp > mUpSpeed && mCurGear > 0)
	{
		gearUp(true);
	}
	else if (speedDown < mDownSpeed && mCurGear > 1)
	{
		gearDown(true);
	}
}

void UFGearTransmission::updateClutchState(float dt)
{
	//prepare params
	float engageDelta = dt / (0.001f * mClutchTime);
	float curRpm = mVehicle->getEngine()->getRpm();
	float throttle = mVehicle->getEngine()->getThrottle();

	//assist when there is throttle only
	if (mClutchAssist && curRpm > 0.0f && throttle > 0.0f && (mCurGear == 1 || mCurGear == -1))
	{
		//find an assist ratio based on current rpm
		float idleRpm = mVehicle->getEngine()->getIdleRpm();
		float targetRpm = mVehicle->getEngine()->getPeakTorqueRpm();
		float deltaRpm = targetRpm - idleRpm;
		if (deltaRpm > 0.0f)
		{
			curRpm = FMath::Clamp(curRpm, idleRpm, targetRpm);
			float ratio = FMath::Max(0.0f, curRpm - idleRpm);
			ratio = (ratio * ratio) / (deltaRpm * deltaRpm); //exponential
			mClutchScale = (mClutchScaleMin + ratio * (mClutchScaleMax - mClutchScaleMin));
		}
		else mClutchScale = mClutchScaleMax;

		//keep engaging state
		if (mClutchState < 1.0f) mClutchState += engageDelta;
	}
	//re-engage
	else if (mAutoClutchActive)
	{
		if (mClutchState < 1.0f) mClutchState += engageDelta;
		if (mClutchAssist && mClutchScale < 100.0f) mClutchScale += 100.0f * engageDelta;
	}

	//clamp
	mClutchState = FMath::Clamp(mClutchState, 0.0f, 1.0f);
	if (mClutchAssist) mClutchScale = FMath::Clamp(mClutchScale, 0.0f, 100.0f);
}

void UFGearTransmission::setCurrentGear(int gear, bool force)
{
	if (force)
	{
		gearChanged(mCurGear, gear);
		mCurGear = gear;
		mNextGear = gear;
	}
	else if (gear != mCurGear && (mCurGear != -1 || gear != 7) && (mAutoClutchActive || mClutchState < 1.0f))
	{
		mNextGear = gear;
		//R
		if (mNextGear == 7) mNextGear = -1;
		//check overflow
		if (mNextGear > mMaxGear) mNextGear = 0;
		if (mAutoClutchActive) mClutchState = 0.0f;
		//rev-match for downshifts
		if (mRevMatching && mNextGear != mCurGear && mNextGear > 0)
		{
			float wheelRpm = mVehicle->getMaxWheelKMHSpeed(true, true) / mVehicle->getMaxWheelRPMToKMH();
			float targetRpm = wheelRpm * getGearRatio(mNextGear);
			if (targetRpm > mVehicle->getEngine()->getRpm())
			{
				mVehicle->getEngine()->setRevMatching(0.001f * mChangeTime, targetRpm);
			}
		}
	}
}

void UFGearTransmission::gearUp(bool skipN)
{
	if (!mAutoClutchActive && mClutchState >= 1.0f) return;
	if (skipN && mCurGear == -1) mNextGear = 0;
	mNextGear++;
	if (mNextGear > mMaxGear)
	{
		mNextGear = mMaxGear;
		return;
	}
	gearChanged(mCurGear, 0);
	mCurGear = 0;
	mClutchState = 0.0f;
	mPauseTime = mChangeTime / 1000.0f; //to secs.
	if (mNextGear > 1)
	{
		//rev-match to next gear
		if (mRevMatching)
		{
			float wheelRpm = mVehicle->getMaxWheelKMHSpeed(true, true) / mVehicle->getMaxWheelRPMToKMH();
			float targetRpm = wheelRpm * getGearRatio(mNextGear);
			mVehicle->getEngine()->setRevMatching(mPauseTime, targetRpm);
		}
		else mVehicle->getEngine()->setLimiter(mPauseTime);
	}
}

void UFGearTransmission::gearDown(bool skipN)
{
	if (!mAutoClutchActive && mClutchState >= 1.0f) return;
	if (skipN && mCurGear == 1) mNextGear = 0;
	mNextGear--;
	if (mNextGear < -1)
	{
		mNextGear = -1;
		return;
	}
	gearChanged(mCurGear, 0);
	mCurGear = 0;
	mClutchState = 0.0f;
	mPauseTime = mChangeTime / 1000.0f; //to secs.
	if (mNextGear > 0)
	{
		//rev-match to next gear
		if (mRevMatching)
		{
			float wheelRpm = mVehicle->getMaxWheelKMHSpeed(true, true) / mVehicle->getMaxWheelRPMToKMH();
			float targetRpm = wheelRpm * getGearRatio(mNextGear);
			mVehicle->getEngine()->setRevMatching(mPauseTime, targetRpm);
		}
		else mVehicle->getEngine()->setLimiter(mPauseTime);
	}
}

float UFGearTransmission::getCurrentGearRatio() const
{
	return getGearRatio(mCurGear);
}

float UFGearTransmission::getTransmissionRatio(const UFGearAxle* axle) const
{
	if (axle == nullptr) return 0.0f;
	return getGearRatio(mCurGear) * axle->getFinalGearRatio();
}

float UFGearTransmission::getGearRatio(int gear) const
{
	//N
	if (gear == 0) return 0.0f;
	//R
	if (gear == -1) return -mGearRatios[mMaxGear];
	//D
	return mGearRatios[gear - 1];
}

void UFGearTransmission::setClutchState(float cs)
{
	if (mAutoClutch)
	{
		//if no user input we keep auto clutch active
		mAutoClutchActive = (cs == 1.0f);
		//if there is user input, override
		if (!mAutoClutchActive)	mClutchState = cs;
	}
	else
	{
		mAutoClutchActive = false;
		mClutchState = cs;
	}
}

void UFGearTransmission::setGearCount(int size)
{
#if ENGINE_MAJOR_VERSION == 5
	mGearRatios.SetNum(size, EAllowShrinking::Yes);
#else
	mGearRatios.SetNum(size, true);
#endif
}

void UFGearTransmission::saveState(FFGearVehicleState& state)
{
	state.mCurGear = mCurGear;
	state.mNextGear = mNextGear;
	state.mPauseTime = mPauseTime;
}

void UFGearTransmission::loadState(FFGearVehicleState& state)
{
	mCurGear = state.mCurGear;
	mNextGear = state.mNextGear;
	mPauseTime = state.mPauseTime;
}

void UFGearTransmission::gearChanged(int prev, int next)
{
	if (OnGearChanged.IsBound() && prev != next)
	{
		OnGearChanged.Broadcast(prev, next);
	}
	//wakes up vehicle
	if (prev != next && mVehicle->getIsSleeping() && mVehicle->canWakeByShifting())
	{
		mVehicle->wakeUp();
	}
}

FVector2D UFGearTransmission::drawTelemetry(UCanvas * Canvas, UFont * Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p)
{
	p = Super::drawTelemetry(Canvas, Font, textColor, renderInfo, p);
	Canvas->SetLinearDrawColor(textColor);
	Canvas->DrawText(Font, "Transmission", p.X, p.Y, 1.0f, 1.0f, renderInfo);
	FGearUtility::drawLine(Canvas, p.X, p.Y + 18, p.X + 80, p.Y + 18, textColor);
	Canvas->DrawText(Font, "Automatic: " + UKismetStringLibrary::Conv_BoolToString(mAutoChange), p.X, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Max Gear: %d"), mMaxGear), p.X, p.Y + 35, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Cur Gear: %s"), (mCurGear < 0 ? *FString("R") : (mCurGear == 0 ? *FString("N") : *FString::FromInt(mCurGear)))), p.X, p.Y + 50, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Change Time: %s ms"), (mPauseTime == 0.0f ? *FString::FromInt(mChangeTime) : *FString::FromInt(mChangeTime - 1000.0f * mPauseTime))), p.X, p.Y + 65, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Up Speed: %.0f kmh"), mUpSpeed), p.X, p.Y + 80, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Down Speed: %.0f kmh"), mDownSpeed), p.X, p.Y + 95, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Clutch State: %%%.0f"), (mClutchState * 100.0f)), p.X, p.Y + 110, 1.0f, 1.0f, renderInfo);
	
	for (int i = 0; i < mMaxSpeeds.Num(); i++)
	{
		FString gearTxt = (i == mMaxSpeeds.Num() - 1) ? "R" : FString::FromInt(i + 1);
		Canvas->DrawText(Font, FString::Printf(TEXT("Gear %s Top Speed %.0f kmh"), *gearTxt, mMaxSpeeds[i]), p.X, p.Y + 125 + i * 15, 1.0f, 1.0f, renderInfo);
	}
	
	setWindowHeight(140 + mMaxSpeeds.Num() * 15);
	p.Y += getWindowHeight();
	return p;
}

#if WITH_EDITOR
void UFGearTransmission::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	refreshParameters();
	if (mLayout != nullptr) mLayout->refresh();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

bool UFGearTransmission::CanEditChange(const FProperty* Property) const
{
	const bool ParentVal = Super::CanEditChange(Property);
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearTransmission, mAutoReverse))	return mAutoChange;
	return ParentVal;
}
#endif