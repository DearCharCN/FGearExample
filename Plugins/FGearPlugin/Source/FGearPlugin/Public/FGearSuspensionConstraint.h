// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGearTelemetryDrawer.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "FGearSuspensionConstraint.generated.h"

class AFGearVehicle;

UCLASS(meta = (BlueprintSpawnableComponent), ClassGroup = (FGear))
class FGEARPLUGIN_API UFGearSuspensionConstraint : public UFGearTelemetryDrawer
{
	GENERATED_BODY()
public:	
	UFGearSuspensionConstraint();
	void init(AFGearVehicle* v);
	void substepTick();
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	void setEnabled(bool enable) { mIsEnabled = enable; }

private:
	UPROPERTY(EditAnywhere, Category = "FGearSuspensionConstraint", DisplayName = "Enabled")
	bool mIsEnabled;
	UPROPERTY(NoClear)
	TArray<UPhysicsConstraintComponent*> mJTS;
	AFGearVehicle* mVehicle;
};