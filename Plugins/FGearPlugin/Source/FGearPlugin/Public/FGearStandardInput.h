// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once
#define ENHANCEDINPUT 1

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "Components/InputComponent.h"
#include "FGearTelemetryDrawer.h"
#include "FGearVehicleState.h"
#if ENHANCEDINPUT
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#endif
#include "FGearStandardInput.generated.h"

class AFGearVehicle;
class AController;

UENUM(BlueprintType)
enum class FGearInputSystemType : uint8
{
	LEGACY		UMETA(DisplayName = "Legacy"),
	ENHANCED	UMETA(DisplayName = "Enhanced")
};

UENUM()
enum class FGearControllerType : uint8
{
	KEYBOARD	UMETA(DisplayName = "Keyboard"),
	JOYSTICK 	UMETA(DisplayName = "Joystick"),
	WHEEL		UMETA(DisplayName = "Wheel")
};

UENUM()
enum class FGearShifterType : uint8
{
	AUTO 		UMETA(DisplayName = "Auto"),
	SEQUENTIAL 	UMETA(DisplayName = "Sequential"),
	MANUAL		UMETA(DisplayName = "Manuel")
};

namespace FGearEnhancedActionName
{
	const FName Vertical = FName(TEXT("Vertical"));
	const FName Horizontal = FName(TEXT("Horizontal"));
	const FName Throttle = FName(TEXT("Throttle"));
	const FName Brake = FName(TEXT("Brake"));
	const FName Left = FName(TEXT("Left"));
	const FName Right = FName(TEXT("Right"));
	const FName Clutch = FName(TEXT("Clutch"));
	const FName Handbrake = FName(TEXT("Handbrake"));
	const FName Shift = FName(TEXT("Shift"));
	const FName Shift1 = FName(TEXT("Shift1"));
	const FName Shift2 = FName(TEXT("Shift2"));
	const FName Shift3 = FName(TEXT("Shift3"));
	const FName Shift4 = FName(TEXT("Shift4"));
	const FName Shift5 = FName(TEXT("Shift5"));
	const FName Shift6 = FName(TEXT("Shift6"));
	const FName ShiftR = FName(TEXT("ShiftR"));
}

UCLASS()
class FGEARPLUGIN_API UFGearStandardInput : public UFGearTelemetryDrawer
{
	GENERATED_BODY()
	friend class FGearSerializer;
	friend class UFGearReplication;
public:
	UFGearStandardInput();
	void init(AFGearVehicle* v);
	void setupPlayerInputComponent(UInputComponent* PlayerInputComponent);
	void initInputMapping(const AController* c);
	void myUpdate(float dt, bool skipReading);

