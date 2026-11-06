// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearSerializer.h"
#include "FGearVehicle.h"
#include "FGearEngine.h"
#include "FGearTransmission.h"
#include "FGearStandardInput.h"
#include "FGearAeroDynamics.h"
#include "FGearAxle.h"
#include "FGearWheelOptions.h"
#include "FGearTire.h"
#include "FGearWheel.h"
#include "Engine/StaticMesh.h"
#include "Runtime/JsonUtilities/Public/JsonObjectConverter.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"

#if WITH_EDITOR
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"

FString FGearSerializer::getFilePath(bool isSave)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		TArray<FString> filenames;
		bool isSuccess = isSave ? DesktopPlatform->SaveFileDialog(nullptr, FString("Save Settings"), FString(), FString(), FString("JSON file|*json"), 0, filenames) :
			DesktopPlatform->OpenFileDialog(nullptr, FString("Open Settings"), FString(), FString(), FString("JSON file|*json"), 0, filenames);

		if (isSuccess && filenames.IsValidIndex(0))	return *filenames[0];
	}
	return FString();
}

#endif

bool FGearSerializer::saveSettings(AFGearVehicle* v, FString path)
{
	TSharedPtr<FJsonObject> jObject = MakeShareable(new FJsonObject());

	// FGearVehicle|General
	jObject->SetNumberField("UpdateRate", v->mUpdateRate);
	jObject->SetNumberField("Mass", v->mMass);
	jObject->SetArrayField("InertiaScale", fromVectorToArray(v->mInertiaScale));
	jObject->SetArrayField("CenterofMass", fromVectorToArray(v->mCenterofMass));
	jObject->SetBoolField("CustomGravity", v->mCustomGravity);
	jObject->SetArrayField("Gravity", fromVectorToArray(v->mGravity));
	jObject->SetNumberField("HandBrakePower", v->mHandBrakePower);
	jObject->SetNumberField("WheelFrictionCoeff", v->mWheelFrictionCoeff);
	jObject->SetNumberField("StickyTireSpeed", v->mStickyTireSpeed);
	jObject->SetNumberField("OptimumSlipWeight", v->mOptimumSlipWeight);
	jObject->SetNumberField("ReactionScale", v->mReactionScale);
	jObject->SetBoolField("DetectGround", v->mDetectGround);
	jObject->SetNumberField("HardContactScale", v->mHardContactScale);
	jObject->SetBoolField("RelaxationDownforce", v->mRelaxationDownforce);
	jObject->SetBoolField("TirePenetration", v->mTirePenetration);

	// FGearVehicle|Tracing
	jObject->SetNumberField("QueryType", (uint8)v->mQueryType);
	jObject->SetNumberField("Rays", v->mRays);
	jObject->SetNumberField("LateralRays", v->mLateralRays);
	jObject->SetNumberField("QueryMobilityType", (uint8)v->mQueryMobilityType);
	jObject->SetNumberField("TraceChannel", (uint8)v->mTraceChannel);
	jObject->SetNumberField("IgnoreMask", v->mIgnoreMask);
	jObject->SetBoolField("TraceComplex", v->mTraceComplex);
	jObject->SetStringField("ConvexAsset", getObjPath(v->mConvexAsset));
	jObject->SetNumberField("ConvexUp", (uint8)v->mConvexUp);
	jObject->SetArrayField("ConvexRotation", fromRotatorToArray(v->mConvexRotation));
	jObject->SetBoolField("ConvexVisible", v->mConvexVisible);

	// FGearVehicle|Telemetry
	jObject->SetStringField("Telemetry", fromStructToString(FGearTelemetryOptions::StaticStruct(), &v->mTelemetry));

	// FGearVehicle|DriveAssists
	jObject->SetNumberField("ABS", v->mABS);
	jObject->SetNumberField("ASR", v->mASR);
	jObject->SetNumberField("UnderSteerAssist", v->mUnderSteerAssist);
	jObject->SetNumberField("MinUnderSteerAngle", v->mMinUnderSteerAngle);
	jObject->SetNumberField("OverSteerAssist", v->mOverSteerAssist);
	jObject->SetNumberField("MinOverSteerAngle", v->mMinOverSteerAngle);
	jObject->SetNumberField("FrontAntiRollPower", v->mFrontAntiRollPower);
	jObject->SetNumberField("RearAntiRollPower", v->mRearAntiRollPower);
	jObject->SetNumberField("MaxHillHolderSpeed", v->mMaxHillHolderSpeed);

	// FGearVehicle|Optimization
	jObject->SetNumberField("SleepLinearThreshold", v->mSleepLinearThreshold);
	jObject->SetNumberField("SleepAngularThreshold", v->mSleepAngularThreshold);
	jObject->SetNumberField("SleepWaitingTime", v->mSleepWaitingTime);
	jObject->SetNumberField("WakeUpSources", v->mWakeUpSources);

	// FGearEngine
	jObject->SetStringField("Engine_TorqueCurve", getObjPath(v->mEngine->mTorqueCurve));
	jObject->SetNumberField("Engine_TorqueScale", v->mEngine->mTorqueScale);
	jObject->SetNumberField("Engine_FrictionTorque", v->mEngine->mFrictionTorque);
	jObject->SetNumberField("Engine_IdleRpm", v->mEngine->mIdleRpm);
	jObject->SetNumberField("Engine_LimitRpm", v->mEngine->mLimitRpm);
	jObject->SetNumberField("Engine_LimiterTime", v->mEngine->mLimiterTime);
	jObject->SetNumberField("Engine_EngineInertia", v->mEngine->mEngineInertia);

	// FGearTransmission
	jObject->SetBoolField("Transmission_AutoChange", v->mTransmission->mAutoChange);
	jObject->SetBoolField("Transmission_AutoReverse", v->mTransmission->mAutoReverse);
	jObject->SetBoolField("Transmission_AutoClutch", v->mTransmission->mAutoClutch);
	jObject->SetBoolField("Transmission_RevMatching", v->mTransmission->mRevMatching);
	jObject->SetNumberField("Transmission_ChangeTime", v->mTransmission->mChangeTime);
	jObject->SetNumberField("Transmission_GearUpRatio", v->mTransmission->mGearUpRatio);
	jObject->SetNumberField("Transmission_GearDownRatio", v->mTransmission->mGearDownRatio);
	jObject->SetNumberField("Transmission_GearUpVelocityBias", v->mTransmission->mGearUpVelocityBias);
	jObject->SetNumberField("Transmission_GearDownVelocityBias", v->mTransmission->mGearDownVelocityBias);
	jObject->SetNumberField("Transmission_ClutchTime", v->mTransmission->mClutchTime);
	jObject->SetNumberField("Transmission_ClutchScale", v->mTransmission->mClutchScale);
	jObject->SetArrayField("Transmission_GearRatios", fromTArrayToJsonArray(v->mTransmission->mGearRatios));
	jObject->SetBoolField("Transmission_ClutchAssist", v->mTransmission->mClutchAssist);
	jObject->SetNumberField("Transmission_ClutchScaleMin", v->mTransmission->mClutchScaleMin);
	jObject->SetNumberField("Transmission_ClutchScaleMax", v->mTransmission->mClutchScaleMax);

	// FGearStandardInput
	jObject->SetBoolField("StandardInput_Enabled", v->mStandardInput->mEnabled);
	jObject->SetNumberField("StandardInput_InputType", (uint8)v->mStandardInput->mInputType);
	jObject->SetBoolField("StandardInput_CombinedAxis", v->mStandardInput->mCombinedAxis);
	jObject->SetNumberField("StandardInput_ControllerType", (uint8)v->mStandardInput->mControllerType);
	jObject->SetNumberField("StandardInput_ShifterType", (uint8)v->mStandardInput->mShifterType);
	jObject->SetStringField("StandardInput_VerticalAxis", v->mStandardInput->mVerticalAxis.ToString());
	jObject->SetStringField("StandardInput_HorizontalAxis", v->mStandardInput->mHorizontalAxis.ToString());
	jObject->SetStringField("StandardInput_ThrottleAxis", v->mStandardInput->mThrottleAxis.ToString());
	jObject->SetStringField("StandardInput_BrakeAxis", v->mStandardInput->mBrakeAxis.ToString());
	jObject->SetStringField("StandardInput_LeftAxis", v->mStandardInput->mLeftAxis.ToString());
	jObject->SetStringField("StandardInput_RightAxis", v->mStandardInput->mRightAxis.ToString());
	jObject->SetStringField("StandardInput_ClutchAxis", v->mStandardInput->mClutchAxis.ToString());
	jObject->SetStringField("StandardInput_HandbrakeAxis", v->mStandardInput->mHandbrakeAxis.ToString());
	jObject->SetStringField("StandardInput_ShiftAxis", v->mStandardInput->mShiftAxis.ToString());
	jObject->SetStringField("StandardInput_Shift1Axis", v->mStandardInput->mShift1Axis.ToString());
	jObject->SetStringField("StandardInput_Shift2Axis", v->mStandardInput->mShift2Axis.ToString());
	jObject->SetStringField("StandardInput_Shift3Axis", v->mStandardInput->mShift3Axis.ToString());
	jObject->SetStringField("StandardInput_Shift4Axis", v->mStandardInput->mShift4Axis.ToString());
	jObject->SetStringField("StandardInput_Shift5Axis", v->mStandardInput->mShift5Axis.ToString());
	jObject->SetStringField("StandardInput_Shift6Axis", v->mStandardInput->mShift6Axis.ToString());
	jObject->SetStringField("StandardInput_ShiftRAxis", v->mStandardInput->mShiftRAxis.ToString());
	jObject->SetNumberField("StandardInput_SteerSensivity", v->mStandardInput->mSteerSensivity);
	jObject->SetStringField("StandardInput_SteerSpeedCurve", getObjPath(v->mStandardInput->mSteerSpeedCurve));
	jObject->SetStringField("StandardInput_SteerLimitCurve", getObjPath(v->mStandardInput->mSteerLimitCurve));
	jObject->SetNumberField("StandardInput_SteerInputGravity", v->mStandardInput->mSteerInputGravity);
	jObject->SetNumberField("StandardInput_SteerRange", v->mStandardInput->mSteerRange);
	jObject->SetNumberField("StandardInput_SteerDeadzone", v->mStandardInput->mSteerDeadzone);
	jObject->SetNumberField("StandardInput_SteeringAssist", v->mStandardInput->mSteeringAssist);
	jObject->SetNumberField("StandardInput_SteeringAssistThresholdAngle", v->mStandardInput->mSteeringAssistThresholdAngle);
	jObject->SetNumberField("StandardInput_ThrottleSensivity", v->mStandardInput->mThrottleSensivity);
	jObject->SetNumberField("StandardInput_ThrottleRange", v->mStandardInput->mThrottleRange);
	jObject->SetNumberField("StandardInput_ThrottleDeadzone", v->mStandardInput->mThrottleDeadzone);
	jObject->SetStringField("StandardInput_ThrottleEngagement", getObjPath(v->mStandardInput->mThrottleEngagement));
	jObject->SetNumberField("StandardInput_BrakingSensivity", v->mStandardInput->mBrakingSensivity);
	jObject->SetNumberField("StandardInput_BrakingRange", v->mStandardInput->mBrakingRange);
	jObject->SetNumberField("StandardInput_BrakingDeadzone", v->mStandardInput->mBrakingDeadzone);
	jObject->SetStringField("StandardInput_BrakeEngagement", getObjPath(v->mStandardInput->mBrakeEngagement));
	jObject->SetNumberField("StandardInput_ClutchSensivity", v->mStandardInput->mClutchSensivity);
	jObject->SetNumberField("StandardInput_ClutchRange", v->mStandardInput->mClutchRange);
	jObject->SetNumberField("StandardInput_ClutchDeadzone", v->mStandardInput->mClutchDeadzone);
	jObject->SetStringField("StandardInput_ClutchEngagement", getObjPath(v->mStandardInput->mClutchEngagement));
#if ENHANCEDINPUT
	jObject->SetStringField("StandardInput_DefaultMappingContext", getObjPath((UObject*)v->mStandardInput->mDefaultMappingContext));
	jObject->SetStringField("StandardInput_VerticalAction", getObjPath((UObject*)v->mStandardInput->mVerticalAction));
	jObject->SetStringField("StandardInput_HorizontalAction", getObjPath((UObject*)v->mStandardInput->mHorizontalAction));
	jObject->SetStringField("StandardInput_ThrottleAction", getObjPath((UObject*)v->mStandardInput->mThrottleAction));
	jObject->SetStringField("StandardInput_BrakeAction", getObjPath((UObject*)v->mStandardInput->mBrakeAction));
	jObject->SetStringField("StandardInput_LeftAction", getObjPath((UObject*)v->mStandardInput->mLeftAction));
	jObject->SetStringField("StandardInput_RightAction", getObjPath((UObject*)v->mStandardInput->mRightAction));
	jObject->SetStringField("StandardInput_ClutchAction", getObjPath((UObject*)v->mStandardInput->mClutchAction));
	jObject->SetStringField("StandardInput_HandbrakeAction", getObjPath((UObject*)v->mStandardInput->mHandbrakeAction));
	jObject->SetStringField("StandardInput_ShiftAction", getObjPath((UObject*)v->mStandardInput->mShiftAction));
	jObject->SetStringField("StandardInput_Shift1Action", getObjPath((UObject*)v->mStandardInput->mShift1Action));
	jObject->SetStringField("StandardInput_Shift2Action", getObjPath((UObject*)v->mStandardInput->mShift2Action));
	jObject->SetStringField("StandardInput_Shift3Action", getObjPath((UObject*)v->mStandardInput->mShift3Action));
	jObject->SetStringField("StandardInput_Shift4Action", getObjPath((UObject*)v->mStandardInput->mShift4Action));
	jObject->SetStringField("StandardInput_Shift5Action", getObjPath((UObject*)v->mStandardInput->mShift5Action));
	jObject->SetStringField("StandardInput_Shift6Action", getObjPath((UObject*)v->mStandardInput->mShift6Action));
	jObject->SetStringField("StandardInput_ShiftRAction", getObjPath((UObject*)v->mStandardInput->mShiftRAction));
#endif

	// FGearAeroDynamics
	jObject->SetNumberField("AeroDynamics_DragCoefficient", v->mAeroDynamics->mDragCoefficient);
	jObject->SetNumberField("AeroDynamics_DownForceCoefficient", v->mAeroDynamics->mDownForceCoefficient);
	jObject->SetNumberField("AeroDynamics_DownForceXOffset", v->mAeroDynamics->mDownForceXOffset);

	TArray<UActorComponent*> componentList;
	v->GetComponents(componentList);
	for (UActorComponent* comp : componentList)
	{
		if (comp == nullptr) continue;
		// FGearAxle
		if (comp->GetClass() == UFGearAxle::StaticClass())
		{
			UFGearAxle* axle = Cast<UFGearAxle>(comp);
			jObject->SetNumberField(axle->GetName() + "_DifferentialType", (uint8)axle->mDifferentialType);
			jObject->SetNumberField(axle->GetName() + "_DiffStrength", axle->mDiffStrength);
			jObject->SetNumberField(axle->GetName() + "_TorqueShare", axle->mTorqueShare);
			jObject->SetNumberField(axle->GetName() + "_FinalGearRatio", axle->mFinalGearRatio);
			jObject->SetNumberField(axle->GetName() + "_MaxSteerAngle", axle->mMaxSteerAngle);
			jObject->SetNumberField(axle->GetName() + "_AckermanCoeff", axle->mAckermanCoeff);
			jObject->SetNumberField(axle->GetName() + "_AckermanReferenceIndex", axle->mAckermanReferenceIndex);
			jObject->SetBoolField(axle->GetName() + "_HasHandbrake", axle->mHasHandbrake);
			jObject->SetNumberField(axle->GetName() + "_CamberAngle", axle->mCamberAngle);
			jObject->SetNumberField(axle->GetName() + "_ToeAngle", axle->mToeAngle);
		}
		// FGearWheelOptions
		else if (comp->GetClass() == UFGearWheelOptions::StaticClass())
		{
			UFGearWheelOptions* wheelOpts = Cast<UFGearWheelOptions>(comp);
			jObject->SetStringField(wheelOpts->GetName() + "_TireModelClass", wheelOpts->mTireModelClass->GetPathName());
			jObject->SetNumberField(wheelOpts->GetName() + "_LateralFriction", wheelOpts->mLateralFriction);
			jObject->SetNumberField(wheelOpts->GetName() + "_LongitudinalFriction", wheelOpts->mLongitudinalFriction);
			jObject->SetNumberField(wheelOpts->GetName() + "_LateralScale", wheelOpts->mLateralScale);
			jObject->SetNumberField(wheelOpts->GetName() + "_LongitudinalScale", wheelOpts->mLongitudinalScale);
			jObject->SetNumberField(wheelOpts->GetName() + "_OverturnScale", wheelOpts->mOverturnScale);
			jObject->SetNumberField(wheelOpts->GetName() + "_RollMomentScale", wheelOpts->mRollMomentScale);
			jObject->SetNumberField(wheelOpts->GetName() + "_SelfAlignScale", wheelOpts->mSelfAlignScale);
			jObject->SetNumberField(wheelOpts->GetName() + "_TirePressure", wheelOpts->mTirePressure);
			jObject->SetNumberField(wheelOpts->GetName() + "_Radius", wheelOpts->mRadius);
			jObject->SetNumberField(wheelOpts->GetName() + "_Width", wheelOpts->mWidth);
			jObject->SetNumberField(wheelOpts->GetName() + "_SpacerOffset", wheelOpts->mSpacerOffset);
			jObject->SetNumberField(wheelOpts->GetName() + "_RimOffset", wheelOpts->mRimOffset);
			jObject->SetNumberField(wheelOpts->GetName() + "_CastOffset", wheelOpts->mCastOffset);
			jObject->SetNumberField(wheelOpts->GetName() + "_Mass", wheelOpts->mMass);
			jObject->SetNumberField(wheelOpts->GetName() + "_BrakeTorque", wheelOpts->mBrakeTorque);
			jObject->SetNumberField(wheelOpts->GetName() + "_SuspensionUpTravel", wheelOpts->mSuspensionUpTravel);
			jObject->SetNumberField(wheelOpts->GetName() + "_SuspensionDownTravel", wheelOpts->mSuspensionDownTravel);
			jObject->SetNumberField(wheelOpts->GetName() + "_SuspensionSpring", wheelOpts->mSuspensionSpring);
			jObject->SetNumberField(wheelOpts->GetName() + "_CompressionDamper", wheelOpts->mCompressionDamper);
			jObject->SetNumberField(wheelOpts->GetName() + "_RelaxationDamper", wheelOpts->mRelaxationDamper);
			jObject->SetNumberField(wheelOpts->GetName() + "_SuspensionPreload", wheelOpts->mSuspensionPreload);
		}
		// FGearWheel
		else if (comp->GetClass() == UFGearWheel::StaticClass())
		{
			UFGearWheel* w = Cast<UFGearWheel>(comp);
			jObject->SetBoolField(w->GetName() + "_IsActive", w->mIsActive);
			jObject->SetNumberField(w->GetName() + "_Side", (uint8)w->mSide);
			jObject->SetStringField(w->GetName() + "_BoneName", w->mBoneName.ToString());
		}
	}

	FString jsonStr;
	if (FJsonSerializer::Serialize(jObject.ToSharedRef(), TJsonWriterFactory<>::Create(&jsonStr)))
	{
		if (FFileHelper::SaveStringToFile(jsonStr, *path)) return true;
	}
	return false;
}

