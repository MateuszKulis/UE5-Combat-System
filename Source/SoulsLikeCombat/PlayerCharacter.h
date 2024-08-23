#pragma once

#include "CoreMinimal.h"
#include "Combatant.h"
#include "PlayerCharacter.generated.h"

/**
 *
 */

class UNiagaraSystem;

UCLASS()
class SOULSLIKECOMBAT_API APlayerCharacter : public ACombatant
{
	GENERATED_BODY()

public:
	APlayerCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class UStaticMeshComponent* Weapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* EnemyDetectionCollider;

	UPROPERTY(EditAnywhere, Category = "Effects")
	UNiagaraSystem* BloodEffectNiagara;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float PassiveMovementSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float CombatMovementSpeed;

	void CycleTarget(bool Clockwise = true);

	UFUNCTION()
	void CycleTargetClockwise();

	UFUNCTION()
	void CycleTargetCounterClockwise();

	void LookAtSmooth();

	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser);

	UPROPERTY(EditAnywhere, Category = "Animations")
	TArray<class UAnimMontage*> Attacks;

	UPROPERTY(EditAnywhere, Category = "Animations")
	class UAnimMontage* CombatRoll;

	UPROPERTY(EditAnywhere, Category = Camera)
	TSubclassOf<class UCameraShakeBase> CameraShakeMinor;

	bool Rolling;
	FRotator RollRotation;

	int AttackIndex;
	float TargetLockDistance;

	TArray<class AActor*> NearbyEnemies;
	int LastStumbleIndex;

	FVector InputDirection;

	UFUNCTION()
	void OnEnemyDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEnemyDetectionEndOverlap(class UPrimitiveComponent*
		OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

protected:

	void MoveForward(float Value);
	void MoveRight(float Value);

	void Attack();
	void EndAttack();

	void Roll();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartRoll();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndRoll();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AttackSoftLunge();
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndSoftLunge();

	void RollRotateSmooth();
	void FocusTarget();
	void ToggleCombatMode();
	void SetInCombat(bool InCombat);

	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	void PlayStumbleAnimation();

	void EndStumble();

	FTimerHandle RollTimerHandle;

	FTimerHandle AttackTimerHandle;

	FTimerHandle StumbleTimerHandle;

	bool Attacking;
	bool IsAttackLunge;
	bool NextAttackReady;
	bool AttackDamaging;
	bool TargetLocked;

	AActor* Target;

	TSet<AActor*> AttackHitActors;

public:

	class USpringArmComponent* GetCameraBoom() const
	{
		return CameraBoom;
	}

	class UCameraComponent* GetFollowCamera() const
	{
		return FollowCamera;
	}

	class UStaticMeshComponent* GetWeapon() const
	{
		return Weapon;
	}

};
