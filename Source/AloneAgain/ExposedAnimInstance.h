// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ExposedAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class ALONEAGAIN_API UExposedAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void MyMontage_SetPosition(const UAnimMontage* Montage, float NewPosition);

};