bool FGearSerializer::loadSettings(AFGearVehicle* v, FString path)
{
	FString jsonStr;
	if (!FFileHelper::LoadFileToString(jsonStr, *path)) return false;

	TSharedPtr<FJsonObject> jObject;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(jsonStr), jObject)) return false;

	// FGearVehicle|General
	v->mUpdateRate = FMath::Clamp(jObject->GetIntegerField(TEXT("UpdateRate")), 100, 1000);
	v->mMass = jObject->GetNumberField(TEXT("Mass"));
	v->mInertiaScale = fromArrayToVector(jObject->GetArrayField(TEXT("InertiaScale")));
	v->mCenterofMass = fromArrayToVector(jObject->GetArrayField(TEXT("CenterofMass")));
	v->mCustomGravity = jObject->GetBoolField(TEXT("CustomGravity"));
	v->mGravity = fromArrayToVector(jObject->GetArrayField(TEXT("Gravity")));
	v->mHandBrakePower = FMath::Clamp((float)jObject->GetNumberField(TEXT("HandBrakePower")), 1.0f, 100.0f);
	v->mWheelFrictionCoeff = jObject->GetNumberField(TEXT("WheelFrictionCoeff"));
	v->mStickyTireSpeed = FMath::Clamp((float)jObject->GetNumberField(TEXT("StickyTireSpeed")), 0.0f, 5.0f);
	v->mOptimumSlipWeight = FMath::Clamp((float)jObject->GetNumberField(TEXT("OptimumSlipWeight")), 0.0f, 1.0f);
	v->mReactionScale = FMath::Clamp((float)jObject->GetNumberField(TEXT("ReactionScale")), 0.0f, 1.0f);
	v->mDetectGround = jObject->GetBoolField(TEXT("DetectGround"));
	v->mHardContactScale = jObject->GetNumberField(TEXT("HardContactScale"));
	v->mRelaxationDownforce = jObject->GetBoolField(TEXT("RelaxationDownforce"));
	v->mTirePenetration = jObject->GetBoolField(TEXT("TirePenetration"));

	// FGearVehicle|Tracing
	v->mQueryType = (FGearQueryType)jObject->GetIntegerField(TEXT("QueryType"));
	v->mRays = FMath::Clamp(jObject->GetIntegerField(TEXT("Rays")), 1, 49);
	v->mLateralRays = FMath::Clamp(jObject->GetIntegerField(TEXT("LateralRays")), 1, 3);
	v->mQueryMobilityType = (FGearQueryMobilityType)jObject->GetIntegerField(TEXT("QueryMobilityType"));
	v->mTraceChannel = TEnumAsByte<ECollisionChannel>(jObject->GetIntegerField(TEXT("TraceChannel")));
	v->mIgnoreMask = jObject->GetIntegerField(TEXT("IgnoreMask"));
	v->mTraceComplex = jObject->GetBoolField(TEXT("TraceComplex"));
	v->mConvexAsset = loadObjFromPath<UStaticMesh>(jObject->GetStringField(TEXT("ConvexAsset")));
	v->mConvexUp = (FGearUpDirection)jObject->GetIntegerField(TEXT("ConvexUp"));
	v->mConvexRotation = fromArrayToRotator(jObject->GetArrayField(TEXT("ConvexRotation")));
	v->mConvexVisible = jObject->GetBoolField(TEXT("ConvexVisible"));

	// FGearVehicle|Telemetry
	FJsonObjectConverter::JsonObjectStringToUStruct<FGearTelemetryOptions>(jObject->GetStringField(TEXT("Telemetry")), &v->mTelemetry, 0, 0);

	// FGearVehicle|DriveAssists
	v->mABS = FMath::Clamp((float)jObject->GetNumberField(TEXT("ABS")), 0.0f, 100.0f);
	v->mASR = FMath::Clamp((float)jObject->GetNumberField(TEXT("ASR")), 0.0f, 100.0f);
	v->mUnderSteerAssist = FMath::Clamp((float)jObject->GetNumberField(TEXT("UnderSteerAssist")), 0.0f, 100.0f);
	v->mMinUnderSteerAngle = FMath::Clamp((float)jObject->GetNumberField(TEXT("MinUnderSteerAngle")), 1.0f, 90.0f);
	v->mOverSteerAssist = FMath::Clamp((float)jObject->GetNumberField(TEXT("OverSteerAssist")), 0.0f, 100.0f);
	v->mMinOverSteerAngle = FMath::Clamp((float)jObject->GetNumberField(TEXT("MinOverSteerAngle")), 1.0f, 90.0f);
	v->mFrontAntiRollPower = FMath::Clamp((float)jObject->GetNumberField(TEXT("FrontAntiRollPower")), 0.0f, 100.0f);
	v->mRearAntiRollPower = FMath::Clamp((float)jObject->GetNumberField(TEXT("RearAntiRollPower")), 0.0f, 100.0f);
	v->mMaxHillHolderSpeed = FMath::Clamp((float)jObject->GetNumberField(TEXT("MaxHillHolderSpeed")), 0.0f, 5.0f);

	// FGearVehicle|Optimization
	v->mSleepLinearThreshold = jObject->GetNumberField(TEXT("SleepLinearThreshold"));
	v->mSleepAngularThreshold = jObject->GetNumberField(TEXT("SleepAngularThreshold"));
	v->mSleepWaitingTime = jObject->GetNumberField(TEXT("SleepWaitingTime"));
	v->mWakeUpSources = FMath::Clamp(jObject->GetIntegerField(TEXT("WakeUpSources")), 0, 15);

	// FGearEngine
	v->mEngine->mTorqueCurve = loadObjFromPath<UCurveFloat>(jObject->GetStringField(TEXT("Engine_TorqueCurve")));
	v->mEngine->mTorqueScale = jObject->GetNumberField(TEXT("Engine_TorqueScale"));
	v->mEngine->mFrictionTorque = jObject->GetNumberField(TEXT("Engine_FrictionTorque"));
	v->mEngine->mIdleRpm = jObject->GetNumberField(TEXT("Engine_IdleRpm"));
	v->mEngine->mLimitRpm = jObject->GetNumberField(TEXT("Engine_LimitRpm"));
	v->mEngine->mLimiterTime = jObject->GetNumberField(TEXT("Engine_LimiterTime"));
	v->mEngine->mEngineInertia = jObject->GetNumberField(TEXT("Engine_EngineInertia"));

	// FGearTransmission
	v->mTransmission->mAutoChange = jObject->GetBoolField(TEXT("Transmission_AutoChange"));
	v->mTransmission->mAutoReverse = jObject->GetBoolField(TEXT("Transmission_AutoReverse"));
	v->mTransmission->mAutoClutch = jObject->GetBoolField(TEXT("Transmission_AutoClutch"));
	v->mTransmission->mRevMatching = jObject->GetBoolField(TEXT("Transmission_RevMatching"));
	v->mTransmission->mChangeTime = FMath::Clamp((float)jObject->GetNumberField(TEXT("Transmission_ChangeTime")), 1.0f, 1000.0f);
	v->mTransmission->mGearUpRatio = FMath::Clamp((float)jObject->GetNumberField(TEXT("Transmission_GearUpRatio")), 0.0f, 100.0f);
	v->mTransmission->mGearDownRatio = FMath::Clamp((float)jObject->GetNumberField(TEXT("Transmission_GearDownRatio")), 0.0f, 100.0f);
	v->mTransmission->mGearUpVelocityBias = FMath::Clamp((float)jObject->GetNumberField(TEXT("Transmission_GearUpVelocityBias")), 0.0f, 1.0f);
	v->mTransmission->mGearDownVelocityBias = FMath::Clamp((float)jObject->GetNumberField(TEXT("Transmission_GearDownVelocityBias")), 0.0f, 1.0f);
	v->mTransmission->mClutchTime = FMath::Clamp((float)jObject->GetNumberField(TEXT("Transmission_ClutchTime")), 1.0f, 1000.0f);
	v->mTransmission->mClutchScale = FMath::Clamp((float)jObject->GetNumberField(TEXT("Transmission_ClutchScale")), 1.0f, 1000.0f);
	v->mTransmission->mGearRatios = fromJsonArrayToFloatArray(jObject->GetArrayField(TEXT("Transmission_GearRatios")));
	v->mTransmission->mClutchAssist = jObject->GetBoolField(TEXT("Transmission_ClutchAssist"));
	v->mTransmission->mClutchScaleMin = FMath::Clamp((float)jObject->GetNumberField(TEXT("Transmission_ClutchScaleMin")), 0.0f, 100.0f);
	v->mTransmission->mClutchScaleMax = FMath::Clamp((float)jObject->GetNumberField(TEXT("Transmission_ClutchScaleMax")), 0.0f, 100.0f);

	// FGearStandardInput
	v->mStandardInput->mEnabled = jObject->GetBoolField(TEXT("StandardInput_Enabled"));
	v->mStandardInput->mInputType = (FGearInputSystemType)jObject->GetIntegerField(TEXT("StandardInput_InputType"));
	v->mStandardInput->mCombinedAxis = jObject->GetBoolField(TEXT("StandardInput_CombinedAxis"));
	v->mStandardInput->mControllerType = (FGearControllerType)jObject->GetIntegerField(TEXT("StandardInput_ControllerType"));
	v->mStandardInput->mShifterType = (FGearShifterType)jObject->GetIntegerField(TEXT("StandardInput_ShifterType"));
	v->mStandardInput->mVerticalAxis = FName(*jObject->GetStringField(TEXT("StandardInput_VerticalAxis")));
	v->mStandardInput->mHorizontalAxis = FName(*jObject->GetStringField(TEXT("StandardInput_HorizontalAxis")));
	v->mStandardInput->mThrottleAxis = FName(*jObject->GetStringField(TEXT("StandardInput_ThrottleAxis")));
	v->mStandardInput->mBrakeAxis = FName(*jObject->GetStringField(TEXT("StandardInput_BrakeAxis")));
	v->mStandardInput->mLeftAxis = FName(*jObject->GetStringField(TEXT("StandardInput_LeftAxis")));
	v->mStandardInput->mRightAxis = FName(*jObject->GetStringField(TEXT("StandardInput_RightAxis")));
	v->mStandardInput->mClutchAxis = FName(*jObject->GetStringField(TEXT("StandardInput_ClutchAxis")));
	v->mStandardInput->mHandbrakeAxis = FName(*jObject->GetStringField(TEXT("StandardInput_HandbrakeAxis")));
	v->mStandardInput->mShiftAxis = FName(*jObject->GetStringField(TEXT("StandardInput_ShiftAxis")));
	v->mStandardInput->mShift1Axis = FName(*jObject->GetStringField(TEXT("StandardInput_Shift1Axis")));
	v->mStandardInput->mShift2Axis = FName(*jObject->GetStringField(TEXT("StandardInput_Shift2Axis")));
	v->mStandardInput->mShift3Axis = FName(*jObject->GetStringField(TEXT("StandardInput_Shift3Axis")));
	v->mStandardInput->mShift4Axis = FName(*jObject->GetStringField(TEXT("StandardInput_Shift4Axis")));
	v->mStandardInput->mShift5Axis = FName(*jObject->GetStringField(TEXT("StandardInput_Shift5Axis")));
	v->mStandardInput->mShift6Axis = FName(*jObject->GetStringField(TEXT("StandardInput_Shift6Axis")));
	v->mStandardInput->mShiftRAxis = FName(*jObject->GetStringField(TEXT("StandardInput_ShiftRAxis")));
	v->mStandardInput->mSteerSensivity = FMath::Clamp((float)jObject->GetNumberField(TEXT("StandardInput_SteerSensivity")), 0.1f, 10.0f);
	v->mStandardInput->mSteerSpeedCurve = loadObjFromPath<UCurveFloat>(jObject->GetStringField(TEXT("StandardInput_SteerSpeedCurve")));
	v->mStandardInput->mSteerLimitCurve = loadObjFromPath<UCurveFloat>(jObject->GetStringField(TEXT("StandardInput_SteerLimitCurve")));
	v->mStandardInput->mSteerInputGravity = jObject->GetNumberField(TEXT("StandardInput_SteerInputGravity"));
	v->mStandardInput->mSteerRange = FMath::Clamp((float)jObject->GetNumberField(TEXT("StandardInput_SteerRange")), 10.0f, 100.0f);
	v->mStandardInput->mSteerDeadzone = FMath::Clamp((float)jObject->GetNumberField(TEXT("StandardInput_SteerDeadzone")), 0.0f, 0.9f);
	v->mStandardInput->mSteeringAssist = FMath::Clamp((float)jObject->GetNumberField(TEXT("StandardInput_SteeringAssist")), 0.0f, 100.0f);
	v->mStandardInput->mSteeringAssistThresholdAngle = FMath::Clamp((float)jObject->GetNumberField(TEXT("StandardInput_SteeringAssistThresholdAngle")), 0.0f, 90.0f);
	v->mStandardInput->mThrottleSensivity = FMath::Clamp((float)jObject->GetNumberField(TEXT("StandardInput_ThrottleSensivity")), 0.1f, 10.0f);
	v->mStandardInput->mThrottleRange = FMath::Clamp((float)jObject->GetNumberField(TEXT("StandardInput_ThrottleRange")), 10.0f, 100.0f);
	v->mStandardInput->mThrottleDeadzone = FMath::Clamp((float)jObject->GetNumberField(TEXT("StandardInput_ThrottleDeadzone")), 0.0f, 0.9f);
	v->mStandardInput->mThrottleEngagement = loadObjFromPath<UCurveFloat>(jObject->GetStringField(TEXT("StandardInput_ThrottleEngagement")));
	v->mStandardInput->mBrakingSensivity = FMath::Clamp((float)jObject->GetNumberField(TEXT("StandardInput_BrakingSensivity")), 0.1f, 10.0f);
	v->mStandardInput->mBrakingRange = FMath::Clamp((float)jObject->GetNumberField(TEXT("StandardInput_BrakingRange")), 10.0f, 100.0f);
	v->mStandardInput->mBrakingDeadzone = FMath::Clamp((float)jObject->GetNumberField(TEXT("StandardInput_BrakingDeadzone")), 0.0f, 0.9f);
	v->mStandardInput->mBrakeEngagement = loadObjFromPath<UCurveFloat>(jObject->GetStringField(TEXT("StandardInput_BrakeEngagement")));
	v->mStandardInput->mClutchSensivity = FMath::Clamp((float)jObject->GetNumberField(TEXT("StandardInput_ClutchSensivity")), 0.1f, 10.0f);
	v->mStandardInput->mClutchRange = FMath::Clamp((float)jObject->GetNumberField(TEXT("StandardInput_ClutchRange")), 10.0f, 100.0f);
	v->mStandardInput->mClutchDeadzone = FMath::Clamp((float)jObject->GetNumberField(TEXT("StandardInput_ClutchDeadzone")), 0.0f, 0.9f);
	v->mStandardInput->mClutchEngagement = loadObjFromPath<UCurveFloat>(jObject->GetStringField(TEXT("StandardInput_ClutchEngagement")));
