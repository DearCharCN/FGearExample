// Copyright (C) Yunus Kara 2019-2024. All Rights Reserved.

#include "FGearRewindReplay.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "AudioDevice.h"

AFGearRewindReplay::AFGearRewindReplay() :
	mStateSize(100),
	mRewindPause(true),
	mReplayPause(true),
	mState(FGearRewindReplayState::RS_PLAY),
	mPlayIndex(0),
	mRewindIndex(0),
	mFirstTick(true)
{
 	PrimaryActorTick.bCanEverTick = true;
}

void AFGearRewindReplay::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (mFirstTick)
	{
		mFirstTick = false;
		setReplayFrames(mStateSize);
	}

	if (mState == FGearRewindReplayState::RS_PLAY)
	{
		//store state each update
		addState(DeltaTime);
	}
	else if (mState == FGearRewindReplayState::RS_REPLAY)
	{
		//replay state each update
		replayState(DeltaTime);
	}
	else if (mState == FGearRewindReplayState::RS_REWIND)
	{
		//rewind state each update
		rewindState(DeltaTime);
	}
}

void AFGearRewindReplay::setReplayFrames(int count)
{
	mStateSize = count;
	if (mVehicleList.Num() == 0) return;

	//fill with current transform
	for (int i = 0; i < mVehicleList.Num(); i++)
	{
		for (int j = 0; j < mStateSize; j++)
		{
			FFGearVehicleState state;
			mStateBuffer.Add(state);
			int index = i * mStateSize + j;
			mVehicleList[i]->saveState(mStateBuffer[index]);
		}
	}
}

void AFGearRewindReplay::addState(float dt)
{
	//store state
	for (int i = 0; i < mVehicleList.Num(); i++)
	{
		int index = i * mStateSize + mPlayIndex;
		mVehicleList[i]->saveState(mStateBuffer[index]);
	}

	//increment index
	mPlayIndex++;
	if (mPlayIndex >= mStateSize) mPlayIndex -= mStateSize;
	mRewindIndex = mPlayIndex;
}

void AFGearRewindReplay::replayState(float dt)
{
	//increment index
	mRewindIndex++;
	if (mRewindIndex >= mStateSize) mRewindIndex = 0;

	//check if done
	if (mRewindIndex == mPlayIndex)
	{
		//pause game?
		if (mReplayPause)
		{
			mRewindIndex--;
			UGameplayStatics::SetGamePaused(GetWorld(), true);
		}
		//back to play
		else
		{
			play();
			return;
		}
	}

	//load next vehicle state
	for (int i = 0; i < mVehicleList.Num(); i++)
	{
		int index = i * mStateSize + mRewindIndex;
		mVehicleList[i]->loadState(mStateBuffer[index]);
	}
}

void AFGearRewindReplay::rewindState(float dt)
{
	//decrement index
	mRewindIndex--;
	if (mRewindIndex < 0) mRewindIndex += mStateSize;

	//check if done
	if (mRewindIndex == mPlayIndex)
	{
		//pause game?
		if (mRewindPause)
		{
			mRewindIndex++;
			UGameplayStatics::SetGamePaused(GetWorld(), true);
		}
		//back to play
		else
		{
			play();
			return;
		}
	}

	//load next vehicle state
	for (int i = 0; i < mVehicleList.Num(); i++)
	{
		int index = i * mStateSize + mRewindIndex;
		mVehicleList[i]->loadState(mStateBuffer[index]);
	}
}

void AFGearRewindReplay::play()
{
	//change state
	if (mState == FGearRewindReplayState::RS_PLAY) return;
	mState = FGearRewindReplayState::RS_PLAY;
	mPlayIndex = mRewindIndex = 0;

	//reset vehicles states
	for (int i = 0; i < mVehicleList.Num(); i++)
	{
		for (int j = 0; j < mStateSize; j++)
		{
			int index = i * mStateSize + j;
			mStateBuffer[index].reset();
			mStateBuffer[index].mPosition = mVehicleList[i]->getPosition();
			mStateBuffer[index].mRotation = mVehicleList[i]->getRotation();
		}
	}

	//unpause game?
	if (mRewindPause || mReplayPause)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}

	//continue input updates
	for (int i = 0; i < mVehicleList.Num(); i++)
	{
		mVehicleList[i]->setUpdateInputs(true);
	}
}

void AFGearRewindReplay::pause()
{
	if (mState != FGearRewindReplayState::RS_PAUSE)
	{
		mState = FGearRewindReplayState::RS_PAUSE;

		//pause game
		UGameplayStatics::SetGamePaused(GetWorld(), true);

		//no input updates while paused
		for (int i = 0; i < mVehicleList.Num(); i++)
		{
			mVehicleList[i]->setUpdateInputs(false);
		}
	}
}

void AFGearRewindReplay::replay(bool resetIndex)
{
	//change state
	if (mState == FGearRewindReplayState::RS_REPLAY) return;
	mState = FGearRewindReplayState::RS_REPLAY;
	if (resetIndex) mRewindIndex = mPlayIndex;

	//unpause game?
	if (mRewindPause || mReplayPause)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}

	//no input updates while replaying
	for (int i = 0; i < mVehicleList.Num(); i++)
	{
		mVehicleList[i]->setUpdateInputs(false);
	}
}

void AFGearRewindReplay::rewind(bool resetIndex)
{
	//change state
	if (mState == FGearRewindReplayState::RS_REWIND) return;
	mState = FGearRewindReplayState::RS_REWIND;
	if (resetIndex) mRewindIndex = mPlayIndex;

	//unpause game?
	if (mRewindPause || mReplayPause)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}

	//no input updates while rewinding
	for (int i = 0; i < mVehicleList.Num(); i++)
	{
		mVehicleList[i]->setUpdateInputs(false);
	}
}

float AFGearRewindReplay::getProgressValue() const
{
	if (mStateSize == 0) return 0.0f;

	if (mState == FGearRewindReplayState::RS_PLAY)
	{
		return (float)mPlayIndex / mStateSize;
	}

	if (mState == FGearRewindReplayState::RS_REPLAY || mState == FGearRewindReplayState::RS_PAUSE)
	{
		int remain = mPlayIndex - mRewindIndex;
		if (remain < 0) remain = (mStateSize - mRewindIndex) + mPlayIndex;
		return 1.0f - (float)remain / mStateSize;
	}
	else if (mState == FGearRewindReplayState::RS_REWIND)
	{
		int remain = mRewindIndex - mPlayIndex;
		if (remain < 0) remain = (mStateSize - mPlayIndex) + mRewindIndex;
		return (float)remain / mStateSize;
	}

	return 0.0f;
}