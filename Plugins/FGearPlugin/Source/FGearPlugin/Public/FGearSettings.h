// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FGearSettings.generated.h"

//These settings are common to all vehicles or can be used to override certain groups of settings.
UCLASS(config = "Game",  defaultconfig, meta = (DisplayName = "FGear Common"))
class FGEARPLUGIN_API UFGearSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UFGearSettings();

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif

	/** If unchecked fgear replication will not have any effect. */
	UPROPERTY(EditAnywhere, config, Category = "FGearReplication", DisplayName = "Enabled")
	bool mEnabled;
	/** If checked the server will have authority over replication. */
	UPROPERTY(EditAnywhere, config, Category = "FGearReplication", DisplayName = "Server Authoritative")
	bool mServerAuthority;
	/** Network send rate for inputs. */
	UPROPERTY(EditAnywhere, config, Category = "FGearReplication", DisplayName = "Input Sendrate", meta = (ClampMin = "0", ClampMax = "100", UIMin = "0", UIMax = "100"))
	uint8 mInputSendRate;
	/** Network send rate for transforms. */
	UPROPERTY(EditAnywhere, config, Category = "FGearReplication", DisplayName = "State Sendrate", meta = (ClampMin = "0", ClampMax = "100", UIMin = "0", UIMax = "100"))
	uint8 mStateSendRate;
	/** Minumum contact speed for network <-> physics blending in KMH. This is only used when server authority is FALSE. */
	UPROPERTY(EditAnywhere, config, Category = "FGearReplication", DisplayName = "Min Collision Blend Speed", meta = (ClampMin = "0", ClampMax = "1000", UIMin = "0", UIMax = "1000"))
	float mMinCollisionBlendSpeed;
	/** When the network distance error is larger then this value(in meters) then we skip the local collision check and force a snap. */
	UPROPERTY(EditAnywhere, config, Category = "FGearReplication", DisplayName = "Force Snap Distance")
	float mForceSnapDistance;
};