#if ENHANCEDINPUT
	v->mStandardInput->mDefaultMappingContext = loadObjFromPath<UInputMappingContext>(jObject->GetStringField(TEXT("StandardInput_DefaultMappingContext")));
	v->mStandardInput->mVerticalAction = loadObjFromPath<UInputAction>(jObject->GetStringField(TEXT("StandardInput_VerticalAction")));
	v->mStandardInput->mHorizontalAction = loadObjFromPath<UInputAction>(jObject->GetStringField(TEXT("StandardInput_HorizontalAction")));
	v->mStandardInput->mThrottleAction = loadObjFromPath<UInputAction>(jObject->GetStringField(TEXT("StandardInput_ThrottleAction")));
	v->mStandardInput->mBrakeAction = loadObjFromPath<UInputAction>(jObject->GetStringField(TEXT("StandardInput_BrakeAction")));
	v->mStandardInput->mLeftAction = loadObjFromPath<UInputAction>(jObject->GetStringField(TEXT("StandardInput_LeftAction")));
	v->mStandardInput->mRightAction = loadObjFromPath<UInputAction>(jObject->GetStringField(TEXT("StandardInput_RightAction")));
	v->mStandardInput->mClutchAction = loadObjFromPath<UInputAction>(jObject->GetStringField(TEXT("StandardInput_ClutchAction")));
	v->mStandardInput->mHandbrakeAction = loadObjFromPath<UInputAction>(jObject->GetStringField(TEXT("StandardInput_HandbrakeAction")));
	v->mStandardInput->mShiftAction = loadObjFromPath<UInputAction>(jObject->GetStringField(TEXT("StandardInput_ShiftAction")));
	v->mStandardInput->mShift1Action = loadObjFromPath<UInputAction>(jObject->GetStringField(TEXT("StandardInput_Shift1Action")));
	v->mStandardInput->mShift2Action = loadObjFromPath<UInputAction>(jObject->GetStringField(TEXT("StandardInput_Shift2Action")));
	v->mStandardInput->mShift3Action = loadObjFromPath<UInputAction>(jObject->GetStringField(TEXT("StandardInput_Shift3Action")));
	v->mStandardInput->mShift4Action = loadObjFromPath<UInputAction>(jObject->GetStringField(TEXT("StandardInput_Shift4Action")));
	v->mStandardInput->mShift5Action = loadObjFromPath<UInputAction>(jObject->GetStringField(TEXT("StandardInput_Shift5Action")));
	v->mStandardInput->mShift6Action = loadObjFromPath<UInputAction>(jObject->GetStringField(TEXT("StandardInput_Shift6Action")));
	v->mStandardInput->mShiftRAction = loadObjFromPath<UInputAction>(jObject->GetStringField(TEXT("StandardInput_ShiftRAction")));
