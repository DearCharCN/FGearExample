// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGearTire.h"
#include "FGearTireMF61.generated.h"

UCLASS()
class FGEARPLUGIN_API UFGearTireMF61 : public UFGearTire
{
	GENERATED_BODY()
public:
	UFGearTireMF61();
	virtual void init() override;
	virtual void calculate(float load, float slipRatio, float slipAngle, float camber, float pressure, float radius, float Vx) override;

private:
	float pneumaticTrail(float at, float slipAngle, float yi, float radius, float Fz, float dFz, float dPi, float signVcx);
	float residualMoment(float ar, float slipAngle, float yi, float radius, float Fz, float dFz, float By, float Cy, float dPi, float signVcx);

private:
	//nominals
	UPROPERTY(EditAnywhere, Category = "FGearTire|Nominal", DisplayName = "Nominal Load")
	float mNominalLoad;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Nominal", DisplayName = "Nominal Pressure")
	float mNominalPressure;

	//pacejka longitudinal
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PCX1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PDX1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PDX2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PDX3;
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
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PPX1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PPX2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PPX3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Longitudinal")
	float PPX4;

	//pacejka lateral
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
	float PEY5;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PKY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PKY2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PKY3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PKY4;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PKY5;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PKY6;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PKY7;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PHY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PHY2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PVY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PVY2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PVY3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PVY4;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PPY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PPY2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PPY3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PPY4;
	UPROPERTY(EditAnywhere, Category = "FGearTire|Lateral")
	float PPY5;

	//pacejka combined longitudinal
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLongitudinal")
	float RBX1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLongitudinal")
	float RBX2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLongitudinal")
	float RBX3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLongitudinal")
	float RCX1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLongitudinal")
	float REX1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLongitudinal")
	float REX2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLongitudinal")
	float RHX1;

	//pacejka combined lateral
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RBY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RBY2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RBY3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RBY4;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RCY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float REY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float REY2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RHY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedLateral")
	float RHY2;
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

	//pacejka overturning moment
	UPROPERTY(EditAnywhere, Category = "FGearTire|OverturningMoment")
	float QSX1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|OverturningMoment")
	float QSX2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|OverturningMoment")
	float QSX3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|OverturningMoment")
	float QSX4;
	UPROPERTY(EditAnywhere, Category = "FGearTire|OverturningMoment")
	float QSX5;
	UPROPERTY(EditAnywhere, Category = "FGearTire|OverturningMoment")
	float QSX6;
	UPROPERTY(EditAnywhere, Category = "FGearTire|OverturningMoment")
	float QSX7;
	UPROPERTY(EditAnywhere, Category = "FGearTire|OverturningMoment")
	float QSX8;
	UPROPERTY(EditAnywhere, Category = "FGearTire|OverturningMoment")
	float QSX9;
	UPROPERTY(EditAnywhere, Category = "FGearTire|OverturningMoment")
	float QSX10;
	UPROPERTY(EditAnywhere, Category = "FGearTire|OverturningMoment")
	float QSX11;
	UPROPERTY(EditAnywhere, Category = "FGearTire|OverturningMoment")
	float QSX12;
	UPROPERTY(EditAnywhere, Category = "FGearTire|OverturningMoment")
	float QSX13;
	UPROPERTY(EditAnywhere, Category = "FGearTire|OverturningMoment")
	float QSX14;
	UPROPERTY(EditAnywhere, Category = "FGearTire|OverturningMoment")
	float PPMX1;

	//pacejka rolling resistance moment
	UPROPERTY(EditAnywhere, Category = "FGearTire|RollingResistanceMoment")
	float QSY1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|RollingResistanceMoment")
	float QSY2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|RollingResistanceMoment")
	float QSY3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|RollingResistanceMoment")
	float QSY4;
	UPROPERTY(EditAnywhere, Category = "FGearTire|RollingResistanceMoment")
	float QSY5;
	UPROPERTY(EditAnywhere, Category = "FGearTire|RollingResistanceMoment")
	float QSY6;
	UPROPERTY(EditAnywhere, Category = "FGearTire|RollingResistanceMoment")
	float QSY7;
	UPROPERTY(EditAnywhere, Category = "FGearTire|RollingResistanceMoment")
	float QSY8;
	UPROPERTY(EditAnywhere, Category = "FGearTire|RollingResistanceMoment")
	float VREF;

	//pacejka aligning moment
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QBZ1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QBZ2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QBZ3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QBZ4;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QBZ5;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QBZ9;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QBZ10;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QCZ1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QDZ1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QDZ2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QDZ3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QDZ4;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QDZ6;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QDZ7;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QDZ8;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QDZ9;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QDZ10;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QDZ11;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QEZ1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QEZ2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QEZ3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QEZ4;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QEZ5;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QHZ1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QHZ2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QHZ3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float QHZ4;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float PPZ1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|AligningMoment")
	float PPZ2;

	//pacejka combined aligning moment
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedAligningMoment")
	float SSZ1;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedAligningMoment")
	float SSZ2;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedAligningMoment")
	float SSZ3;
	UPROPERTY(EditAnywhere, Category = "FGearTire|CombinedAligningMoment")
	float SSZ4;
};
