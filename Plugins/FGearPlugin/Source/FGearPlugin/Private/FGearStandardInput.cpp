// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearStandardInput.h"
#include "FGearVehicle.h"
#include "FGearTransmission.h"
#include "FGearEngine.h"
#include "FGearAxle.h"
#include "FGearWheel.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"

UFGearStandardInput::UFGearStandardInput() :
	UFGearTelemetryDrawer(),
	mEnabled(true),
	mInputType(FGearInputSystemType::LEGACY),
	mCombinedAxis(true),
	mControllerType(FGearControllerType::KEYBOARD),
	mShifterType(FGearShifterType::AUTO),

	mSteerSensivity(5.0f),
	mSteerSpeedCurve(nullptr),
	mSteerLimitCurve(nullptr),
	mSteerInputGravity(1.5f),
	mSteerRange(100.0f),
	mSteerDeadzone(0.01f),
	mSteeringAssist(0.0f),
	mSteeringAssistThresholdAngle(1.0f),
	
	mThrottleSensivity(3.0f),
	mThrottleRange(100.0f),
	mThrottleDeadzone(0.01f),
	mThrottleEngagement(nullptr),

	mBrakingSensivity(3.0f),
	mBrakingRange(100.0f),
	mBrakingDeadzone(0.01f),
	mBrakeEngagement(nullptr),

	mClutchSensivity(3.0f),
	mClutchRange(100.0f),
	mClutchDeadzone(0.01f),
	mClutchEngagement(nullptr),

	//final inputs
	mThrottleInput(0.0f),
	mBrakeInput(0.0f),
	mSteerInput(0.0f),
	mClutchInput(0.0f),

	//raw inputs
	mForward(0.0f),
	mBackward(0.0f),
	mHorizontal(0.0f),
	mClutch(0.0f),

	//resulting inputs
	mThrottleState(0),
	mThrottleValue(0.0f),
	mBrakeValue(0.0f),
	mClutchValue(0.0f),
	mHandBrakeInput(false),
	mGearboxInput(0),
	mLastGearboxInput(0),
	mTargetSteeringInput(0.0f),
	mCurrentSteeringInput(0.0f),
	mCurrentSteeringLimit(0.0f),

	mInputMappingInitialized(false),
	mReadInputs(true),
	mSteerAssistActive(false),
	mStartGridMode(false)
{
}

void UFGearStandardInput::init(AFGearVehicle * v)
{
	mVehicle = v;

	//do not leave curves empty
	if (mSteerSpeedCurve == nullptr) mSteerSpeedCurve = NewObject<UCurveFloat>();
	if (mSteerSpeedCurve->FloatCurve.GetNumKeys() <= 1)
	{
		mSteerSpeedCurve->FloatCurve.AddKey(0.0f, 2.0f);
		mSteerSpeedCurve->FloatCurve.AddKey(1.0f, 2.0f);
	}

	//do not leave curves empty
	if (mSteerLimitCurve == nullptr) mSteerLimitCurve = NewObject<UCurveFloat>();
	if (mSteerLimitCurve->FloatCurve.GetNumKeys() <= 1)
	{
		mSteerLimitCurve->FloatCurve.AddKey(0.0f, 100.0f);
		mSteerLimitCurve->FloatCurve.AddKey(1.0f, 100.0f);
	}

	//do not leave curves empty
	if (mThrottleEngagement == nullptr) mThrottleEngagement = NewObject<UCurveFloat>();
	if (mThrottleEngagement->FloatCurve.GetNumKeys() <= 1)
	{
		mThrottleEngagement->FloatCurve.AddKey(0.0f, 0.0f);
		mThrottleEngagement->FloatCurve.AddKey(1.0f, 1.0f);
	}

	//do not leave curves empty
	if (mBrakeEngagement == nullptr) mBrakeEngagement = NewObject<UCurveFloat>();
	if (mBrakeEngagement->FloatCurve.GetNumKeys() <= 1)
	{
		mBrakeEngagement->FloatCurve.AddKey(0.0f, 0.0f);
		mBrakeEngagement->FloatCurve.AddKey(1.0f, 1.0f);
	}

	//do not leave curves empty
	if (mClutchEngagement == nullptr) mClutchEngagement = NewObject<UCurveFloat>();
	if (mClutchEngagement->FloatCurve.GetNumKeys() <= 1)
	{
		mClutchEngagement->FloatCurve.AddKey(0.0f, 0.0f);
		mClutchEngagement->FloatCurve.AddKey(1.0f, 1.0f);
	}

	setWindowHeight(170);
}