#endif

	// FGearAeroDynamics
	v->mAeroDynamics->mDragCoefficient = jObject->GetNumberField(TEXT("AeroDynamics_DragCoefficient"));
	v->mAeroDynamics->mDownForceCoefficient = jObject->GetNumberField(TEXT("AeroDynamics_DownForceCoefficient"));
	v->mAeroDynamics->mDownForceXOffset = FMath::Clamp((float)jObject->GetNumberField(TEXT("AeroDynamics_DownForceXOffset")), -10.0f, 10.0f);

	TArray<UActorComponent*> componentList;
	v->GetComponents(componentList);
	for (UActorComponent* comp : componentList)
	{
		if (comp == nullptr) continue;
		// FGearAxle
		if (comp->GetClass() == UFGearAxle::StaticClass())
		{
			UFGearAxle* axle = Cast<UFGearAxle>(comp);

			// check next field whether its an axle or not
			if (!jObject->HasField(axle->GetName() + "_DifferentialType")) continue;

			axle->mDifferentialType = (FGearDifferentialType)jObject->GetIntegerField(axle->GetName() + "_DifferentialType");
			axle->mDiffStrength = FMath::Clamp((float)jObject->GetNumberField(axle->GetName() + "_DiffStrength"), 0.0f, 1.0f);
			axle->mTorqueShare = FMath::Clamp((float)jObject->GetNumberField(axle->GetName() + "_TorqueShare"), 0.0f, 1.0f);
			axle->mFinalGearRatio = (float)jObject->GetNumberField(axle->GetName() + "_FinalGearRatio");
			axle->mMaxSteerAngle = FMath::Clamp((float)jObject->GetNumberField(axle->GetName() + "_MaxSteerAngle"), -90.0f, 90.0f);
			axle->mAckermanCoeff = FMath::Clamp((float)jObject->GetNumberField(axle->GetName() + "_AckermanCoeff"), -1.0f, 1.0f);
			axle->mAckermanReferenceIndex = jObject->GetIntegerField(axle->GetName() + "_AckermanReferenceIndex");
			axle->mHasHandbrake = jObject->GetBoolField(axle->GetName() + "_HasHandbrake");
			axle->mCamberAngle = FMath::Clamp((float)jObject->GetNumberField(axle->GetName() + "_CamberAngle"), -10.0f, 10.0f);
			axle->mToeAngle = FMath::Clamp((float)jObject->GetNumberField(axle->GetName() + "_ToeAngle"), -10.0f, 10.0f);
		}
		// FGearWheelOptions
		else if (comp->GetClass() == UFGearWheelOptions::StaticClass())
		{
			UFGearWheelOptions* wheelOpts = Cast<UFGearWheelOptions>(comp);
			wheelOpts->mTireModelClass = TSoftClassPtr<UFGearTire>(FSoftObjectPath(jObject->GetStringField(wheelOpts->GetName() + "_TireModelClass"))).Get();
			wheelOpts->mLateralFriction = jObject->GetNumberField(wheelOpts->GetName() + "_LateralFriction");
			wheelOpts->mLongitudinalFriction = jObject->GetNumberField(wheelOpts->GetName() + "_LongitudinalFriction");
			wheelOpts->mLateralScale = jObject->GetNumberField(wheelOpts->GetName() + "_LateralScale");
			wheelOpts->mLongitudinalScale = jObject->GetNumberField(wheelOpts->GetName() + "_LongitudinalScale");
			wheelOpts->mOverturnScale = jObject->GetNumberField(wheelOpts->GetName() + "_OverturnScale");
			wheelOpts->mRollMomentScale = jObject->GetNumberField(wheelOpts->GetName() + "_RollMomentScale");
			wheelOpts->mSelfAlignScale = jObject->GetNumberField(wheelOpts->GetName() + "_SelfAlignScale");
			wheelOpts->mTirePressure = jObject->GetNumberField(wheelOpts->GetName() + "_TirePressure");
			wheelOpts->mRadius = jObject->GetNumberField(wheelOpts->GetName() + "_Radius");
			wheelOpts->mWidth = jObject->GetNumberField(wheelOpts->GetName() + "_Width");
			wheelOpts->mSpacerOffset = jObject->GetNumberField(wheelOpts->GetName() + "_SpacerOffset");
			wheelOpts->mRimOffset = jObject->GetNumberField(wheelOpts->GetName() + "_RimOffset");
			wheelOpts->mCastOffset = jObject->GetNumberField(wheelOpts->GetName() + "_CastOffset");
			wheelOpts->mMass = jObject->GetNumberField(wheelOpts->GetName() + "_Mass");
			wheelOpts->mBrakeTorque = jObject->GetNumberField(wheelOpts->GetName() + "_BrakeTorque");
			wheelOpts->mSuspensionUpTravel = jObject->GetNumberField(wheelOpts->GetName() + "_SuspensionUpTravel");
			wheelOpts->mSuspensionDownTravel = jObject->GetNumberField(wheelOpts->GetName() + "_SuspensionDownTravel");
			wheelOpts->mSuspensionSpring = jObject->GetNumberField(wheelOpts->GetName() + "_SuspensionSpring");
			wheelOpts->mCompressionDamper = jObject->GetNumberField(wheelOpts->GetName() + "_CompressionDamper");
			wheelOpts->mRelaxationDamper = jObject->GetNumberField(wheelOpts->GetName() + "_RelaxationDamper");
			wheelOpts->mSuspensionPreload = FMath::Clamp((float)jObject->GetNumberField(wheelOpts->GetName() + "_SuspensionPreload"), 0.0f, 100.0f);
		}
		//FGearWheel
		else if (comp->GetClass() == UFGearWheel::StaticClass())
		{
			UFGearWheel* w = Cast<UFGearWheel>(comp);
			w->mIsActive = jObject->GetBoolField(w->GetName() + "_IsActive");
			w->mSide = (FGearWheelSide)jObject->GetIntegerField(w->GetName() + "_Side");
			w->mBoneName = FName(*jObject->GetStringField(w->GetName() + "_BoneName"));
		}
	}
	return true;
}

