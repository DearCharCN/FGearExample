// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGearTelemetryDrawer.h"
#include "FGearAeroDynamics.generated.h"

class AFGearVehicle;

UCLASS()
class FGEARPLUGIN_API UFGearAeroDynamics : public UFGearTelemetryDrawer
{
	GENERATED_BODY()
	friend class FGearSerializer;
public:
	UFGearAeroDynamics();
	void init(AFGearVehicle* v);
	void myFixedUpdate();

	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getDragCoefficient() const { return mDragCoefficient; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getDownForceCoefficient() const { return mDownForceCoefficient; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getDownForceXOffset() const { return mDownForceXOffset; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setDragCoefficient(float f) { mDragCoefficient = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setDownForceCoefficient(float f) { mDownForceCoefficient = f; }
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setDownForceXOffset(float f) { mDownForceXOffset = f; }

	virtual FVector2D drawTelemetry(UCanvas* Canvas, UFont* Font, FLinearColor textColor, FFontRenderInfo renderInfo, FVector2D p) override;

private:
	/** Aero drag is calculated using a unit area factor, vehicle speed and air density constant. This value is multiplied with the result of this calculation. The direction of the force is always the opposite of vehicles velocity. */
	UPROPERTY(EditAnywhere, Category = "FGearAeroDynamics", DisplayName = "Drag Coefficient")
	float mDragCoefficient;
	/** Just like drag force, the calculated down force is multiplied with this value. The direction of the down force is against the gravity. */
	UPROPERTY(EditAnywhere, Category = "FGearAeroDynamics", DisplayName = "Down Force Coefficient")
	float mDownForceCoefficient;
	/** If you want the down force to be applied more to the front of the vehicle give a positive value in meters and negative for the rear side */
	UPROPERTY(EditAnywhere, Category = "FGearAeroDynamics", DisplayName = "Down Force X Offset", meta = (ClampMin = "-10.0", ClampMax = "10.0", UIMin = "-10.0", UIMax = "10.0"))
	float mDownForceXOffset;

	FBodyInstance* mBody;
	AFGearVehicle* mVehicle;
	float mDragForce;
	float mDownForce;
	const float mAirDensity = 1.225f; //kg/m3

};