void UFGearStandardInput::setupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	mInputComp = PlayerInputComponent;
	if (mInputType == FGearInputSystemType::LEGACY)
	{
		if (mVerticalAxis != NAME_None) mInputComp->BindAxis(mVerticalAxis);
		if (mHorizontalAxis != NAME_None) mInputComp->BindAxis(mHorizontalAxis);
		if (mThrottleAxis != NAME_None) mInputComp->BindAxis(mThrottleAxis);
		if (mBrakeAxis != NAME_None) mInputComp->BindAxis(mBrakeAxis);
		if (mLeftAxis != NAME_None) mInputComp->BindAxis(mLeftAxis);
		if (mRightAxis != NAME_None) mInputComp->BindAxis(mRightAxis);
		if (mClutchAxis != NAME_None) mInputComp->BindAxis(mClutchAxis);
		if (mHandbrakeAxis != NAME_None) mInputComp->BindAxis(mHandbrakeAxis);
		if (mShiftAxis != NAME_None) mInputComp->BindAxis(mShiftAxis);
		if (mShift1Axis != NAME_None) mInputComp->BindAxis(mShift1Axis);
		if (mShift2Axis != NAME_None) mInputComp->BindAxis(mShift2Axis);
		if (mShift3Axis != NAME_None) mInputComp->BindAxis(mShift3Axis);
		if (mShift4Axis != NAME_None) mInputComp->BindAxis(mShift4Axis);
		if (mShift5Axis != NAME_None) mInputComp->BindAxis(mShift5Axis);
		if (mShift6Axis != NAME_None) mInputComp->BindAxis(mShift6Axis);
		if (mShiftRAxis != NAME_None) mInputComp->BindAxis(mShiftRAxis);
	}
	else
	{
#if ENHANCEDINPUT
		if (UEnhancedInputComponent* inputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent))
		{
			//bind actions
			if (mVerticalAction) inputComp->BindActionValue(mVerticalAction);
			if (mHorizontalAction) inputComp->BindActionValue(mHorizontalAction);
			if (mThrottleAction) inputComp->BindActionValue(mThrottleAction);
			if (mBrakeAction) inputComp->BindActionValue(mBrakeAction);
			if (mLeftAction) inputComp->BindActionValue(mLeftAction);
			if (mRightAction) inputComp->BindActionValue(mRightAction);
			if (mClutchAction) inputComp->BindActionValue(mClutchAction);
			if (mHandbrakeAction) inputComp->BindActionValue(mHandbrakeAction);
			if (mShiftAction) inputComp->BindActionValue(mShiftAction);
			if (mShift1Action) inputComp->BindActionValue(mShift1Action);
			if (mShift2Action) inputComp->BindActionValue(mShift2Action);
			if (mShift3Action) inputComp->BindActionValue(mShift3Action);
			if (mShift4Action) inputComp->BindActionValue(mShift4Action);
			if (mShift5Action) inputComp->BindActionValue(mShift5Action);
			if (mShift6Action) inputComp->BindActionValue(mShift6Action);
			if (mShiftRAction) inputComp->BindActionValue(mShiftRAction);

			//find binded action indexes and store in our action binding map
			const TArray<FEnhancedInputActionValueBinding>& actionBindings = inputComp->GetActionValueBindings();
			for (int i = 0; i < actionBindings.Num(); i++)
			{
				if (actionBindings[i].GetAction() == mVerticalAction) mInputActionBindings.Add(FGearEnhancedActionName::Vertical, i);
				if (actionBindings[i].GetAction() == mHorizontalAction) mInputActionBindings.Add(FGearEnhancedActionName::Horizontal, i);
				if (actionBindings[i].GetAction() == mThrottleAction) mInputActionBindings.Add(FGearEnhancedActionName::Throttle, i);
				if (actionBindings[i].GetAction() == mBrakeAction) mInputActionBindings.Add(FGearEnhancedActionName::Brake, i);
				if (actionBindings[i].GetAction() == mLeftAction) mInputActionBindings.Add(FGearEnhancedActionName::Left, i);
				if (actionBindings[i].GetAction() == mRightAction) mInputActionBindings.Add(FGearEnhancedActionName::Right, i);
				if (actionBindings[i].GetAction() == mClutchAction) mInputActionBindings.Add(FGearEnhancedActionName::Clutch, i);
				if (actionBindings[i].GetAction() == mHandbrakeAction) mInputActionBindings.Add(FGearEnhancedActionName::Handbrake, i);
				if (actionBindings[i].GetAction() == mShiftAction) mInputActionBindings.Add(FGearEnhancedActionName::Shift, i);
				if (actionBindings[i].GetAction() == mShift1Action) mInputActionBindings.Add(FGearEnhancedActionName::Shift1, i);
				if (actionBindings[i].GetAction() == mShift2Action) mInputActionBindings.Add(FGearEnhancedActionName::Shift2, i);
				if (actionBindings[i].GetAction() == mShift3Action) mInputActionBindings.Add(FGearEnhancedActionName::Shift3, i);
				if (actionBindings[i].GetAction() == mShift4Action) mInputActionBindings.Add(FGearEnhancedActionName::Shift4, i);
				if (actionBindings[i].GetAction() == mShift5Action) mInputActionBindings.Add(FGearEnhancedActionName::Shift5, i);
				if (actionBindings[i].GetAction() == mShift6Action) mInputActionBindings.Add(FGearEnhancedActionName::Shift6, i);
				if (actionBindings[i].GetAction() == mShiftRAction) mInputActionBindings.Add(FGearEnhancedActionName::ShiftR, i);
			}
		}
		else UE_LOG(LogTemp, Warning, TEXT("Failed to get enhanced input component..."));
