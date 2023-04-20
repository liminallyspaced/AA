// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#include "GenPlayerController.h"

GAME_STATE_CLASS::GAME_STATE_CLASS()
{
  ServerWorldTimeSecondsUpdateFrequency = 0.1f;
}

float GAME_STATE_CLASS::GetLastReplicatedServerWorldTimeSeconds() const
{
  return ReplicatedWorldTimeSeconds;
}

void GAME_STATE_CLASS::OnRep_ReplicatedWorldTimeSeconds()
{
  Super::OnRep_ReplicatedWorldTimeSeconds();

  if (UGameInstance* GameInstance = GetGameInstance())
  {
    AGenPlayerController* LocalController = Cast<AGenPlayerController>(GameInstance->GetFirstLocalPlayerController());
    if (LocalController)
    {
      LocalController->Client_SyncWithServerTime();
    }
  }
}

void GAME_STATE_CLASS::UpdateServerTimeSeconds()
{
  if (UWorld* World = GetWorld())
  {
    ReplicatedWorldTimeSeconds = World->GetTimeSeconds();

    // @attention We need to replicate the server time as quickly as possible to the client to keep the discrepancy not caused by network
    // latency minimal.
    ForceNetUpdate();
  }
}