FString FGearSerializer::getObjPath(const UObject* obj)
{
	if (!obj) return FString("");
	FSoftObjectPath thePath(obj);
	if (!thePath.IsValid()) return FString("");
	return obj->GetClass()->GetDescription() + "'" + thePath.ToString() + "'";
}

TArray<TSharedPtr<FJsonValue>> FGearSerializer::fromVectorToArray(FVector v)
{
	TArray<TSharedPtr<FJsonValue>> arr;
	arr.Add(MakeShareable(new FJsonValueNumber(v.X)));
	arr.Add(MakeShareable(new FJsonValueNumber(v.Y)));
	arr.Add(MakeShareable(new FJsonValueNumber(v.Z)));
	return arr;
}

FVector FGearSerializer::fromArrayToVector(TArray<TSharedPtr<FJsonValue>> arr)
{
	return FVector((float)arr[0]->AsNumber(), (float)arr[1]->AsNumber(), (float)arr[2]->AsNumber());
}

TArray<TSharedPtr<FJsonValue>> FGearSerializer::fromRotatorToArray(FRotator r)
{
	TArray<TSharedPtr<FJsonValue>> arr;
	arr.Add(MakeShareable(new FJsonValueNumber(r.Pitch)));
	arr.Add(MakeShareable(new FJsonValueNumber(r.Yaw)));
	arr.Add(MakeShareable(new FJsonValueNumber(r.Roll)));
	return arr;
}

FRotator FGearSerializer::fromArrayToRotator(TArray<TSharedPtr<FJsonValue>> arr)
{
	return FRotator((float)arr[0]->AsNumber(), (float)arr[1]->AsNumber(), (float)arr[2]->AsNumber());
}

FString FGearSerializer::fromStructToString(const UStruct* StructDefinition, const void* Struct)
{
	FString str = FString("");
	FJsonObjectConverter::UStructToJsonObjectString(StructDefinition, Struct, str, 0, 0);
	return str;
}

TArray<TSharedPtr<FJsonValue>> FGearSerializer::fromTArrayToJsonArray(TArray<float> src)
{
	TArray<TSharedPtr<FJsonValue>> dest;
	for (int i = 0; i < src.Num(); i++)
	{
		dest.Add(MakeShareable(new FJsonValueNumber(src[i])));
	}
	return dest;
}

TArray<float> FGearSerializer::fromJsonArrayToFloatArray(TArray<TSharedPtr<FJsonValue>> src)
{
	TArray<float> dest;
	for (int i = 0; i < src.Num(); i++)
	{
		dest.Add(src[i]->AsNumber());
	}
	return dest;
}