#endif
	}
}

void UFGearStandardInput::initInputMapping(const AController* c)
{
#if ENHANCEDINPUT
	if (mInputMappingInitialized || mInputType == FGearInputSystemType::LEGACY) return;
	if (c == nullptr || GetNetMode() == NM_DedicatedServer) return;

	if (const APlayerController* pc = Cast<APlayerController>(c))
	{
		if (pc->IsLocalPlayerController() && mDefaultMappingContext)
		{
			if (UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer()))
			{
				subsystem->AddMappingContext(mDefaultMappingContext, 0);
				mInputMappingInitialized = true;
			}
		}
	}
#endif
}

void UFGearStandardInput::myUpdate(float dt, bool skipReading)
{
	if (!mEnabled) return;
	if (!skipReading) getInputs(dt);
	updateAxisInputs(dt);
	modifyInputs(dt);
	filterInputs();
	applyInputs();
}

float UFGearStandardInput::getAxis(FName name) const
{
	if (mInputComp == nullptr) return 0.0f;
	if (name == NAME_None) return 0.0f;
#if ENHANCEDINPUT
	if (mInputType == FGearInputSystemType::ENHANCED)
	{
		if (!mInputActionBindings.Contains(name)) return 0.0f;
		if (UEnhancedInputComponent* inputComp = Cast<UEnhancedInputComponent>(mInputComp))
		{
			const TArray<FEnhancedInputActionValueBinding>& actionBindings = inputComp->GetActionValueBindings();
			int actionIndex = mInputActionBindings[name];
			if (actionBindings.IsValidIndex(actionIndex))
			{
				return actionBindings[actionIndex].GetValue().Get<float>();
			}
		}
		return 0.0f;
	}
#endif
	return mInputComp->GetAxisValue(name);
}

//if mReadInputs is false this method should be called to provide the inputs
//throttle [0, 1], brake [0, 1], steer [-1, 1], clutch [0, 1], hb [false, true]
//sequential gear [-1 1], manual gear [0 7]
void UFGearStandardInput::setInputs(float throttle, float brake, float steer, float clutch, int gear, bool hb)
{
	mForward = throttle;
	mBackward = brake;
	mHorizontal = steer;
	mClutch = clutch;
	mHandBrakeInput = hb;
	mGearboxInput = 0;
	if (mShifterType != FGearShifterType::MANUAL)
	{
		if (gear != mLastGearboxInput) mGearboxInput = gear;
		mLastGearboxInput = gear;
	}
	else mGearboxInput = gear;
}

