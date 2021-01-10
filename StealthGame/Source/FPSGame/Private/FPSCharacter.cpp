// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSCharacter.h"
#include "FPSProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include"Net/UnrealNetwork.h"


AFPSCharacter::AFPSCharacter()
{
	// Create a CameraComponent	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
	CameraComponent->SetRelativeLocation(FVector(0, 0, BaseEyeHeight)); // Position the camera
	CameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1PComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	Mesh1PComponent->SetupAttachment(CameraComponent);
	Mesh1PComponent->CastShadow = false;
	Mesh1PComponent->SetRelativeRotation(FRotator(2.0f, -15.0f, 5.0f));
	Mesh1PComponent->SetRelativeLocation(FVector(0, 0, -160.0f));

	// Create a gun mesh component
	GunMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	GunMeshComponent->CastShadow = false;
	GunMeshComponent->SetupAttachment(Mesh1PComponent, "GripPoint");

	NoiseEmitterComponent = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("NoiseEmitter"));
}


void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSCharacter::Fire);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
}

void AFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// we want to update the movement (looking up and down was not working for example)
	// so we want to run the animation on the other buys
	if(!IsLocallyControlled())
	{
		// we are rotating the camera one because it is the easiest to rotate such that the other actors rotate well with it
		FRotator NewRot = CameraComponent->GetRelativeRotation();
		// RemoteViewPitch is unit8 so it's compressed, to get it, we need to uncompress it then store it in FRotator
		NewRot.Pitch = RemoteViewPitch * 360.f/ 255.0f;

		CameraComponent->SetRelativeRotation(NewRot);
	}
}

void AFPSCharacter::ServerFire_Implementation()
{
	// so practically the projectiles run on the server - no matter who shot them - and clients can see them
	// try and fire a projectile
	if (ProjectileClass)
	{
		FVector MuzzleLocation = GunMeshComponent->GetSocketLocation("Muzzle");
		FRotator MuzzleRotation = GunMeshComponent->GetSocketRotation("Muzzle");

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		// when ever we fire, tell the projectile which insitigator did that 
		ActorSpawnParams.Instigator = this;

		// spawn the projectile at the muzzle
		GetWorld()->SpawnActor<AFPSProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, ActorSpawnParams);
	}
}

bool AFPSCharacter::ServerFire_Validate()
{
	return true;
}


void AFPSCharacter::Fire()
{
	// this function if called :
	// from client it will ask the server to run it
	// from server it will do the normal sutff and just excute
	ServerFire();
	
	

	// try and play the sound if specified
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1PComponent->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->PlaySlotAnimationAsDynamicMontage(FireAnimation, "Arms", 0.0f);
		}
	}
}


void AFPSCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}


void AFPSCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

// it's like a function to get the properties which are related only to the machine we are running on (read this please)
// https://docs.unrealengine.com/en-US/API/Runtime/Engine/GameFramework/AActor/GetLifetimeReplicatedProps/index.html
// 
void AFPSCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSCharacter, bIsCarryingObjective);

	// this one is faster
	//DOREPLIFETIME_CONDITION(AFPSCharacter, bIsCarryingObjective, COND_OwnerOnly);
}