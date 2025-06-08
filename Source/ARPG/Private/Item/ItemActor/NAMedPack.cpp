﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemActor/NAMedPack.h"


// Sets default values
ANAMedPack::ANAMedPack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANAMedPack::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANAMedPack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

