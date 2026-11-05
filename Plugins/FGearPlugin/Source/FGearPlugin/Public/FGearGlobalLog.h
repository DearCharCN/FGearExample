// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#pragma once

#define GET_CLASSNAME_WITH_FUNCTION	FString(__FUNCTION__)
#define GET_LINE_NUMBER				FString::FromInt(__LINE__)

#define LOG(LogCategory, Message)	UE_LOG(LogCategory, Log, TEXT("%s [%s] %s"), *GET_CLASSNAME_WITH_FUNCTION, *GET_LINE_NUMBER, *FString(Message))

DEFINE_LOG_CATEGORY_STATIC(FGear, All, All)