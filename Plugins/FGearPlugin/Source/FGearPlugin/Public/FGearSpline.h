// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FGearSpline.generated.h"

class USplineComponent;

UCLASS()
class FGEARPLUGIN_API AFGearSpline : public AActor
{
	GENERATED_BODY()
public:	
	AFGearSpline();
	UFUNCTION(BlueprintCallable, Category = "FGear")
	float getClosestParam(FVector p) const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector moveParam(float &p, float distance);
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getPoint(float p) const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getTangent(float p) const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	FVector getDirection(float p) const;
	UFUNCTION(BlueprintCallable, Category = "FGear")
	USplineComponent* getSpline() const { return mSplineComp; }

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "FGearSpline", DisplayName = "Spline")
	USplineComponent* mSplineComp;

	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "FGearSpline", DisplayName = "Debug")
	bool mDebug;
	UPROPERTY(EditAnywhere, Category = "FGearSpline", DisplayName = "Debug Color", meta = (EditCondition = "mDebug"))
	FColor mDebugColor;
	UPROPERTY(EditAnywhere, Category = "FGearSpline", DisplayName = "Debug Thickness", meta = (EditCondition = "mDebug"))
	float mDebugThickness;
	UPROPERTY(EditAnywhere, Category = "FGearSpline", DisplayName = "Debug Iterator", meta = (EditCondition = "mDebug"))
	float mDebugIterator;
};