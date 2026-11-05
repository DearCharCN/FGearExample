// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGearTire.h"
#include "FGearTireSimple.generated.h"

UCLASS()
class FGEARPLUGIN_API UFGearTireSimple : public UFGearTire
{
	GENERATED_BODY()
public:
	UFGearTireSimple();
	virtual void init() override;
	virtual void calculate(float load, float slipRatio, float slipAngle, float camber, float pressure, float radius, float Vx) override;

private:
	//simplified pacejka longitudinal
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float Lnb;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float Lnc;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float Lnd;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float Lne;
	//simplified pacejka lateral
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float Ltb;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float Ltc;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float Ltd;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float Lte;
};