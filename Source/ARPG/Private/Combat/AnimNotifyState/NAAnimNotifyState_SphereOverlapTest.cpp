// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/AnimNotifyState/NAAnimNotifyState_SphereOverlapTest.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Combat/ActorComponent/NAMontageCombatComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HP/GameplayEffect/NAGE_Damage.h"

#include "Perception/AISenseConfig_Damage.h"

void UNAAnimNotifyState_SphereOverlapTest::NotifyBegin( USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                        float TotalDuration, const FAnimNotifyEventReference& EventReference )
{
	Super::NotifyBegin( MeshComp, Animation, TotalDuration, EventReference );

	if ( MeshComp->GetWorld()->IsGameWorld() )
	{
		if (  MeshComp->GetOwner()->HasAuthority() )
		{
			const TScriptInterface<IAbilitySystemInterface>& SourceInterface = MeshComp->GetOwner();

			if ( !SourceInterface )
			{
				// GAS가 없는 객체로부터 시도됨
				check( false );
				return;
			}

			const FVector SocketLocation = MeshComp->GetSocketLocation( SocketName );
	
			ContextHandle = SourceInterface->GetAbilitySystemComponent()->MakeEffectContext();
			ContextHandle.AddOrigin( SocketLocation );
			ContextHandle.AddInstigator( MeshComp->GetOwner()->GetInstigatorController(), MeshComp->GetOwner() );
			ContextHandle.SetAbility( SourceInterface->GetAbilitySystemComponent()->GetAnimatingAbility() );
			ContextHandle.AddSourceObject( this );

			SpecHandle = SourceInterface->GetAbilitySystemComponent()->MakeOutgoingSpec( UNAGE_Damage::StaticClass(), 1.f, ContextHandle );
			if ( const UNAMontageCombatComponent* CombatComponent = MeshComp->GetOwner()->GetComponentByClass<UNAMontageCombatComponent>() )
			{
				SpecHandle.Data->SetSetByCallerMagnitude( FGameplayTag::RequestGameplayTag( "Data.Damage" ), -CombatComponent->GetBaseDamage() );	
				BaseDamage = CombatComponent->GetBaseDamage();
			}
		}

		// 서버와 클라이언트 간 플레이어 컨트롤러 설정 동기화
		if (MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>())
		{
			MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>()->bUseControllerDesiredRotation = false;
			MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>()->StopMovementImmediately();
			MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>()->DisableMovement();
		}
		// monster는 aicontroller가 서버에서 만들어 지기 때문에 동기화 과정은 필요 없을거 같음
		//else if (MeshComp->GetOwner()->GetComponentByClass<UPawnMovementComponent>())
		//{
		//	MeshComp->GetOwner()->GetComponentByClass<UPawnMovementComponent>()->bUseControllerDesiredRotation = false;
		//	MeshComp->GetOwner()->GetComponentByClass<UPawnMovementComponent>()->StopMovementImmediately();
		//	MeshComp->GetOwner()->GetComponentByClass<UPawnMovementComponent>()->DisableMovement();
		//}

	}
}

void UNAAnimNotifyState_SphereOverlapTest::NotifyEnd( USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference )
{
	Super::NotifyEnd( MeshComp, Animation, EventReference );

	if ( MeshComp->GetWorld()->IsGameWorld() )
	{
		if ( MeshComp->GetOwner()->HasAuthority() )
		{
			SpecHandle.Clear();
			ContextHandle.Clear();
			AppliedActors.Empty();
		}

		// 서버와 클라이언트 간 플레이어 컨트롤러 설정 동기화
		if (MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>())
		{
			MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>()->bUseControllerDesiredRotation = true;
			MeshComp->GetOwner()->GetComponentByClass<UCharacterMovementComponent>()->SetMovementMode(MOVE_Walking);
		}
	}
}

void UNAAnimNotifyState_SphereOverlapTest::NotifyTick( USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                       float FrameDeltaTime, const FAnimNotifyEventReference& EventReference )
{
	Super::NotifyTick( MeshComp, Animation, FrameDeltaTime, EventReference );

	// 에디터에서 서버도 같이 찾아가지고 gameworld 먼저 확인
	if (MeshComp->GetWorld()->IsGameWorld())
	{
		// 충돌 처리는 서버의 책임
		if (MeshComp->GetOwner()->HasAuthority())
		{
			// 충돌 확인 지연
			OverlapElapsed += FrameDeltaTime;

			if (OverlapElapsed >= OverlapInterval)
			{
				const FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
				TArray<FOverlapResult> OverlapResults;
				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(MeshComp->GetOwner()); // 시전자 제외
				const bool bOverlap = MeshComp->GetWorld()->OverlapMultiByChannel
				(
					OverlapResults,
					SocketLocation,
					FQuat::Identity,
					ECC_Pawn,
					FCollisionShape::MakeSphere(SphereRadius),
					QueryParams
				);

#if WITH_EDITOR || UE_BUILD_DEBUG
				DrawDebugSphere
				(
					MeshComp->GetWorld(),
					SocketLocation,
					SphereRadius,
					8,
					bOverlap || !OverlapResults.IsEmpty() ? FColor::Green : FColor::Red
				);
#endif

				if (!OverlapResults.IsEmpty() && MeshComp->GetWorld()->IsGameWorld())
				{
					const TScriptInterface<IAbilitySystemInterface>& SourceInterface = MeshComp->GetOwner();

					if (!SourceInterface)
					{
						// GAS가 없는 객체로부터 시도됨
						check(false);
						return;
					}

					for (const FOverlapResult& OverlapResult : OverlapResults)
					{
						if (const TScriptInterface<IAbilitySystemInterface>& TargetInterface = OverlapResult.GetActor())
						{
							if (!AppliedActors.Contains(OverlapResult.GetActor()))
							{
								UE_LOG(LogTemp, Log, TEXT("[%hs]: Found target %s"), __FUNCTION__, *OverlapResult.GetActor()->GetName());
								SourceInterface->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget
								(
									*SpecHandle.Data.Get(),
									TargetInterface->GetAbilitySystemComponent()
								);
																
								// Damage 이벤트 보고 및 UAISense_Damage로 callback 함수 등록: 여기서 TargetActor는 피해를 입은 액터
								AActor* TargetActor = OverlapResult.GetActor();
								UAISense_Damage::ReportDamageEvent(GetWorld(), TargetActor, MeshComp->GetOwner(),BaseDamage, TargetActor->GetActorLocation(), MeshComp->GetOwner()->GetActorLocation());

								AppliedActors.Add(OverlapResult.GetActor());
							}
						}
					}
				}

				OverlapElapsed = 0.f;
			}
		}

	}
	else
	{
		
#if WITH_EDITOR || UE_BUILD_DEBUG
		const FVector SocketLocation = MeshComp->GetSocketLocation(SocketName);
		TArray<FOverlapResult> OverlapResults;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MeshComp->GetOwner()); // 시전자 제외
		const bool bOverlap = MeshComp->GetWorld()->OverlapMultiByChannel
		(
			OverlapResults,
			SocketLocation,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(SphereRadius),
			QueryParams
		);
		DrawDebugSphere
		(
			MeshComp->GetWorld(),
			SocketLocation,
			SphereRadius,
			8,
			bOverlap || !OverlapResults.IsEmpty() ? FColor::Green : FColor::Red
		);
#endif
	}
	
}