//overrides final values and calls apply, currently used by cruise control in arcade assists
void UFGearStandardInput::overrideFinalInputs(float throttle, float brake, float steer, float clutch, int gear, bool hb)
{
	mThrottleInput = throttle;
	mBrakeInput = brake;
	mSteerInput = steer;
	mClutchInput = clutch;
	mGearboxInput = gear;
	mHandBrakeInput = hb;
	applyInputs();
}

void UFGearStandardInput::resetInputs()
{
	//final inputs
	mThrottleInput = mBrakeInput = mSteerInput = mClutchInput = 0.0f;
	
	//raw inputs
	mForward = mBackward = mHorizontal = mClutch = 0.0f;

	//resulting inputs
	mThrottleState = mGearboxInput = mLastGearboxInput = 0;
	mThrottleValue = mBrakeValue = mClutchValue = 0.0f;
	mTargetSteeringInput = mCurrentSteeringInput = mCurrentSteeringLimit = 0.0f;
	mHandBrakeInput = false;

	//vehicle itself
	if (mVehicle == nullptr) return;
	mVehicle->getEngine()->setThrottle(0.0f);
	mVehicle->getTransmission()->setThrottleState(0);
	mVehicle->getTransmission()->setClutchState(1.0f);
	mVehicle->setBraking(0.0f, false);
	mVehicle->setSteering(0.0f);
}

void UFGearStandardInput::getInputs(float dt)
{
	if (mReadInputs)
	{
		const bool enhanced = mInputType == FGearInputSystemType::ENHANCED;
		//axis inputs
		if (mCombinedAxis)
		{
			//combined vertical
			float vertical = getAxis(enhanced ? FGearEnhancedActionName::Vertical : mVerticalAxis);
			mForward = vertical > 0.0f ? vertical : 0.0f;
			mBackward = vertical < 0.0f ? -vertical : 0.0f;
			//combined horizontal
			mHorizontal = getAxis(enhanced ? FGearEnhancedActionName::Horizontal : mHorizontalAxis);
		}
		else
		{
			//seperate vertical throttle/brake
			mForward = getAxis(enhanced ? FGearEnhancedActionName::Throttle : mThrottleAxis);
			mBackward = getAxis(enhanced ? FGearEnhancedActionName::Brake : mBrakeAxis);
			//seperate horizontal left/right
			float left = getAxis(enhanced ? FGearEnhancedActionName::Left : mLeftAxis);
			float right = getAxis(enhanced ? FGearEnhancedActionName::Right : mRightAxis);
			mHorizontal = right - left;
		}

		//clutch
		mClutch = getAxis(enhanced ? FGearEnhancedActionName::Clutch : mClutchAxis);

		//clutch axis is between -1 and 1, scale down to [0,1]
		if (mControllerType == FGearControllerType::WHEEL)
		{
			mClutch = (mClutch + 1.0f) / 2.0f;
		}

		//shifter
		mGearboxInput = 0;
		if (mShifterType != FGearShifterType::MANUAL)
		{
			int shiftAxis = (int)getAxis(enhanced ? FGearEnhancedActionName::Shift : mShiftAxis);
			if (shiftAxis != mLastGearboxInput) mGearboxInput = shiftAxis;
			mLastGearboxInput = shiftAxis;
		}
		else if (mShifterType == FGearShifterType::MANUAL)
		{
			if (getAxis(enhanced ? FGearEnhancedActionName::Shift1 : mShift1Axis) > 0.0f) mGearboxInput = 1;
			else if (getAxis(enhanced ? FGearEnhancedActionName::Shift2 : mShift2Axis) > 0.0f) mGearboxInput = 2;
			else if (getAxis(enhanced ? FGearEnhancedActionName::Shift3 : mShift3Axis) > 0.0f) mGearboxInput = 3;
			else if (getAxis(enhanced ? FGearEnhancedActionName::Shift4 : mShift4Axis) > 0.0f) mGearboxInput = 4;
			else if (getAxis(enhanced ? FGearEnhancedActionName::Shift5 : mShift5Axis) > 0.0f) mGearboxInput = 5;
			else if (getAxis(enhanced ? FGearEnhancedActionName::Shift6 : mShift6Axis) > 0.0f) mGearboxInput = 6;
			else if (getAxis(enhanced ? FGearEnhancedActionName::ShiftR : mShiftRAxis) > 0.0f) mGearboxInput = 7;
		}

		//other inputs
		mHandBrakeInput = getAxis(enhanced ? FGearEnhancedActionName::Handbrake : mHandbrakeAxis) != 0.0f;
	}

	//override input event
	if (mVehicle->onOverrideInput.IsBound())
	{
		mVehicle->onOverrideInput.Broadcast(dt);
	}
}

