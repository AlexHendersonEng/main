#include "InterceptTargetSpawner.h"

#include "InterceptTarget.h"
#include "TimerManager.h"

AInterceptTargetSpawner::AInterceptTargetSpawner() {
  PrimaryActorTick.bCanEverTick = false;
  RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
  TargetClass = AInterceptTarget::StaticClass();
}

void AInterceptTargetSpawner::BeginPlay() {
  Super::BeginPlay();
  if (bAutoStart) {
    SetSpawningEnabled(true);
  }
}

void AInterceptTargetSpawner::SetSpawningEnabled(bool bEnabled) {
  FTimerManager& Timers = GetWorldTimerManager();
  if (bEnabled) {
    Timers.SetTimer(
        SpawnTimer,
        FTimerDelegate::CreateWeakLambda(this, [this]() { SpawnTarget(); }),
        SpawnInterval, true);
  } else {
    Timers.ClearTimer(SpawnTimer);
  }
}

AInterceptTarget* AInterceptTargetSpawner::SpawnTarget(float SpeedMultiplier) {
  if (!TargetClass) {
    return nullptr;
  }

  const FVector Defended = GetActorLocation() + DefendedOffset;

  // Uniform random bearing around the ring so approaches come from all sides.
  const float Bearing = FMath::FRandRange(0.f, 2.f * PI);
  const FVector SpawnLocation =
      Defended + FVector(FMath::Cos(Bearing) * SpawnRadius,
                         FMath::Sin(Bearing) * SpawnRadius, SpawnHeight);

  FActorSpawnParameters Params;
  Params.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  AInterceptTarget* Target = GetWorld()->SpawnActor<AInterceptTarget>(
      TargetClass, SpawnLocation, FRotator::ZeroRotator, Params);
  if (Target) {
    Target->SetTargetVelocity((Defended - SpawnLocation).GetSafeNormal() *
                              TargetSpeed * SpeedMultiplier);
  }
  return Target;
}
