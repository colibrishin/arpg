// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NAAmmoWidget.generated.h"

class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class ARPG_API UNAAmmoWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta=(BindWidget, AllowPrivateAccess="true"))
	UImage* Background;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ammo", meta=(BindWidget, AllowPrivateAccess="true"))
	UTextBlock* AmmoCount;
};
