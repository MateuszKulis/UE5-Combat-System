#include "PlayerCharacter.h"

#include "GameFramework/Actor.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/CameraShake.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnemyBase.h"
#include "Animation/AnimInstance.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	TargetLockDistance = 1500.0f;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(RootComponent);

	CameraBoom->TargetArmLength = 500.0f;

	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	FollowCamera->bUsePawnControlRotation = false;

	Weapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(GetMesh(), "RightHandItem");
	Weapon->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	EnemyDetectionCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Enemy Detection Collider"));
	EnemyDetectionCollider->SetupAttachment(RootComponent);
	EnemyDetectionCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	EnemyDetectionCollider->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,
		ECollisionResponse::ECR_Overlap);
	EnemyDetectionCollider->SetSphereRadius(TargetLockDistance);

	Attacking = false;
	IsAttackLunge = false;
	Rolling = false;
	TargetLocked = false;
	NextAttackReady = false;
	AttackDamaging = false;
	AttackIndex = 0;

	PassiveMovementSpeed = 450.0f;
	CombatMovementSpeed = PassiveMovementSpeed;
	GetCharacterMovement()->MaxWalkSpeed = PassiveMovementSpeed;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	EnemyDetectionCollider->OnComponentBeginOverlap.
		AddDynamic(this, &APlayerCharacter::OnEnemyDetectionBeginOverlap);
	EnemyDetectionCollider->OnComponentEndOverlap.
		AddDynamic(this, &APlayerCharacter::OnEnemyDetectionEndOverlap);

	TSet<AActor*> NearActors;
	EnemyDetectionCollider->GetOverlappingActors(NearActors);
	for (auto& EnemyActor : NearActors)
	{
		if (Cast<AEnemyBase>(EnemyActor)) {
			NearbyEnemies.Add(EnemyActor);
			UE_LOG(LogTemp, Warning, TEXT("Enemy detected in BeginPlay: %s"), *EnemyActor->GetName());
		}
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (Rolling || IsAttackLunge)
	{
		float Value = 0;
		if (Rolling)
			Value = 600.0f;
		else if (IsAttackLunge) {
			FocusTarget();
			Value = 100.0f;
		}

		AddMovementInput(GetActorForwardVector(), Value * GetWorld()->GetDeltaSeconds());
	}
	else if (Stumbling && MovingBackwards)
	{
		AddMovementInput(-GetActorForwardVector(), 40.0 * GetWorld()->GetDeltaSeconds());
	}
	else if (Attacking && AttackDamaging)
	{
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

					GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(CameraShakeMinor);
				}
			}
		}

	}

	/*if (Target != NULL && TargetLocked && !IsAttackLunge)
	{
		FVector TargetDirection = Target->GetActorLocation() - GetActorLocation();

		if (TargetDirection.Size2D() > 400)
		{
			FRotator Difference = UKismetMathLibrary::NormalizedDeltaRotator(Controller->GetControlRotation(), TargetDirection.ToOrientationRotator());

			if (FMath::Abs(Difference.Yaw) > 30.0f)
			{
				AddControllerYawInput(DeltaTime * -Difference.Yaw * 0.5f);
			}
		}
	}*/
	

}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("CombatModeToggle", IE_Pressed, this,
		&APlayerCharacter::ToggleCombatMode);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this,
		&APlayerCharacter::Attack);
	PlayerInputComponent->BindAction("Roll", IE_Pressed, this,
		&APlayerCharacter::Roll);
	PlayerInputComponent->BindAction("CycleTarget+", IE_Pressed, this,
		&APlayerCharacter::CycleTargetClockwise);
	PlayerInputComponent->BindAction("CycleTarget-", IE_Pressed, this,
		&APlayerCharacter::CycleTargetCounterClockwise);
}

void APlayerCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && !IsAttackLunge  && !Attacking)//&& !Rolling  && !Stumbling
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(Direction, Value);
	}

	InputDirection.X = Value;
}

void APlayerCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f) && !IsAttackLunge  && !Attacking)//&& !Rolling  && !Stumbling
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, Value);
	}

	InputDirection.Y = Value;
}

void APlayerCharacter::CycleTarget(bool Clockwise)
{
	if (NearbyEnemies.Num() == 0)
	{
		Target = nullptr;
		TargetLocked = false;
		UE_LOG(LogTemp, Warning, TEXT("No nearby enemies to target"));
		return;
	}

	int32 TargetIndex = NearbyEnemies.IndexOfByKey(Target);

	if (Clockwise)
	{
		TargetIndex = (TargetIndex + 1) % NearbyEnemies.Num();
	}
	else
	{
		TargetIndex = (TargetIndex - 1 + NearbyEnemies.Num()) % NearbyEnemies.Num();
	}

	Target = NearbyEnemies[TargetIndex];
	TargetLocked = true;

	UE_LOG(LogTemp, Warning, TEXT("New target: %s"), *Target->GetName());
}

