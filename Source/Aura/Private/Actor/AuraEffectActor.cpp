// Copyright KroNus Games 2023
//thanks to Jeff from Discord DruidMechanics, i use ur code for multiple effects


#include "Actor/AuraEffectActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"


AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}

void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, const TSubclassOf<UGameplayEffect> GameplayEffectClass, const EEffectRemovalPolicy EffectRemovalPolicy)
{
	if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies) return;
	
	// Uses Global Library (AbilitySystemGlobals) to get the Ability System Component
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC == nullptr) return;
	
	check(GameplayEffectClass);
	// From Ability System Component u can get the GameplayEffectContext with MakeEffectContext
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	// From Ability System Component u can get the GameplayEffectSpec with MakeOutgoingSpec
	// Contains a TSharedPtr named Data
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);
	// EffectSpecHandle.Data is a raw pointer, to get the EffectSpec use .Get() and add dereference the Ptr with * at start
	const FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	
	const bool bIsInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
	if (bIsInfinite && EffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap && ActiveEffectHandle.IsValid())
	{
		ActiveEffectHandles.Add(ActiveEffectHandle, TargetASC);
	}

	if (!bIsInfinite && bDestroyOnEffectApplication)
	{
		Destroy();
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	// Loop through all the instant effects and apply them
	for (const FAuraEffect& InstantEffect : InstantEffects)
	{
		if (InstantEffect.EffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
		{
			ApplyEffectToTarget(TargetActor, InstantEffect.GameplayEffectClass);
		}
	}

	// Loop through all the duration effects and apply them
	for (const FAuraEffect& DurationEffect : DurationEffects)
	{
		if (DurationEffect.EffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
		{
			ApplyEffectToTarget(TargetActor, DurationEffect.GameplayEffectClass);
		}
	}

	// Loop through all the infinite effects and apply them
	for (const FAuraEffect& InfiniteEffect : InfiniteEffects)
	{
		if (InfiniteEffect.EffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
		{
			ApplyEffectToTarget(TargetActor, InfiniteEffect.GameplayEffectClass);
		}
	}
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!IsValid(TargetASC)) return;

	// Loop through all the instant effects and apply them
	for (const auto& InstantEffect : InstantEffects)
	{
		if (InstantEffect.EffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
		{
			ApplyEffectToTarget(TargetActor, InstantEffect.GameplayEffectClass);
		}
	}

	// Loop through all the duration effects and apply them
	for (const auto& DurationEffect : DurationEffects)
	{
		if (DurationEffect.EffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
		{
			ApplyEffectToTarget(TargetActor, DurationEffect.GameplayEffectClass);
		}
	}

	// Loop through all the infinite effects and apply them
	for (const auto& InfiniteEffect : InstantEffects)
	{
		if (InfiniteEffect.EffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
		{
			ApplyEffectToTarget(TargetActor, InfiniteEffect.GameplayEffectClass);
		}
	} 

	TArray<FActiveGameplayEffectHandle> ActiveEffectHandlesToRemove;
	for (const auto& InfiniteEffect : InfiniteEffects)
	{
		if (InfiniteEffect.EffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
		{
			// Find the handles that match the target ASC
			for (const auto& ActiveEffectHandle : ActiveEffectHandles)
			{
				if (TargetASC == ActiveEffectHandle.Value)
				{
					TargetASC->RemoveActiveGameplayEffect(ActiveEffectHandle.Key, ActorLevel);
					ActiveEffectHandlesToRemove.Add(ActiveEffectHandle.Key);
				}
			}
		}
	}

	// Remove the handles from the array
	for (auto ActiveEffectHandleToRemove : ActiveEffectHandlesToRemove)
	{
		ActiveEffectHandles.Remove(ActiveEffectHandleToRemove);
	}
}


