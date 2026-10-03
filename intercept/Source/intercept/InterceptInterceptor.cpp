#include "InterceptInterceptor.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "InterceptTarget.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

// Console toggle (intercept.DebugGuidance 1) so guidance can be visualised on
// every rocket without editing Blueprints.
static TAutoConsoleVariable<bool> CVarDebugGuidance(
    TEXT("intercept.DebugGuidance"), false,
    TEXT("Draw line of sight and commanded acceleration for all interceptors."),
    ECVF_Cheat);

AInterceptInterceptor::AInterceptInterceptor() {
  PrimaryActorTick.bCanEverTick = true;

  Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
  Collision->InitSphereRadius(25.f);
  Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
  Collision->SetGenerateOverlapEvents(true);
  RootComponent = Collision;

  Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
  Mesh->SetupAttachment(Collision);
  Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  // Engine cylinder (100cm) scaled into a rocket-like body so it is visible
  // with no content.
  static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(
      TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
  if (CylinderMesh.Succeeded()) {
    Mesh->SetStaticMesh(CylinderMesh.Object);
    Mesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 1.6f));
    // Cylinder axis is Z; rotate so the actor's forward (X) is the rocket's
    // nose.
    Mesh->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
  }

  // The glow lets the player follow the rocket against the sky.
  GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
  GlowLight->SetupAttachment(Collision);
  GlowLight->SetAttenuationRadius(1200.f);
  GlowLight->SetCastShadows(false);
}

void AInterceptInterceptor::BeginPlay() {
  Super::BeginPlay();
  Collision->OnComponentBeginOverlap.AddDynamic(
      this, &AInterceptInterceptor::HandleOverlap);

  // BasicShapeMaterial exposes a "Color" vector parameter; if a Blueprint swaps
  // in a material without it, this is a harmless no-op.
  if (UMaterialInstanceDynamic* Material =
          Mesh->CreateAndSetMaterialInstanceDynamic(0)) {
    Material->SetVectorParameterValue(TEXT("Color"), GlowColor);
  }
  GlowLight->SetLightColor(GlowColor);
  GlowLight->SetIntensity(GlowIntensity);
}

void AInterceptInterceptor::Launch(AInterceptTarget* InTarget,
                                   const FVector& InitialVelocity) {
  Target = InTarget;
  Velocity = InitialVelocity;
  SetActorRotation(Velocity.Rotation());
}

void AInterceptInterceptor::Tick(float DeltaSeconds) {
  Super::Tick(DeltaSeconds);
  if (bDetonated) {
    return;
  }

  Age += DeltaSeconds;
  if (Age >= MaxLifetime) {
    Detonate();
    return;
  }

  const FVector Position = GetActorLocation();

  if (IsValid(Target)) {
    if (FVector::DistSquared(Position, Target->GetActorLocation()) <=
        FMath::Square(ProximityFuseRadius)) {
      Detonate();
      return;
    }

    FInterceptGuidance Guidance;
    Guidance.NavigationConstant = NavigationConstant;
    Guidance.MaxLateralAcceleration = MaxLateralAcceleration;
    Guidance.bAugmented = bAugmentedGuidance;

    FInterceptGuidance::FState State;
    State.MissilePosition = Position;
    State.MissileVelocity = Velocity;
    State.TargetPosition = Target->GetActorLocation();
    State.TargetVelocity = Target->GetTargetVelocity();
    State.TargetAcceleration = Target->GetTargetAcceleration();

    FVector Command = Guidance.ComputeAcceleration(State);
    if (Command.IsZero()) {
      // PN gives no command if the target is opening or the geometry is
      // degenerate; steer straight at the target so the rocket can recover
      // instead of coasting.
      const FVector ToTarget =
          (State.TargetPosition - Position).GetSafeNormal();
      const FVector Lateral =
          ToTarget - FVector::DotProduct(ToTarget, Velocity.GetSafeNormal()) *
                         Velocity.GetSafeNormal();
      Command = Lateral * MaxLateralAcceleration;
    }

    Velocity += Command * DeltaSeconds;

    if (bDebugDraw || CVarDebugGuidance.GetValueOnGameThread()) {
      DrawDebugLine(GetWorld(), Position, State.TargetPosition, FColor::Yellow,
                    false, -1.f);
      DrawDebugLine(GetWorld(), Position, Position + Command * 0.01f,
                    FColor::Red, false, -1.f);
    }
  }

  // Thrust along the heading up to cruise speed. Applied after steering so the
  // lateral command (which is perpendicular to velocity) can't be mistaken for
  // speed change.
  const FVector Heading = Velocity.GetSafeNormal();
  const float Speed = Velocity.Size();
  if (Speed < CruiseSpeed) {
    Velocity = Heading * FMath::Min(Speed + ThrustAcceleration * DeltaSeconds,
                                    CruiseSpeed);
  } else {
    // Discrete steering adds a little speed each step; renormalise to keep
    // speed constant.
    Velocity = Heading * CruiseSpeed;
  }

  SetActorLocation(Position + Velocity * DeltaSeconds);
  SetActorRotation(Velocity.Rotation());
}

void AInterceptInterceptor::HandleOverlap(
    UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult) {
  // Ignore the shooter and other rockets; targets and world geometry both
  // detonate.
  if (!OtherActor || OtherActor == this || OtherActor == GetOwner() ||
      OtherActor->IsA<AInterceptInterceptor>()) {
    return;
  }
  Detonate();
}

void AInterceptInterceptor::Detonate() {
  if (bDetonated) {
    return;
  }
  bDetonated = true;

  UGameplayStatics::ApplyRadialDamage(this, BlastDamage, GetActorLocation(),
                                      BlastRadius, nullptr, TArray<AActor*>(),
                                      this, GetInstigatorController(), true);
  Destroy();
}
