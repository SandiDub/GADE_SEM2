#include "TdPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SceneComponent.h"

ATdPawn::ATdPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
	SetRootComponent(Pivot);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Pivot);
	SpringArm->TargetArmLength = 3500.f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetRelativeRotation(FRotator(-55.f, 0.f, 0.f));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bAutoActivate = true;
}

void ATdPawn::BeginPlay()
{
	Super::BeginPlay();
}

void ATdPawn::FrameLocation(FVector WorldLocation)
{
	SetActorLocation(FVector(WorldLocation.X, WorldLocation.Y, 0.f));
}

void ATdPawn::Pan(FVector2D Axis)
{
	const FVector Delta = FVector(Axis.Y, Axis.X, 0.f) * PanSpeed * GetWorld()->GetDeltaSeconds();
	AddActorWorldOffset(Delta, false);
}
