// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearSettings.h"

UFGearSettings::UFGearSettings() :
	mEnabled(true),
	mServerAuthority(false),
	mInputSendRate(20),
	mStateSendRate(20),
	mMinCollisionBlendSpeed(10.0f),
	mForceSnapDistance(15.0f)
{
}

#if WITH_EDITOR
bool UFGearSettings::CanEditChange(const FProperty* Property) const
{
	const bool parentVal = Super::CanEditChange(Property);

	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearSettings, mServerAuthority)) return mEnabled;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearSettings, mInputSendRate)) return mEnabled;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearSettings, mStateSendRate)) return mEnabled;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearSettings, mMinCollisionBlendSpeed)) return mEnabled && !mServerAuthority;
	if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UFGearSettings, mForceSnapDistance)) return mEnabled && !mServerAuthority;
	
	return parentVal;
}
#endif