float UFGearStandardInput::getSteerSpeed() const
{
	return mSteerSpeedCurve->GetFloatValue(FMath::Abs(mVehicle->getKMHSpeed()));
}

float UFGearStandardInput::getSteerLimit() const
{
	return mSteerLimitCurve->GetFloatValue(FMath::Abs(mVehicle->getKMHSpeed()));
}

void UFGearStandardInput::updateAxisInputs(float dt)
{
	//throttle input
	if (mForward > 0.0f) mThrottleState = 1;
	else if (mBackward > 0.0f) mThrottleState = -1;
	else mThrottleState = 0;

	//apply throttle dead zone
	float throttleVerticle = mForward - mThrottleDeadzone;
	if (throttleVerticle < 0.0f) throttleVerticle = 0.0f;
	else throttleVerticle = throttleVerticle / (1.0f - mThrottleDeadzone) * mForward;

	//throttle input
	if (throttleVerticle > 0.0f)
	{
		if (mThrottleValue < throttleVerticle)
		{
			mThrottleValue += mThrottleSensivity * dt;
			if (mThrottleValue > throttleVerticle) mThrottleValue = throttleVerticle;
		}
		else if (mThrottleValue > throttleVerticle)
		{
			mThrottleValue -= mThrottleSensivity * dt;
			if (mThrottleValue < throttleVerticle) mThrottleValue = throttleVerticle;
		}
	}
	else if (mThrottleValue > 0.0f)
	{
		if (mThrottleValue > mThrottleRange / 100.0f)
		{
			mThrottleValue = mThrottleRange / 100.0f;
		}

		mThrottleValue -= mThrottleSensivity * dt;
		if (mThrottleValue < 0.0f) mThrottleValue = 0.0f;
	}

	//apply brake dead zone
	float brakeVerticle = mBackward - mBrakingDeadzone;
	if (brakeVerticle < 0.0f) brakeVerticle = 0.0f;
	else brakeVerticle = brakeVerticle / (1.0f - mBrakingDeadzone) * mBackward;

	//brake input
	if (brakeVerticle > 0.0f)
	{
		if (mBrakeValue < brakeVerticle)
		{
			mBrakeValue += mBrakingSensivity * dt;
			if (mBrakeValue > brakeVerticle) mBrakeValue = brakeVerticle;
		}
		else if (mBrakeValue > brakeVerticle)
		{
			mBrakeValue -= mBrakingSensivity * dt;
			if (mBrakeValue < brakeVerticle) mBrakeValue = brakeVerticle;
		}
	}
	else if (mBrakeValue > 0.0f)
	{
		if (mBrakeValue > mBrakingRange / 100.0f)
		{
			mBrakeValue = mBrakingRange / 100.0f;
		}

		mBrakeValue -= mBrakingSensivity * dt;
		if (mBrakeValue < 0.0f) mBrakeValue = 0.0f;
	}

	//apply clutch dead zone
	float clutchVerticle = mClutch - mClutchDeadzone;
	if (clutchVerticle < 0.0f) clutchVerticle = 0.0f;
	else clutchVerticle = clutchVerticle / (1.0f - mClutchDeadzone);

	//clutch input
	if (mClutchValue < clutchVerticle)
	{
		mClutchValue += mClutchSensivity * dt;
		if (mClutchValue > clutchVerticle) mClutchValue = clutchVerticle;
	}
	else if (mClutchValue > clutchVerticle)
	{
		mClutchValue -= mClutchSensivity * dt;
		if (mClutchValue < clutchVerticle) mClutchValue = clutchVerticle;
	}

	//steering input & apply dead zone
	mTargetSteeringInput = FMath::Abs(mHorizontal) - mSteerDeadzone;
	if (mTargetSteeringInput < 0.0f) mTargetSteeringInput = 0.0f;
	else mTargetSteeringInput = mTargetSteeringInput / (1.0f - mSteerDeadzone) * FGearUtility::Sign(mHorizontal);
}

