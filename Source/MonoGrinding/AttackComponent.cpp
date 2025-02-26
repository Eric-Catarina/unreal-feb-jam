// Fill out your copyright notice in the Description page of Project Settings.

#include "AttackComponent.h"

#include "EnemyComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MonoGrinding/DefaultPlayer.h"
#include "NiagaraFunctionLibrary.h"

UAttackComponent::UAttackComponent() {
    PrimaryComponentTick.bCanEverTick = true;
}

void UAttackComponent::BeginPlay() {
    Super::BeginPlay();

    GetWorld()->GetTimerManager().SetTimer(TimerHandle_Attack, this,
                                           &UAttackComponent::PerformAttack,
                                           AttackInterval, true);
}

void UAttackComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAttackComponent::PerformAttack() {
    TArray<AActor *> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(
        GetWorld(), AMonoGrindingCharacter::StaticClass(),
        FoundActors); // Consider filtering by enemy class

    UHealthComponent *HealthComponent =
        GetOwner()->FindComponentByClass<UHealthComponent>();

    if (!HealthComponent || HealthComponent->IsDead)
        return;

    AActor *NearestTarget = nullptr;
    float NearestDistance = AttackRange;

    for (AActor *PossibleTargetActor : FoundActors) {
        AMonoGrindingCharacter *PossibleTargetCharacter =
            Cast<AMonoGrindingCharacter>(PossibleTargetActor);

        if (!PossibleTargetCharacter || PossibleTargetCharacter == GetOwner() ||
            TargetType != PossibleTargetCharacter->Team) {
            continue;
        }

        float Distance =
            FVector::Distance(PossibleTargetActor->GetActorLocation(),
                              GetOwner()->GetActorLocation());
        if (Distance >= NearestDistance)
            continue;

        NearestTarget = PossibleTargetActor;
        NearestDistance = Distance;
    }

    if (!NearestTarget)
        return;

    DealDamage(NearestTarget);

    if (SlashVFX && SlashVFX2) {
        // Calcula a rotação do VFX para apontar para o alvo
        const FVector Direction =
            (NearestTarget->GetActorLocation() - GetOwner()->GetActorLocation())
                .GetSafeNormal();
        const FRotator VFXRotation = Direction.Rotation();

        // Spawn do VFX na posição do alvo com a rotação calculada
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), SlashVFX, GetOwner()->GetActorLocation(), VFXRotation);
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), SlashVFX2, NearestTarget->GetActorLocation(),
            VFXRotation);
    }

    if (HitSound) {
        UGameplayStatics::PlaySoundAtLocation(this, HitSound,
                                              GetOwner()->GetActorLocation());
    }
}

void UAttackComponent::DealDamage(AActor *Target) {
    UE_LOG(LogTemp, Warning, TEXT("%f was dealed from %s to %s"), AttackDamage,
           *GetOwner()->GetName(), *Target->GetName());
    UGameplayStatics::ApplyDamage(Target, AttackDamage,
                                  GetOwner()->GetInstigatorController(),
                                  GetOwner(), nullptr);
}