	void saveState(FFGearVehicleState& state);
	void loadState(FFGearVehicleState& state);

	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isEnabled() const { return mEnabled; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FGearInputSystemType getInputSystemType() const { return mInputType; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool getCombinedAxis() const { return mCombinedAxis; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FGearControllerType getControllerType() const { return mControllerType; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FGearShifterType getShifterType() const { return mShifterType; }

	//getters for axis names (legacy input)
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FName getVerticalAxis() const { return mVerticalAxis; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FName getHorizontalAxis() const { return mHorizontalAxis; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FName getThrottleAxis() const { return mThrottleAxis; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FName getBrakeAxis() const { return mBrakeAxis; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FName getLeftAxis() const { return mLeftAxis; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FName getRightAxis() const { return mRightAxis; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FName getClutchAxis() const { return mClutchAxis; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FName getHandbrakeAxis() const { return mHandbrakeAxis; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FName getShiftAxis() const { return mShiftAxis; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FName getShift1Axis() const { return mShift1Axis; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FName getShift2Axis() const { return mShift2Axis; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FName getShift3Axis() const { return mShift3Axis; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FName getShift4Axis() const { return mShift4Axis; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FName getShift5Axis() const { return mShift5Axis; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FName getShift6Axis() const { return mShift6Axis; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FName getShiftRAxis() const { return mShiftRAxis; }

	//getters for input actions (enhanced input)
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputMappingContext* getDefaultMappingContext() const { return mDefaultMappingContext; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputAction* getVerticalAction() const { return mVerticalAction; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputAction* getHorizontalAction() const { return mHorizontalAction; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputAction* getThrottleAction() const { return mThrottleAction; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputAction* getBrakeAction() const { return mBrakeAction; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputAction* getLeftAction() const { return mLeftAction; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputAction* getRightAction() const { return mRightAction; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputAction* getClutchAction() const { return mClutchAction; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputAction* getHandbrakeAction() const { return mHandbrakeAction; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputAction* getShiftAction() const { return mShiftAction; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputAction* getShift1Action() const { return mShift1Action; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputAction* getShift2Action() const { return mShift2Action; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputAction* getShift3Action() const { return mShift3Action; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputAction* getShift4Action() const { return mShift4Action; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputAction* getShift5Action() const { return mShift5Action; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputAction* getShift6Action() const { return mShift6Action; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UInputAction* getShiftRAction() const { return mShiftRAction; }

	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSteerSensivity() const { return mSteerSensivity; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSteerInputGravity() const { return mSteerInputGravity; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSteerRange() const { return mSteerRange; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSteerDeadzone() const { return mSteerDeadzone; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getThrottleSensivity() const { return mThrottleSensivity; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getThrottleRange() const { return mThrottleRange; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getThrottleDeadzone() const { return mThrottleDeadzone; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getBrakingSensivity() const { return mBrakingSensivity; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getBrakingRange() const { return mBrakingRange; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getBrakingDeadzone() const { return mBrakingDeadzone; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getClutchSensivity() const { return mClutchSensivity; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getClutchRange() const { return mClutchRange; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getClutchDeadzone() const { return mClutchDeadzone; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSteeringAssist() const { return mSteeringAssist; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSteeringAssistThresholdAngle() const { return mSteeringAssistThresholdAngle; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool isSteeringAssistActive() const { return mSteerAssistActive; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UCurveFloat* getSteerSpeedCurve() const { return mSteerSpeedCurve; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UCurveFloat* getSteerLimitCurve() const { return mSteerLimitCurve; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UCurveFloat* getThrottleEngagementCurve() const { return mThrottleEngagement; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UCurveFloat* getBrakeEngagementCurve() const { return mBrakeEngagement; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	UCurveFloat* getCluthEngagementCurve() const { return mClutchEngagement; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setEnabled(bool enable, bool resetInputs);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setCombinedAxis(bool b) { mCombinedAxis = b; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setReadInputs(bool b) { mReadInputs = b; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setStartGridMode(bool b);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setControllerType(FGearControllerType ct) { mControllerType = ct; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShifterType(FGearShifterType st) { mShifterType = st; }

	//setters for axis names (legacy input)
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setVerticalAxis(FName s) { mVerticalAxis = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setHorizontalAxis(FName s) { mHorizontalAxis = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setThrottleAxis(FName s) { mThrottleAxis = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setBrakeAxis(FName s) { mBrakeAxis = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setLeftAxis(FName s) { mLeftAxis = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRightAxis(FName s) { mRightAxis = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setClutchAxis(FName s) { mClutchAxis = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setHandbrakeAxis(FName s) { mHandbrakeAxis = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShiftAxis(FName s) { mShiftAxis = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShift1Axis(FName s) { mShift1Axis = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShift2Axis(FName s) { mShift2Axis = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShift3Axis(FName s) { mShift3Axis = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShift4Axis(FName s) { mShift4Axis = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShift5Axis(FName s) { mShift5Axis = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShift6Axis(FName s) { mShift6Axis = s; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShiftRAxis(FName s) { mShiftRAxis = s; }

	//setters for input actions (enhanced input)
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setDefaultMappingContext(UInputMappingContext* imc) {  mDefaultMappingContext = imc; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setVerticalAction(UInputAction* ia) { mVerticalAction = ia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setHorizontalAction(UInputAction* ia) { mHorizontalAction = ia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setThrottleAction(UInputAction* ia) { mThrottleAction = ia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setBrakeAction(UInputAction* ia) { mBrakeAction = ia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setLeftAction(UInputAction* ia) { mLeftAction = ia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRightAction(UInputAction* ia) { mRightAction = ia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setClutchAction(UInputAction* ia) { mClutchAction = ia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setHandbrakeAction(UInputAction* ia) { mHandbrakeAction = ia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShiftAction(UInputAction* ia) { mShiftAction = ia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShift1Action(UInputAction* ia) { mShift1Action = ia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShift2Action(UInputAction* ia) { mShift2Action = ia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShift3Action(UInputAction* ia) { mShift3Action = ia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShift4Action(UInputAction* ia) { mShift4Action = ia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShift5Action(UInputAction* ia) { mShift5Action = ia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShift6Action(UInputAction* ia) { mShift6Action = ia; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setShiftRAction(UInputAction* ia) { mShiftRAction = ia; }

	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSteerSensivity(float f) { mSteerSensivity = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSteerInputGravity(float f) { mSteerInputGravity = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSteerRange(float f) { mSteerRange = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSteerDeadzone(float f) { mSteerDeadzone = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setThrottleSensivity(float f) { mThrottleSensivity = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setThrottleRange(float f) { mThrottleRange = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setThrottleDeadzone(float f) { mThrottleDeadzone = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setBrakingSensivity(float f) { mBrakingSensivity = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setBrakingRange(float f) { mBrakingRange = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setBrakingDeadzone(float f) { mBrakingDeadzone = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setClutchSensivity(float f) { mClutchSensivity = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setClutchRange(float f) { mClutchRange = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setClutchDeadzone(float f) { mClutchDeadzone = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSteeringAssist(float f) { mSteeringAssist = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSteeringAssistThresholdAngle(float f) { mSteeringAssistThresholdAngle = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSteerSpeedCurve(UCurveFloat* c) { mSteerSpeedCurve = c; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSteerLimitCurve(UCurveFloat* c) { mSteerLimitCurve = c; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setThrottleEngagementCurve(UCurveFloat* c) { mThrottleEngagement = c; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setBrakeEngagementCurve(UCurveFloat* c) { mBrakeEngagement = c; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setCluthEngagementCurve(UCurveFloat* c) { mClutchEngagement = c; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setInputs(float throttle, float brake, float steer, float clutch, int gear, bool hb);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void overrideFinalInputs(float throttle, float brake, float steer, float clutch, int gear, bool hb);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void resetInputs();

	//getters for final inputs
	float getThrottleFinalInput() const { return mThrottleInput; }
	float getBrakeFinalInput() const { return mBrakeInput; }
	float getSteerFinalInput() const { return mSteerInput; }
	float getClutchFinalInput() const { return mClutchInput; }
	
	//getters for raw inputs, used for replication
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getThrottleRawInput() const { return mForward; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getBrakeRawInput() const { return mBackward; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSteerRawInput() const { return mHorizontal; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getClutchRawInput() const { return mClutch; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	int getGearboxInput() const { return mGearboxInput; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	bool getHandbrakeInput() const { return mHandBrakeInput; }
	bool getReadInputs() const { return mReadInputs; }

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif
	virtual FVector2D drawTelemetry(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p) override;

private:
	float getAxis(FName name) const;
	void getInputs(float dt);
	float getSteerSpeed() const;
	float getSteerLimit() const;
	void updateAxisInputs(float dt);
	void modifyInputs(float dt);
	void filterInputs();
	void applyInputs();

private:
	/** If unchecked user inputs will not have any effect. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Enabled")
	bool mEnabled;
	/** Use legacy or enhanced input. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Input Type", meta = (EditCondition = "mEnabled"))
	FGearInputSystemType mInputType;
	/** This applies only to throttle, brake and steering controls. If you want to use a combined axis like Vertical Axis for throttle/brake controls and Horizontal Axis for steering then enable this. If you want to use seperate axis for those then uncheck this option and setup proper axis values. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Use Combined Axis", meta = (EditCondition = "mEnabled"))
	bool mCombinedAxis;
	/**  Type of the controller : Keyboard, joystick or wheel. Most of the controls can work at the same time, for ex. even if you select keyboard you can play with joystick too. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Controller Type", meta = (EditCondition = "mEnabled"))
	FGearControllerType mControllerType;
	/** Type of the shifter : Auto, sequential or manual. If the AutoChange option of the transmission is checked this selection has no effect but if you want to manually change gears then you can choose a sequential mode or full manual mode. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Shifter Type", meta = (EditCondition = "mEnabled"))
	FGearShifterType mShifterType;
	/** This is a combined axis for throttle and brakes and only available if you check UseCombinedAxis option. Range : [-1, 1]. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Vertical Axis", meta = (EditCondition = "mEnabled"))
	FName mVerticalAxis;
	/** This is the steering axis name and this is only available if you check UseCombinedAxis option. Range : [-1, 1]. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Horizontal Axis", meta = (EditCondition = "mEnabled"))
	FName mHorizontalAxis;
	/** If you uncheck UseCombinedAxis option you can assign an axis for throttle with this. Range : [0, 1]. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Throttle Axis", meta = (EditCondition = "mEnabled"))
	FName mThrottleAxis;
	/** If you uncheck UseCombinedAxis option you can assign an axis for braking with this. Range : [0, 1]. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Brake Axis", meta = (EditCondition = "mEnabled"))
	FName mBrakeAxis;
	/** If you uncheck UseCombinedAxis option you can assign an axis for left steering with this. Range : [0, 1]. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Left Axis", meta = (EditCondition = "mEnabled"))
	FName mLeftAxis;
	/** If you uncheck UseCombinedAxis option you can assign an axis for right steering with this. Range : [0, 1]. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Right Axis", meta = (EditCondition = "mEnabled"))
	FName mRightAxis;
	/** Clutch axis for manual transmission. Range : [-1, 1] but internally normalized to [0,1]. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Clutch Axis", meta = (EditCondition = "mEnabled"))
	FName mClutchAxis;
	/** Handbrake input axis. Range : [0, 1]. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Handbrake Axis", meta = (EditCondition = "mEnabled"))
	FName mHandbrakeAxis;
	/** This is a combined axis for gear up and gear down. Range : [-1, 1]. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Shift Axis", meta = (EditCondition = "mEnabled"))
	FName mShiftAxis;
	/** Axis for first gear. Range : [0, 1] */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Shift 1 Axis", meta = (EditCondition = "mEnabled"))
	FName mShift1Axis;
	/** Axis for second gear. Range : [0, 1] */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Shift 2 Axis", meta = (EditCondition = "mEnabled"))
	FName mShift2Axis;
	/** Axis for third gear. Range : [0, 1] */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Shift 3 Axis", meta = (EditCondition = "mEnabled"))
	FName mShift3Axis;
	/** Axis for fourth gear. Range : [0, 1] */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Shift 4 Axis", meta = (EditCondition = "mEnabled"))
	FName mShift4Axis;
	/** Axis for fifth gear. Range : [0, 1] */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Shift 5 Axis", meta = (EditCondition = "mEnabled"))
	FName mShift5Axis;
	/** Axis for sixth gear. Range : [0, 1] */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Shift 6 Axis", meta = (EditCondition = "mEnabled"))
	FName mShift6Axis;
	/** Axis for reverse. Range : [0, 1] */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Shift R Axis", meta = (EditCondition = "mEnabled"))
	FName mShiftRAxis;

	/** If you want the steering speed to decrease as the vehicle goes faster you can use this curve. The x-axis represents speed in kmh and the y-axis is the steering speed */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Steer Sensivity", meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.1", UIMax = "10.0", EditCondition = "mEnabled"))
	float mSteerSensivity;
	/**  If you want the steering speed to decrease as the vehicle goes faster you can use this curve. The x-axis represents speed in kmh and the y-axis is the steering speed */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Steer Speed Curve", meta = (EditCondition = "mEnabled"))
	UCurveFloat* mSteerSpeedCurve;
	/** If you want the maximum steering angle to be limited as the vehicle goes faster you can use this curve. The x-axis represents speed in kmh and the y-axis should be a value between 0 and 100. 100 means steering is not limited and 0 means no steering possible at all. The limit angle is calculated as (value/100) * MaxSteerAngle. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Steer Limit Curve", meta = (EditCondition = "mEnabled"))
	UCurveFloat* mSteerLimitCurve;
	/** If the current steering angle and the desired steering angle have different signs than current steering speed is multiplied with this value. This lets user to counter steer more easily with the keyboard or joystick. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Steer Input Gravity", meta = (EditCondition = "mEnabled"))
	float mSteerInputGravity;
	/** The percentage of steering range. For ex. if you set this to %10 then a %10 input will result %100 steering. Another example is that a steering wheel with 900 degress of working range needs 450 degrees of rotation to make a full turn but if this value is %10 then a 45 degrees of rotation will make a full turn. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Steer Range", meta = (ClampMin = "10.0", ClampMax = "100.0", UIMin = "10.0", UIMax = "100.0", EditCondition = "mEnabled"))
	float mSteerRange;
	/** Input values below this value are discarded. A small dead zone is mostly usefull. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Steer Deadzone", meta = (ClampMin = "0.0", ClampMax = "0.9", UIMin = "0.0", UIMax = "0.9", EditCondition = "mEnabled"))
	float mSteerDeadzone;

	/** Steering assist tries to steer towards the direction that the vehicle body travels. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Steering Assist", meta = (ClampMin = "0.0", ClampMax = "100.0", UIMin = "0.0", UIMax = "100.0", EditCondition = "mEnabled"))
	float mSteeringAssist;
	/** If the angle between the steering direction and the vehicles velocity direction is above this value, steering assist is activated. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Steering Assist Threshold Angle", meta = (ClampMin = "0.0", ClampMax = "90.0", UIMin = "0.0", UIMax = "90.0", EditCondition = "mEnabled"))
	float mSteeringAssistThresholdAngle;

	/** This determines how fast the throttle value catches up with the input axis value. Higher values result sharper throttle */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Throttle Sensivity", meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.1", UIMax = "10.0", EditCondition = "mEnabled"))
	float mThrottleSensivity;
	/** The percentage of throttle range. Works similar to steering range. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Throttle Range", meta = (ClampMin = "10.0", ClampMax = "100.0", UIMin = "10.0", UIMax = "100.0", EditCondition = "mEnabled"))
	float mThrottleRange;
	/** Input values below this value are discarded. A small dead zone is mostly usefull */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Throttle Deadzone", meta = (ClampMin = "0.0", ClampMax = "0.9", UIMin = "0.0", UIMax = "0.9", EditCondition = "mEnabled"))
	float mThrottleDeadzone;
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Throttle Engagement", meta = (EditCondition = "mEnabled"))
	UCurveFloat* mThrottleEngagement;

	/** This determines how fast the braking value catches up with the input axis value. Higher values result sharper braking */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Braking Sensivity", meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.1", UIMax = "10.0", EditCondition = "mEnabled"))
	float mBrakingSensivity;
	/** The percentage of braking range. Works similar to steering range */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Braking Range", meta = (ClampMin = "10.0", ClampMax = "100.0", UIMin = "10.0", UIMax = "100.0", EditCondition = "mEnabled"))
	float mBrakingRange;
	/** Input values below this value are discarded. A small dead zone is mostly usefull */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Braking Deadzone", meta = (ClampMin = "0.0", ClampMax = "0.9", UIMin = "0.0", UIMax = "0.9", EditCondition = "mEnabled"))
	float mBrakingDeadzone;
	/** Brake input takes values between 0 and 1. If brake is fully pressed it is 1 and if it is not pressed at all then it is 0. This value changes linearly as you press the pedal. The brake input represents the X-Axis of this curve and the resulting value represents the real brake power ratio. Brake pedal behaviour changes from vehicle to vehicle so you can customise it with this curve. The default curve is a linear pedal. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Brake Engagement", meta = (EditCondition = "mEnabled"))
	UCurveFloat* mBrakeEngagement;

	/** This determines how fast the clutch value catches up with the input axis value. Higher values result sharper clutch engagement */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Clutch Sensivity", meta = (ClampMin = "0.1", ClampMax = "10.0", UIMin = "0.1", UIMax = "10.0", EditCondition = "mEnabled"))
	float mClutchSensivity;
	/** The percentage of clutch range. Works similar to steering range */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Clutch Range", meta = (ClampMin = "10.0", ClampMax = "100.0", UIMin = "10.0", UIMax = "100.0", EditCondition = "mEnabled"))
	float mClutchRange;
	/** Input values below this value are discarded. A small dead zone is mostly usefull. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Clutch Deadzone", meta = (ClampMin = "0.0", ClampMax = "0.9", UIMin = "0.0", UIMax = "0.9", EditCondition = "mEnabled"))
	float mClutchDeadzone;
	/** There is an internal variable called ClutchState which takes values between 0 and 1. If clutch is full pressed it is 1 and if it is not pressed at all then it is 0. This value changes linearly as you press the pedal. The ClutchState represents the X-Axis of this curve and the resulting value represent the real clutch power ratio. Clutch pedal behaviour changes from vehicle to vehicle but generally try to leave a dead zone before engaging and then quickly raise the power ratio. The default curve is a simple example to this */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput", DisplayName = "Cluth Engagement", meta = (EditCondition = "mEnabled"))
	UCurveFloat* mClutchEngagement;

	/** Input mapping context for the enhanced input system. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "Default Mapping Context", meta = (EditCondition = "mEnabled"))
	class UInputMappingContext* mDefaultMappingContext;
	/** Input action that corresponds to mVerticalAxis. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "Vertical Action", meta = (EditCondition = "mEnabled"))
	UInputAction* mVerticalAction;
	/** Input action that corresponds to mHorizontalAxis. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "Horizontal Action", meta = (EditCondition = "mEnabled"))
	UInputAction* mHorizontalAction;
	/** Input action that corresponds to mThrottleAxis. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "Throttle Action", meta = (EditCondition = "mEnabled"))
	UInputAction* mThrottleAction;
	/** Input action that corresponds to mBrakeAxis. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "Brake Action", meta = (EditCondition = "mEnabled"))
	UInputAction* mBrakeAction;
	/** Input action that corresponds to mLeftAxis. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "Left Action", meta = (EditCondition = "mEnabled"))
	UInputAction* mLeftAction;
	/** Input action that corresponds to mRightAxis. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "Right Action", meta = (EditCondition = "mEnabled"))
	UInputAction* mRightAction;
	/** Input action that corresponds to mClutchAxis. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "Clutch Action", meta = (EditCondition = "mEnabled"))
	UInputAction* mClutchAction;
	/** Input action that corresponds to mHandbrakeAxis. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "Handbrake Action", meta = (EditCondition = "mEnabled"))
	UInputAction* mHandbrakeAction;
	/** Input action that corresponds to mShiftAxis. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "Shift Action", meta = (EditCondition = "mEnabled"))
	UInputAction* mShiftAction;
	/** Input action that corresponds to mShift1Axis. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "Shift1 Action", meta = (EditCondition = "mEnabled"))
	UInputAction* mShift1Action;
	/** Input action that corresponds to mShift2Axis. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "Shift2 Action", meta = (EditCondition = "mEnabled"))
	UInputAction* mShift2Action;
	/** Input action that corresponds to mShift3Axis. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "Shift3 Action", meta = (EditCondition = "mEnabled"))
	UInputAction* mShift3Action;
	/** Input action that corresponds to mShift4Axis. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "Shift4 Action", meta = (EditCondition = "mEnabled"))
	UInputAction* mShift4Action;
	/** Input action that corresponds to mShift5Axis. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "Shift5 Action", meta = (EditCondition = "mEnabled"))
	UInputAction* mShift5Action;
	/** Input action that corresponds to mShift6Axis. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "Shift6 Action", meta = (EditCondition = "mEnabled"))
	UInputAction* mShift6Action;
	/** Input action that corresponds to mShiftRAxis. */
	UPROPERTY(EditAnywhere, Category = "FGearStandardInput|EnhancedInput", DisplayName = "ShiftR Action", meta = (EditCondition = "mEnabled"))
	UInputAction* mShiftRAction;

	AFGearVehicle* mVehicle;
	UPROPERTY(NoClear)
	UInputComponent* mInputComp;
	TMap<FName, int> mInputActionBindings;

	//final inputs
	float mThrottleInput;
	float mBrakeInput;
	float mSteerInput;
	float mClutchInput;
	//raw inputs
	float mForward;
	float mBackward;
	float mHorizontal;
	float mClutch;
	//resulting inputs
	int mThrottleState;
	float mThrottleValue;  //[-1, 1]
	float mBrakeValue;     //[ 0, 1]
	float mClutchValue;
	bool mHandBrakeInput;
	int mGearboxInput;
	int mLastGearboxInput;
	float mTargetSteeringInput;
	float mCurrentSteeringInput;
	float mCurrentSteeringLimit;
	bool mInputMappingInitialized;

	bool mReadInputs; //if set to false, inputs should be provided manually(see setInputs method)
	bool mSteerAssistActive;
	bool mStartGridMode;
};