// Copyright KroNus Games 2023


#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"

#include "Player/AuraPlayerController.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MyObj;
}

void UTargetDataUnderMouse::Activate()
{
	//Because PlayerController is a TWeakObjectPtr we have to use .Get()
	APlayerController* PlayerController = Ability->GetCurrentActorInfo()->PlayerController.Get();
	
	FHitResult CursorHit;
	PlayerController->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	ValidData.Broadcast(CursorHit.Location);
}
