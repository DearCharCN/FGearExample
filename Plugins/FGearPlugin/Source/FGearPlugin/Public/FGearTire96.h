// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGearTire.h"
#include "FGearTire96.generated.h"

UCLASS()
class FGEARPLUGIN_API UFGearTire96 : public UFGearTire
{
	GENERATED_BODY()
public:
	UFGearTire96();
	virtual void init() override;
	virtual void calculate(float load, float slipRatio, float slipAngle, float camber, float pressure, float radius, float Vx) override;

private:
	float pacejkaFx(float slipRatio, float Fz, float dFz);
	float pacejkaFy(float slipAngle, float camber, float Fz, float dFz, float& Dyk);
	float pacejkaGx(float slipRatio, float slipAngle, float dFz);
	float pacejkaGy(float slipRatio, float slipAngle, float dFz);
	float pacejkaSvyk(float slipRatio, float slipAngle, float camber, float dFz, float Dyk);

private:
	UPROPERTY(EditAnywhere, Category = "FGearTire|Nominal", DisplayName = "Nominal Load")
	float mNominalLoad;

	//pacejka 96 longitudinal
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PCX1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PDX1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PDX2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PEX1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PEX2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PEX3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PEX4;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PKX1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PKX2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PKX3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PHX1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PHX2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PVX1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PVX2;

	//pacejka 96 lateral
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PCY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PDY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PDY2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PDY3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PEY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PEY2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PEY3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PEY4;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PKY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PKY2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PKY3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PHY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PHY2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PHY3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PVY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PVY2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PVY3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PVY4;

	//pacejka 96 combined longitudinal
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLongitudinall")
	float RBX1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLongitudinall")
	float RBX2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLongitudinall")
	float RCX1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLongitudinall")
	float REX1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLongitudinall")
	float REX2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLongitudinall")
	float RHX1;

	//pacejka 96 combined lateral
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RBY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RBY2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RBY3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RCY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float REY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float REY2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RHY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RVY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RVY2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RVY3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RVY4;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RVY5;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RVY6;
};