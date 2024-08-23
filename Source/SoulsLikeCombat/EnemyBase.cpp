#include "EnemyBase.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AEnemyBase::AEnemyBase()
{
	Weapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon"));
	//Weapon->SetupAttachment(GetMesh(), "RightHandItem");
	Weapon->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	PrimaryActorTick.bCanEverTick = true;
	MovingForward = false;
	MovingBackwards = false;
	Interruptable = true;
	LastStumbleIndex = 0;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	ActiveState = State::IDLE;

	Target = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Attacking) {
		TSet<AActor*> WeaponOverlappingActors;
		Weapon->GetOverlappingActors(WeaponOverlappingActors);

		for (AActor* HitActor : WeaponOverlappingActors)
		{
			if (HitActor == this)
				continue;

			if (!AttackHitActors.Contains(HitActor))
			{
				float AppliedDamage = UGameplayStatics::ApplyDamage(HitActor, 1.0f, GetController(), this, UDamageType::StaticClass());

				if (AppliedDamage > 0.0f)
				{
					AttackHitActors.Add(HitActor);

					//GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(CameraShakeMinor);
				}
			}
		}
	}


	TickStateMachine();

	


}

void AEnemyBase::TickStateMachine()
{
	switch (ActiveState)
	{
	case State::IDLE:
		StateIdle();
		break;

	case State::CHASE_CLOSE:
		StateChaseClose();
		break;

	case State::CHASE_FAR:
		StateChaseFar();
		break;

	case State::ATTACK:
		StateAttack();
		break;

	case State::STUMBLE:
		StateStumble();
		break;

	/*case State::TAUNT:
		break;*/

	case State::DEAD:
		StateDead();
		break;

	}
}

void AEnemyBase::SetState(State NewState)
{
	if (ActiveState != State::DEAD)
		ActiveState = NewState;
}

void AEnemyBase::StateIdle()
{
	if (Target && FVector::Distance(Target->GetActorLocation(), GetActorLocation()) <= 1200.0f && !Attacking) 
	{
		TargetLocked = true;

		SetState(State::CHASE_CLOSE);
	}
}

void AEnemyBase::StateChaseClose()
{
	float Distance = FVector::Distance(GetActorLocation(), Target->GetActorLocation());

	if (Distance <= 300.0f) 
	{
		FVector TargetDirection = Target->GetActorLocation() - GetActorLocation();

		float DotProduct = FVector::DotProduct(GetActorForwardVector(), TargetDirection.GetSafeNormal());

		if (DotProduct > 0.95f && !Attacking && !Stumbling)
		{
			Attack(false);
		}
	}
	else
	{
		AAIController* AIController = Cast<AAIController>(Controller);
		if (!AIController->IsFollowingAPath())
			AIController->MoveToActor(Target);
	}
}

void AEnemyBase::StateChaseFar()
{
}

void AEnemyBase::StateAttack()
{
	

	if (AttackAnimations.Num() > 0)
	{
		AttackHitActors.Empty();
		int32 AnimIndex = FMath::RandRange(0, AttackAnimations.Num() - 1);
		UAnimMontage* AnimMontage = AttackAnimations[AnimIndex];
		if (AnimMontage)
		{
			PlayAnimMontage(AnimMontage);
			float AttackDuration = AnimMontage->GetPlayLength();
			GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AEnemyBase::EndAttack, AttackDuration, false);
		}
	}
	SetState(State::IDLE);
}

void AEnemyBase::StateStumble()
{
	if (TakeHit_StumbleBackwards.Num() > 0)
	{
		Stumbling = true;
		int32 AnimIndex = FMath::RandRange(0, TakeHit_StumbleBackwards.Num() - 1);
		UAnimMontage* AnimMontage = TakeHit_StumbleBackwards[AnimIndex];
		if (AnimMontage)
		{
			PlayAnimMontage(AnimMontage);
			float StumbleDuration = AnimMontage->GetPlayLength();
			GetWorldTimerManager().SetTimer(StumbleTimerHandle, this, &AEnemyBase::EndStumble, StumbleDuration, false);
		}
	}
	SetState(State::IDLE);

}

void AEnemyBase::EndStumble()
{
	Stumbling = false;
} 

void AEnemyBase::StateTaunt()
{
}

void AEnemyBase::StateDead()
{
}

void AEnemyBase::FocusTarget()
{
}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.0f)
	{	
		SetState(State::STUMBLE);	

		if (BloodEffectNiagara)
		{
			FVector HitLocation = GetActorLocation();
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BloodEffectNiagara, HitLocation);
		}
	}

	return ActualDamage;
}

void AEnemyBase::MoveForward()
{
}

void AEnemyBase::Attack(bool Rotate)
{
	Attacking = true;
	SetState(State::ATTACK);
}

void AEnemyBase::AttackNextReady()
{
}

void AEnemyBase::EndAttack()
{
	Attacking = false;
}

void AEnemyBase::AttackLunge()
{
}

void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}