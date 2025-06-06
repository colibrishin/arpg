﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemActor/NAWeapon.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ANAWeapon::ANAWeapon() : ANAPickableItemActor(FObjectInitializer::Get())
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CombatComponent = CreateDefaultSubobject<UNAMontageCombatComponent>( TEXT("CombatComponent") );
	// 무기가 캐릭터의 Child Actor로 부착될 것이기에 CombatComponent의 설정이 Parent Actor로 가도록
	CombatComponent->SetConsiderChildActor( true );

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>( TEXT("AbilitySystemComponent") );

	PickupMode = EPickupMode::PM_Holdable | EPickupMode::PM_Inventory;
}

// Called when the game starts or when spawned
void ANAWeapon::BeginPlay()
{
	Super::BeginPlay();

	// 몽타주랑 공격이 설정되어 있는지 확인
	check( CombatComponent->GetMontage() && CombatComponent->GetAttackAbility() );
	CombatComponent->SetActive( true );

	if ( !HasAuthority() )
	{
		if ( AActor* Actor = GetAttachParentActor() )
		{
			AbilitySystemComponent->InitAbilityActorInfo( Actor, this );

			// 클라이언트에 Child Actor 리플리케이션이 발생한 경우에 대한 대응
			// 만약 해당 무기 액터의 소유권자가 클라이언트 자신이라면 공격 Ability 부여 요청을 재시도
			if ( const APawn* Pawn = Cast<APawn>( Actor );
				 Pawn->GetController() == GetWorld()->GetFirstPlayerController() )
			{
				CombatComponent->Server_RequestAttackAbility();
			}
		}	
	}
}

void ANAWeapon::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	DOREPLIFETIME( ANAWeapon, CombatComponent );
	DOREPLIFETIME( ANAWeapon, AbilitySystemComponent );
}

// Called every frame
void ANAWeapon::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

