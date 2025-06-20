﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "NAItemWidgetComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_API UNAItemWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNAItemWidgetComponent(const FObjectInitializer& ObjectInitializer);
	virtual void PostInitProperties() override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual void InitWidget() override;

	class UNAItemData* GetItemDataFromOwner() const;
	
	void ReleaseItemWidgetPopup();
	void CollapseItemWidgetPopup();

	class UNAItemWidget* GetItemWidget() const;

	void SetItemInteractionName(const FString& NewString) const;
	
protected:
	uint8 bFaceCamera : 1 = false;
	void FaceCamera();
};
