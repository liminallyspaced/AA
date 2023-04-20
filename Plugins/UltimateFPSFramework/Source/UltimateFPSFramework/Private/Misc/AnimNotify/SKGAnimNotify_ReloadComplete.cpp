// Copyright 2022, Dakota Dawe, All rights reserved


#include "Misc/AnimNotify/SKGAnimNotify_ReloadComplete.h"

#include "Interfaces/SKGFirearmInterface.h"

USKGAnimNotify_ReloadComplete::USKGAnimNotify_ReloadComplete()
{
	
}

#if ENGINE_MAJOR_VERSION == 5
void USKGAnimNotify_ReloadComplete::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
#else
	void USKGAnimNotify_ReloadComplete::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
	{
		Super::Notify(MeshComp, Animation);
#endif
	if (IsValid(MeshComp) && MeshComp->GetOwner() && MeshComp->GetOwner()->GetClass()->ImplementsInterface(USKGFirearmInterface::StaticClass()))
	{
		ISKGFirearmInterface::Execute_OnReloadComplete(MeshComp->GetOwner());
	}
}
