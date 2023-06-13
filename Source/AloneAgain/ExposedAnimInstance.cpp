// Fill out your copyright notice in the Description page of Project Settings.


#include "ExposedAnimInstance.h"

void UExposedAnimInstance::MyMontage_SetPosition(const UAnimMontage* Montage, float NewPosition) 
{
	Super::Montage_SetPosition(Montage, NewPosition);
}
