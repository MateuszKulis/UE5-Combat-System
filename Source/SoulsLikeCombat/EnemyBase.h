#pragma once

#include "CoreMinimal.h"
#include "Combatant.h"
#include "EnemyBase.generated.h"


UENUM(BlueprintType)
enum class State : uint8
{
	IDLE,					
	CHASE_CLOSE,			
	CHASE_FAR,				
	ATTACK,					
	STUMBLE,				
	TAUNT,					
	DEAD					
};

class UNiagaraSystem;

UCLASS()
class SOULSLIKECOMBAT_API AEnemyBase : public ACombatant
{
	GENERATED_BODY()

public:

	AEnemyBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UStaticMeshComponent* Weapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Finite State Machine")
	State ActiveState;

	UPROPERTY(EditAnywhere, Category = "Effects")
	UNiagaraSystem* BloodEffectNiagara; 

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser);

	UPROPERTY(EditAnywhere, Category = "Animations")
	UAnimMontage* OverheadSmash;

	int LastStumbleIndex;

protected:

	virtual void BeginPlay() override;

	virtual void TickStateMachine();

	void SetState(State NewState);

	virtual void StateIdle();

	virtual void StateChaseClose();

	virtual void StateChaseFar();

	virtual void StateAttack();

	virtual void StateStumble();

	virtual void EndStumble();

	virtual void StateTaunt();

	virtual void StateDead();

	virtual void MoveForward();

	virtual void Attack(bool Rotate = true);

	void AttackNextReady();

	void EndAttack();

	virtual void AttackLunge();

	FTimerHandle AttackTimerHandle;
	FTimerHandle StumbleTimerHandle;


	bool Interruptable;

public:

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void FocusTarget();

	FORCEINLINE class UStaticMeshComponent* GetWeapon() const { return Weapon; }

};
