// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGearTelemetryDrawer.h"
#include "FGearWheelOptions.generated.h"

class UFGearTire;
class UFGearAxle;

UCLASS(meta = (BlueprintSpawnableComponent), ClassGroup = (FGear))
class FGEARPLUGIN_API UFGearWheelOptions : public UFGearTelemetryDrawer
{
	GENERATED_BODY()
	friend class UFGearAxle;
	friend class FGearSerializer;
public:
	UFGearWheelOptions();
	void reset();
	void checkNoTireState();
	void forceCreateTire();

	UFUNCTION(BlueprintCallable, Category = "FGear")
	UFGearTire* getTireModel() const { return mTireModel; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getLateralFriction() const { return mLateralFriction; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getLongitudinalFriction() const { return mLongitudinalFriction; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getLateralScale() const { return mLateralScale; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getLongitudinalScale() const { return mLongitudinalScale; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getOverturnScale() const { return mOverturnScale; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getRollMomentScale() const { return mRollMomentScale; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSelfAlignScale() const { return mSelfAlignScale; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getRadius() const { return mRadius; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getWidth() const { return mWidth; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getForwardOffset() const { return mForwardOffset; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSpacerOffset() const { return mSpacerOffset; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getRimOffset() const { return mRimOffset; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getCastOffset() const { return mCastOffset; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getMass() const { return mMass; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getTirePressure() const { return mTirePressure; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getBrakeTorque() const { return mBrakeTorque; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSuspensionUpTravel() const { return mSuspensionUpTravel; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSuspensionDownTravel() const { return mSuspensionDownTravel; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSuspensionSpring() const { return mSuspensionSpring; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getCompressionDamper() const { return mCompressionDamper; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getRelaxationDamper() const { return mRelaxationDamper; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getSuspensionPreload() const { return mSuspensionPreload; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setTireModel(UFGearTire* t);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setLateralFriction(float f) { mLateralFriction = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setLongitudinalFriction(float f) { mLongitudinalFriction = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setLateralScale(float f) { mLateralScale = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setLongitudinalScale(float f) { mLongitudinalScale = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setOverturnScale(float f) { mOverturnScale = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRollMomentScale(float f) { mRollMomentScale = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSelfAlignScale(float f) { mSelfAlignScale = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRadius(float f) { mRadius = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setWidth(float f) { mWidth = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setForwardOffset(float f) { mForwardOffset = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSpacerOffset(float f) { mSpacerOffset = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRimOffset(float f) { mRimOffset = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setCastOffset(float f) { mCastOffset = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setMass(float f) { mMass = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setTirePressure(float f) { mTirePressure = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setBrakeTorque(float f) { mBrakeTorque = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSuspensionUpTravel(float f) { mSuspensionUpTravel = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSuspensionDownTravel(float f) { mSuspensionDownTravel = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSuspensionSpring(float f) { mSuspensionSpring = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setCompressionDamper(float f) { mCompressionDamper = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setRelaxationDamper(float f) { mRelaxationDamper = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setSuspensionPreload(float f) { mSuspensionPreload = f; }

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	/** This is a reference to the TireModel object that is used for calculating wheel forces. This list will only include the tiremodels that are found in the scene and all tiremodel prefabs can be found in Resources/Tires folder. If you want to try another tiremodel just drag&drop it to the scene and its name will appear in this combobox. Warning : If different tire models are added or removed from the scene this value may change automatically because internally this is an index value of the tire list and when the tire list changes your tire model might be changed too. The best practive is to add all the tiremodels to the scene and do not remove any even if you do not use them. */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Tire", DisplayName = "Tire Model")
	TSubclassOf<UFGearTire> mTireModelClass;
	UPROPERTY()
	UFGearTire* mTireModel;
	/** The output of the tire models lateral force is multiplied with this value. Even if you use large values the final tire force will be limited. */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Tire", DisplayName = "Lateral Friction")
	float mLateralFriction;
	/** The output of the tire models longitudinal force is multiplied with this value. Even if you use large values the final tire force will be limited. */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Tire", DisplayName = "Longitudinal Friction")
	float mLongitudinalFriction;
	/** The final lateral tire force is multiplied with this value */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Tire", DisplayName = "(Fx)Lateral Force Scale")
	float mLateralScale;
	/** The final longitudinal tire force is multiplied with this value */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Tire", DisplayName = "(Fy)Longitudinal Force Scale")
	float mLongitudinalScale;
	/** The final X-Axis moment is multiplied with this */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Tire", DisplayName = "(Mx)Over-Turn Moment Scale")
	float mOverturnScale;
	/** The final Y-Axis moment is multiplied with this */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Tire", DisplayName = "(My)Wheel-Roll Moment Scale")
	float mRollMomentScale;
	/** The final Z-Axis moment is multiplied with this */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Tire", DisplayName = "(Mz)Self-Align Moment Scale")
	float mSelfAlignScale;
	/** Inflation pressure of tire in bars. This is only used by MF6.1 tire model. */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Tire", DisplayName = "Pressure")
	float mTirePressure;
	/** Radius of the wheel in meters. You can see the resulting circle in the editor mode and in the play mode if you enable gizmos */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Wheel", DisplayName = "Radius")
	float mRadius;
	/** Wheels do not have real width in this system but the width(in meters) parameter is integrated to effect one thing. If you set lateral ray count to 2 or 3 then the distance between the rays are calculated using the width value. The best practice is to use realistic values and you can preview the tire width with the gizmos drawn */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Wheel", DisplayName = "Width")
	float mWidth;
	/** A positive forward offset(in meters) will move the wheels towards +X direction in local space. */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Wheel", DisplayName = "Forward Offset")
	float mForwardOffset;
	/** A positive spacer offset(in meters) will move the wheels away from each other resulting a wider track. */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Wheel", DisplayName = "Spacer Offset")
	float mSpacerOffset;
	/** This is a lateral offset to the wheel rim. If your wheel position does not match with the physical wheel position you can adjust by using this value. You can activate gizmos from telemetry settings to see if your physical wheels match your wheel models. */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Wheel", DisplayName = "Rim Offset")
	float mRimOffset;
	/** This is an offset along the suspension normal. Use a positive value to offset the origin of ray/shape casts. If your wheels get stuck under ground when it falls from height then you can try using this. Do not use a large value, try to keep the cast origins inside vehicle hull. */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Wheel", DisplayName = "Cast Offset")
	float mCastOffset;
	/** Represents the mass of the wheel and used for calculating the rotational acceleration of the wheels. Lower mass wheels tend to spin more and higher mass wheels spin less. Too small values usually result jittery behaviour and too high values limit accelerations so try to experiment and find logical values */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Wheel", DisplayName = "Mass")
	float mMass;
	/** Brake torque applied when the braking input is %100. If you need more torque for handbrakes, there is a parameter for that in General Settings. */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Wheel", DisplayName = "Brake Torque")
	float mBrakeTorque;
	/** Total suspension spring length is divided into up&down travel values. This is the maximum distance in meters that your suspension will compress after the default pose. This will not effect the visible ride height */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Suspension", DisplayName = "Up Travel")
	float mSuspensionUpTravel;
	/** This is the maximum distance in meters that your suspension will extend after the default pose. This value is the one that will effect the visible ride height */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Suspension", DisplayName = "Down Travel")
	float mSuspensionDownTravel;
	/** Suspension spring force per mm, total force is SpringRate x TotalLength.  If your spring rate is not sufficient then you can not get the desired ride height so you are recommended to open telemetry gui and tweak accordingly */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Suspension", DisplayName = "Spring Rate (N/mm)")
	float mSuspensionSpring;
	/** Suspension compression damper force per 1 ms velocity. This value is used when suspension is compressing and it absorbs some of the spring force and reduces suspension oscillations */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Suspension", DisplayName = "Compression Damper (N/ms)")
	float mCompressionDamper;
	/** Suspension relaxation damper force per 1 ms velocity. This value is used when suspension is compressing and it absorbs some of the spring force and reduces suspension oscillations. Also see "Suspension Relaxation Downforce" */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Suspension", DisplayName = "Relaxation Damper (N/ms)")
	float mRelaxationDamper;
	/** This is the percentage of suspension length that is already compressed while it is fully extended */
	UPROPERTY(EditAnywhere, Category = "FGearWheelOptions|Suspension", DisplayName = "Preload(%)", meta = (ClampMin = "0", ClampMax = "100", UIMin = "0", UIMax = "100"))
	float mSuspensionPreload;

	UFGearAxle* mAxle;
};