void UFGearStandardInput::modifyInputs(float dt)
{
	//steering assist
	mSteerAssistActive = false;
	if (mSteeringAssist > 0.0f)
	{
		float targetAngle = 0.0f;
		if (mVehicle->getForwardSpeed() > 1.0f)
		{
			FVector velDir = mVehicle->getVelocityDir();
			FVector dir = mVehicle->getForwardDir();
			float sign = FGearUtility::Sign(FVector::CrossProduct(dir, velDir).Z);
			targetAngle = sign * FGearUtility::Angle(dir, velDir);
			//threshold
			float minAngle = mSteeringAssistThresholdAngle;
			float absTarget = FMath::Abs(targetAngle);
			if (absTarget < minAngle) targetAngle = 0.0f;
			else targetAngle = FGearUtility::Sign(targetAngle) * (absTarget - minAngle);
		}
		//apply
		float maxAngle = mVehicle->getMaxSteeringAngle();
		mTargetSteeringInput += (0.01f * mSteeringAssist) * (targetAngle / maxAngle);
		//clamp
		mTargetSteeringInput = FMath::Clamp(mTargetSteeringInput, -1.0f, 1.0f);
		//active?
		mSteerAssistActive = targetAngle != 0.0f;
	}

	//steering range limit
	mCurrentSteeringLimit = 1.0f;
	if (mControllerType != FGearControllerType::WHEEL && getSteerLimit() > 0.0f && mVehicle->getKMHSpeed() > 1.0f)
	{
		mCurrentSteeringLimit = 0.01f * getSteerLimit();
		if (FMath::Abs(mTargetSteeringInput) > mCurrentSteeringLimit)
		{
			mTargetSteeringInput = FGearUtility::Sign(mTargetSteeringInput) * mCurrentSteeringLimit;
		}
	}

	//steering speed limit
	if (mCurrentSteeringInput != mTargetSteeringInput)
	{
		float steerSpeed = mSteerSensivity;

		if (mControllerType != FGearControllerType::WHEEL)
		{
			steerSpeed = getSteerSpeed();
			//counter steer coeff.
			if (mSteerInputGravity != 0.0f)
			{
				if (FMath::Sign(mCurrentSteeringInput) != FMath::Sign(mTargetSteeringInput))
				{
					steerSpeed *= mSteerInputGravity;
				}
			}
			else if (mTargetSteeringInput == 0.0f) steerSpeed = 0.0f;
		}

		//apply
		float deltaInput = FMath::Abs(mTargetSteeringInput - mCurrentSteeringInput);
		float steerTime = FMath::Clamp(steerSpeed / deltaInput * dt, 0.0f, 1.0f);
		mCurrentSteeringInput = FMath::Lerp(mCurrentSteeringInput, mTargetSteeringInput, steerTime);
	}
}

void UFGearStandardInput::filterInputs()
{
	//apply throttle dead zone & range
	float rangeFactor = 100.0f / mThrottleRange;
	mThrottleInput = FMath::Abs(mThrottleValue) * rangeFactor;
	mThrottleInput = FMath::Clamp(mThrottleInput, 0.0f, 1.0f);

	//apply braking dead zone & range
	rangeFactor = 100.0f / mBrakingRange;
	mBrakeInput = FMath::Abs(mBrakeValue)  * rangeFactor;
	mBrakeInput = FMath::Clamp(mBrakeInput, 0.0f, 1.0f);

	//apply clutch dead zone & range
	rangeFactor = 100.0f / mClutchRange;
	float clutchInput = FMath::Abs(mClutchValue) * rangeFactor;
	mClutchInput = FMath::Clamp(clutchInput, 0.0f, 1.0f);

	//apply steering dead zone
	rangeFactor = 100.0f / mSteerRange;
	mSteerInput = mCurrentSteeringInput * rangeFactor;
	mSteerInput = FMath::Clamp(mSteerInput, -mCurrentSteeringLimit, mCurrentSteeringLimit);

	//apply engagement curves
	mThrottleInput = mThrottleEngagement->GetFloatValue(mThrottleInput);
	mBrakeInput = mBrakeEngagement->GetFloatValue(mBrakeInput);
	mClutchInput = mClutchEngagement->GetFloatValue(mClutchInput);
}

