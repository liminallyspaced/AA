// Copyright 2022, Dakota Dawe, All rights reserved


#include "Misc/AnimNotify/SKGAnimNotify_CameraShake.h"

USKGAnimNotify_CameraShake::USKGAnimNotify_CameraShake()
{
	ShakeScale = 1.0f;
}

#if ENGINE_MAJOR_VERSION == 5
void USKGAnimNotify_CameraShake::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
#else
void USKGAnimNotify_CameraShake::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
#endif
	if (MeshComp && CameraShake && ShakeScale > 0.0f)
	{
		if (const APawn* Pawn = MeshComp->GetOwner<APawn>())
		{
			if (APlayerController* PC = Pawn->GetController<APlayerController>())
			{
				if (PC->IsLocalController())
				{
					PC->ClientStartCameraShake(CameraShake, ShakeScale);
				}
			}
		}
	}
}