void APlayerCharacter::CycleTargetClockwise()
{
	CycleTarget(true);
}

void APlayerCharacter::CycleTargetCounterClockwise()
{
	CycleTarget(false);
}

void APlayerCharacter::LookAtSmooth()
{
}

float APlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.0f && !Rolling)
	{
		PlayStumbleAnimation();

		if (BloodEffectNiagara)
		{
			FVector HitLocation = GetActorLocation();
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BloodEffectNiagara, HitLocation);
		}
	}

	return ActualDamage;
}

void APlayerCharacter::PlayStumbleAnimation()
{

	if (TakeHit_StumbleBackwards.Num() > 0)
	{
		//Stumbling = true;
		int32 AnimIndex = FMath::RandRange(0, TakeHit_StumbleBackwards.Num() - 1);
		UAnimMontage* AnimMontage = TakeHit_StumbleBackwards[AnimIndex];
		if (AnimMontage)
		{
			PlayAnimMontage(AnimMontage);
			float StumbleDuration = AnimMontage->GetPlayLength();
			GetWorldTimerManager().SetTimer(StumbleTimerHandle, this, &APlayerCharacter::EndStumble, StumbleDuration, false);

		}
	}
}

void APlayerCharacter::EndStumble()
{
	Stumbling = false;
	Attacking = false;
	AttackDamaging = false;
	IsAttackLunge = false;
	NextAttackReady = true;
}


void APlayerCharacter::OnEnemyDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
	if (Enemy)
	{
		NearbyEnemies.Add(Enemy);
		if (!TargetLocked)
			CycleTargetClockwise();
	}
}

void APlayerCharacter::OnEnemyDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
	if (Enemy)
	{
		NearbyEnemies.Remove(Enemy);

		if (Target == Enemy)
		{
			Target = nullptr;
			TargetLocked = false;
		}
	}
}

void APlayerCharacter::Attack()
{
	if (Attacks.Num() > 0 && !Attacking && !Rolling && !Stumbling)
	{
		Attacking = true;
		NextAttackReady = false;
		AttackDamaging = true;
		AttackHitActors.Empty();

		UAnimMontage* AttackMontage = Attacks[AttackIndex];
		if (AttackMontage)
		{
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(AttackMontage);

				float AttackDuration = AttackMontage->GetPlayLength();
				GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &APlayerCharacter::EndAttack, AttackDuration, false);
			}
		}

		AttackIndex = (AttackIndex + 1) % Attacks.Num();
	}
}

void APlayerCharacter::EndAttack()
{
	Attacking = false;
	AttackDamaging = false;
	NextAttackReady = true;
}

void APlayerCharacter::AttackSoftLunge()
{
	GetCharacterMovement()->MaxWalkSpeed = PassiveMovementSpeed * 1.2f;
	IsAttackLunge = true;

}

void APlayerCharacter::EndSoftLunge()
{
	GetCharacterMovement()->MaxWalkSpeed = TargetLocked ? CombatMovementSpeed : PassiveMovementSpeed;
	IsAttackLunge = false;
}

void APlayerCharacter::Roll()
{
	if (CombatRoll && !Rolling && !Stumbling)
	{
		Rolling = true;

		PlayAnimMontage(CombatRoll);

		StartRoll();

		Attacking = false;
		AttackDamaging = false;
		NextAttackReady = true;

		float RollDuration = CombatRoll->GetPlayLength();
		GetWorldTimerManager().SetTimer(RollTimerHandle, this, &APlayerCharacter::EndRoll, RollDuration, false);
	}
}

void APlayerCharacter::StartRoll()
{
	GetCharacterMovement()->MaxWalkSpeed = PassiveMovementSpeed * 1.5f;
}

void APlayerCharacter::EndRoll()
{
	Rolling = false;
	Attacking = false;
	AttackDamaging = false;
	NextAttackReady = true;
	GetCharacterMovement()->MaxWalkSpeed = TargetLocked ? CombatMovementSpeed : PassiveMovementSpeed;
}

void APlayerCharacter::RollRotateSmooth()
{
}

void APlayerCharacter::FocusTarget()
{
	if (TargetLocked && Target)
	{
		FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target->GetActorLocation());
		FRotator SmoothRotation = FMath::RInterpTo(GetActorRotation(), LookAtRotation, GetWorld()->GetDeltaSeconds(), 15.0f);
		SetActorRotation(SmoothRotation);
	}
}

void APlayerCharacter::ToggleCombatMode()
{
	TargetLocked = !TargetLocked;
	SetInCombat(TargetLocked);
}

void APlayerCharacter::SetInCombat(bool InCombat)
{
	GetCharacterMovement()->MaxWalkSpeed = InCombat ? CombatMovementSpeed : PassiveMovementSpeed;
}