void UFGearStandardInput::applyInputs()
{
	//throttle free, full brake, force neutral gear
	if (mStartGridMode)
	{
		mVehicle->getEngine()->setThrottle(mThrottleInput);
		mVehicle->setBraking(1.0f, false);
		mVehicle->getTransmission()->setCurrentGear(0, true);
		mVehicle->getTransmission()->setPauseTime(1.0f);
		return;
	}

	//gearbox input
	if (mShifterType == FGearShifterType::SEQUENTIAL)
	{
		if (mGearboxInput > 0) mVehicle->getTransmission()->gearUp(false);
		else if (mGearboxInput < 0) mVehicle->getTransmission()->gearDown(false);
	}
	else if (mShifterType == FGearShifterType::MANUAL)
	{
		mVehicle->getTransmission()->setCurrentGear(mGearboxInput, false);
	}

	//clutch input
	mVehicle->getTransmission()->setClutchState(1.0f - mClutchInput);

	//invert throttle/brake inputs in reverse(auto)
	bool invertVertical = mVehicle->getTransmission()->isAutoChange() &&
						  mVehicle->getTransmission()->isAutoReverse() &&
						  mVehicle->getTransmission()->getCurGear() < 0;
	if (invertVertical)
	{
		float tmp = mThrottleInput;
		mThrottleInput = mBrakeInput;
		mBrakeInput = tmp;
	}

	//apply inputs
	mVehicle->getEngine()->setThrottle(mThrottleInput);
	mVehicle->getTransmission()->setThrottleState(mThrottleState);
	mVehicle->setBraking(mBrakeInput, mHandBrakeInput);
	mVehicle->setSteering(mSteerInput);
}

void UFGearStandardInput::setEnabled(bool enable, bool resetInputs)
{
	mEnabled = enable;
	if (resetInputs) this->resetInputs();
}

void UFGearStandardInput::setStartGridMode(bool b)
{
	mStartGridMode = b;
	if (mVehicle == nullptr) return;
	mVehicle->getTransmission()->setPauseTime(0.0f);
}

void UFGearStandardInput::saveState(FFGearVehicleState& state)
{
	state.mInput.mBrake = mBrakeInput;
	state.mInput.mClutch = mClutchInput;
	state.mInput.mThrottle = mThrottleInput;
	state.mInput.mGear = mGearboxInput;
	state.mInput.mHandbrake = mHandBrakeInput;
	state.mInput.mSteer = mSteerInput;
	state.mInput.mThrottleState = mThrottleState;
}

void UFGearStandardInput::loadState(FFGearVehicleState& state)
{
	mBrakeInput = state.mInput.mBrake;
	mClutchInput = state.mInput.mClutch;
	mThrottleInput = state.mInput.mThrottle;
	mGearboxInput = state.mInput.mGear;
	mHandBrakeInput = state.mInput.mHandbrake;
	mSteerInput = state.mInput.mSteer;
	mThrottleState = state.mInput.mThrottleState;
	applyInputs();
}

#if WITH_EDITOR
bool UFGearStandardInput::CanEditChange(const FProperty* Property) const
{
	const bool parentVal = Super::CanEditChange(Property);

	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mInputType))
#if ENHANCEDINPUT
		return true;
#else
		return false;
