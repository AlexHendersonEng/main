#include "InterceptTarget.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AInterceptTarget::AInterceptTarget() {
  PrimaryActorTick.bCanEverTick = true;

  Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
  Collision->InitSphereRadius(50.f);
  // Overlap everything: the target moves itself, so a blocking collision would
  // only cause sweep stops.
  Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
  Collision->SetGenerateOverlapEvents(true);
  RootComponent = Collision;

  Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
  Mesh->SetupAttachment(Collision);
  Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  // Engine sphere (100cm diameter) as a placeholder so the target is visible
  // with no content.
  static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
      TEXT("/Engine/BasicShapes/Sphere.Sphere"));
  if (SphereMesh.Succeeded()) {
    Mesh->SetStaticMesh(SphereMesh.Object);
  }
}

void AInterceptTarget::BeginPlay() {
  Super::BeginPlay();
  Health = MaxHealth;
  Collision->OnComponentBeginOverlap.AddDynamic(
      this, &AInterceptTarget::HandleOverlap);
}

void AInterceptTarget::Tick(float DeltaSeconds) {
  Super::Tick(DeltaSeconds);

  // Semi-implicit Euler: update velocity first, then position, which is stable
  // for a constant-gravity arc.
  Velocity += GetTargetAcceleration() * DeltaSeconds;
  SetActorLocation(GetActorLocation() + Velocity * DeltaSeconds);
}

FVector AInterceptTarget::GetTargetAcceleration() const {
  return FVector(0.f, 0.f,
                 GetWorld() ? GetWorld()->GetGravityZ() * GravityScale : 0.f);
}

float AInterceptTarget::TakeDamage(float DamageAmount,
                                   FDamageEvent const& DamageEvent,
                                   AController* EventInstigator,
                                   AActor* DamageCauser) {
  const float Applied = Super::TakeDamage(DamageAmount, DamageEvent,
                                          EventInstigator, DamageCauser);
  if (bFinished || Applied <= 0.f) {
    return Applied;
  }

  Health -= Applied;
  if (Health <= 0.f) {
    bFinished = true;
    OnTargetDestroyed.Broadcast(this, DamageCauser);
    Destroy();
  }
  return Applied;
}

void AInterceptTarget::HandleOverlap(UPrimitiveComponent* OverlappedComponent,
                                     AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp,
                                     int32 OtherBodyIndex, bool bFromSweep,
                                     const FHitResult& SweepResult) {
  // Other targets are ignored; anything else (ground, pawn, buildings) counts
  // as an impact.
  if (!OtherActor || OtherActor == this ||
      OtherActor->IsA<AInterceptTarget>()) {
    return;
  }
  Impact(OtherActor);
}

void AInterceptTarget::Impact(AActor* HitActor) {
  if (bFinished) {
    return;
  }
  bFinished = true;
  OnTargetImpact.Broadcast(this, HitActor);
  Destroy();
}
