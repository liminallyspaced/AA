// Copyright 2022, Dakota Dawe, All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SKGAnimNotify_ReloadComplete.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMATEFPSFRAMEWORK_API USKGAnimNotify_ReloadComplete : public UAnimNotify
{
	GENERATED_BODY()
public:
	USKGAnimNotify_ReloadComplete();

protected:
#if ENGINE_MAJOR_VERSION == 5
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
#else
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
#endif
};