#endif
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mDefaultMappingContext) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mVerticalAction) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mHorizontalAction) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mThrottleAction) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mBrakeAction) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mLeftAction) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mRightAction) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mClutchAction) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mHandbrakeAction) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mShiftAction) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mShift1Action) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mShift2Action) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mShift3Action) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mShift4Action) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mShift5Action) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mShift6Action) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mShiftRAction)) return mInputType == FGearInputSystemType::ENHANCED;

	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mClutchAxis) ||
		Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mHandbrakeAxis)) return mInputType == FGearInputSystemType::LEGACY;

	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mSteerSpeedCurve))	return mControllerType != FGearControllerType::WHEEL;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mSteerLimitCurve))	return mControllerType != FGearControllerType::WHEEL;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mSteerSensivity))	return mControllerType == FGearControllerType::WHEEL;

	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mVerticalAxis))	return mInputType == FGearInputSystemType::LEGACY && mCombinedAxis;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mHorizontalAxis))	return mInputType == FGearInputSystemType::LEGACY && mCombinedAxis;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mThrottleAxis))	return mInputType == FGearInputSystemType::LEGACY && !mCombinedAxis;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mBrakeAxis))		return mInputType == FGearInputSystemType::LEGACY && !mCombinedAxis;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mLeftAxis))		return mInputType == FGearInputSystemType::LEGACY && !mCombinedAxis;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mRightAxis))		return mInputType == FGearInputSystemType::LEGACY && !mCombinedAxis;

	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mShiftAxis))		return mInputType == FGearInputSystemType::LEGACY && mShifterType == FGearShifterType::SEQUENTIAL;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mShift1Axis))		return mInputType == FGearInputSystemType::LEGACY && mShifterType == FGearShifterType::MANUAL;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mShift2Axis))		return mInputType == FGearInputSystemType::LEGACY && mShifterType == FGearShifterType::MANUAL;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mShift3Axis))		return mInputType == FGearInputSystemType::LEGACY && mShifterType == FGearShifterType::MANUAL;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mShift4Axis))		return mInputType == FGearInputSystemType::LEGACY && mShifterType == FGearShifterType::MANUAL;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mShift5Axis))		return mInputType == FGearInputSystemType::LEGACY && mShifterType == FGearShifterType::MANUAL;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mShift6Axis))		return mInputType == FGearInputSystemType::LEGACY && mShifterType == FGearShifterType::MANUAL;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearStandardInput, mShiftRAxis))		return mInputType == FGearInputSystemType::LEGACY && mShifterType == FGearShifterType::MANUAL;

	return parentVal;
}
#endif

FVector2D UFGearStandardInput::drawTelemetry(UCanvas * Canvas, UFont * Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p)
{
	p = Super::drawTelemetry(Canvas, Font, textColor, renderInfo, p);
	Canvas->SetLinearDrawColor(textColor);
	Canvas->DrawText(Font, "Input", p.X, p.Y, 1.0f, 1.0f, renderInfo);
	FGearUtility::drawLine(Canvas, p.X, p.Y + 18, p.X + 31, p.Y + 18, textColor);
	Canvas->DrawText(Font, "Enabled : " + UKismetStringLibrary::Conv_BoolToString(mEnabled), p.X, p.Y + 20, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Controller: %s"), *FGearUtility::EnumToString(stringify(FGearControllerType), mControllerType)), p.X, p.Y + 35, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Shifter: %s"), *FGearUtility::EnumToString(stringify(FGearShifterType), mShifterType)), p.X, p.Y + 50, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Throttle: %%%.0f"), (100.0f * mVehicle->getEngine()->getThrottle())), p.X, p.Y + 65, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Brakes: %%%.0f"), (100.0f * mVehicle->getAxle(0)->getLeftWheel()->getBraking())), p.X, p.Y + 80, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, "Handbrake: " + UKismetStringLibrary::Conv_BoolToString(mVehicle->isHandbrakeOn()), p.X, p.Y + 95, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Steering: %%%.0f"), (100.0f * mVehicle->getNormalizedSteering())), p.X, p.Y + 110, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Steer Speed: %.1f"), getSteerSpeed()), p.X, p.Y + 125, 1.0f, 1.0f, renderInfo);
	Canvas->DrawText(Font, FString::Printf(TEXT("Steer Limit: %%%.0f"), getSteerLimit()), p.X, p.Y + 140, 1.0f, 1.0f, renderInfo);
	p.Y += getWindowHeight();
	return p;
}