// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Combatant.generated.h"

UCLASS()
class SOULSLIKECOMBAT_API ACombatant : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACombatant();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	AActor* Target;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool TargetLocked;

	bool Attacking;
	bool AttackDamaging;
	bool MovingForward;
	bool MovingBackwards;
	bool NextAttackReady;
	bool Stumbling;

	bool RotateTowardsTarget;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float RotationSmoothing;

	UPROPERTY(EditAnywhere, Category = "Animations")
	TArray<UAnimMontage*> AttackAnimations;

	UPROPERTY(EditAnywhere, Category = "Animations")
	TArray<UAnimMontage*> TakeHit_StumbleBackwards;

	TArray<AActor*> AttackHitActors;

	virtual void Attack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void AttackLunge();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void EndAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void SetAttackDamaging(bool Damaging);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	virtual void SetMovingForward(bool IsMovingForward);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	virtual void SetMovingBackwards(bool IsMovingBackwards);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	virtual void EndStumble();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void AttackNextReady();

	virtual void LookAtSmooth();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	float GetCurrentRotationSpeed();

	float LastRotationSpeed;

};

