// Copyright 2022 Dominik Lips. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "AI/RVOAvoidanceInterface.h"
#include "AI/Navigation/NavigationAvoidanceTypes.h"
#include "GenMovementComponent.h"
#include "GenOrganicMovementComponent.generated.h"

DECLARE_STATS_GROUP(TEXT("GMCOrganicMovementComponent_Game"), STATGROUP_GMCOrganicMovementComp, STATCAT_Advanced);

USTRUCT()
struct GENERALMOVEMENT_API FMove_OrganicMovement : public FMove
{
  GENERATED_BODY()

  FMove_OrganicMovement()
  {
    bSerializeInputVectorX = true;
    bSerializeInputVectorY = true;
    bSerializeInputVectorZ = true;
    bSerializeRotationInputRoll = false;
    bSerializeRotationInputPitch = true;
    bSerializeRotationInputYaw = true;
    bSerializeOutLocation = true;
    bSerializeOutRotationRoll = false;
    bSerializeOutRotationPitch = false;
    bSerializeOutRotationYaw = true;
    bSerializeOutControlRotationRoll = false;
    bSerializeOutControlRotationPitch = true;
    bSerializeOutControlRotationYaw = true;
  }

  bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) { return Super::NetSerialize(Ar, Map, bOutSuccess); }
};

template<>
struct TStructOpsTypeTraits<FMove_OrganicMovement> : public TStructOpsTypeTraitsBase2<FMove_OrganicMovement>
{
  enum
  {
    WithNetSerializer = true
  };
};

UENUM(BlueprintType)
enum class EGenMovementMode : uint8
{
  None UMETA(DisplayName = "None", ToolTip = "No movement."),
  Grounded UMETA(DisplayName = "Grounded", ToolTip = "Moving along a surface under the effect of gravity."),
  Airborne UMETA(DisplayName = "Airborne", ToolTip = "Moving through the air with or without being affected by gravity."),
  Buoyant UMETA(DisplayName = "Buoyant", ToolTip = "Moving through a fluid volume under the effect of buoyancy."),
  Custom1 UMETA(DisplayName = "Custom 1"),
  Custom2 UMETA(DisplayName = "Custom 2"),
  Custom3 UMETA(DisplayName = "Custom 3"),
  Custom4 UMETA(DisplayName = "Custom 4"),
  Custom5 UMETA(DisplayName = "Custom 5"),
  Custom6 UMETA(DisplayName = "Custom 6"),
  Custom7 UMETA(DisplayName = "Custom 7"),
  Custom8 UMETA(DisplayName = "Custom 8"),
  Custom9 UMETA(DisplayName = "Custom 9"),
  Custom10 UMETA(DisplayName = "Custom 10"),
  Custom11 UMETA(DisplayName = "Custom 11"),
  Custom12 UMETA(DisplayName = "Custom 12"),
  MAX UMETA(Hidden),
};

UENUM(BlueprintType)
enum class EStepMontagePolicy : uint8
{
  All UMETA(DisplayName = "All", ToolTip = "All montages must be stepped manually. Provides the most accurate network synchronisation for the mesh pose but may not be practical for complex animation systems."),
  RootMotionOnly UMETA(DisplayName = "RootMotionOnly", ToolTip = "Only root motion montages must be stepped manually. Recommended for most networked games."),
  None UMETA(DisplayName = "None", ToolTip = "All montages are played via Unreal's default node and are not stepped manually. Only for standalone games."),
  MAX UMETA(Hidden),
};

/// Movement component intended for animate things such as humans and animals. Typically organic movement is characterized by quick
/// acceleration to a maximum speed (where most of the movement happens) and quick deceleration to a stop again. Physical forces like air
/// resistance are being ignored with this type of movement due to their negligible influence. This implementation is not limited to bipeds
/// or any particular collision shape.
UCLASS(ClassGroup = "Movement", BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class GENERALMOVEMENT_API UGenOrganicMovementComponent : public UGenMovementComponent, public IRVOAvoidanceInterface
{
  GENERATED_BODY()

public:

  UGenOrganicMovementComponent();

  void BeginPlay() override;
  void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
  void NotifyBumpedPawn(APawn* BumpedPawn) override;
  void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;
  FVector GetPenetrationAdjustment(const FHitResult& Hit) const override;
  FVector ComputeSlideVector(const FVector& Delta, float Time, const FVector& Normal, const FHitResult& Hit) const override;
  float SlideAlongSurface(const FVector& Delta, float Time, const FVector& Normal, FHitResult& Hit, bool bHandleImpact = false) override;
  void TwoWallAdjust(FVector& Delta, const FHitResult& Hit, const FVector& OldHitNormal) const override;
  void HaltMovement() override;
  bool CanMove() const override;
  USceneComponent* SetRootCollisionShape(EGenCollisionShape NewCollisionShape, const FVector& Extent, FName Name = {}/*not used*/) override;
  void SetRootCollisionExtent(const FVector& NewExtent, bool bUpdateOverlaps = true) override;
  EGenCollisionShape GetRootCollisionShape() const override;
  FVector GetRootCollisionExtent() const override;
  FHitResult AutoResolvePenetration() override;
  bool StepMontage(USkeletalMeshComponent* Mesh, UAnimMontage* Montage, float Position, float Weight, float PlayRate = 1.f) override;

#if WITH_EDITOR

  void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

#endif

  /// Whether the root collision shape and extent should be replicated. This is implemented in the most general way (direct replication of
  /// the collision shape enum and the extent vector) which has the advantage that it works in virtually all scenarious, but it will be
  /// quite inefficient with regard to bandwidth usage for most applications (because they usually don't require such a general solution).
  /// Disable if you handle collision replication yourself of if you don't need it.
  /// @attention Only handles the collision shape and extent, no other component settings will be replicated.
  static constexpr bool REPLICATE_COLLISION = true;
  /// Prefixes for the names given to a dynamically spawned root components (@see SetRootCollisionShape).
  inline static const TCHAR* ROOT_NAME_DYNAMIC_CAPSULE = TEXT("RCSDynamicCapsule");
  inline static const TCHAR* ROOT_NAME_DYNAMIC_FLAT_CAPSULE = TEXT("RCSDynamicFlatCapsule");
  inline static const TCHAR* ROOT_NAME_DYNAMIC_BOX = TEXT("RCSDynamicBox");
  inline static const TCHAR* ROOT_NAME_DYNAMIC_SPHERE = TEXT("RCSDynamicSphere");
  /// If we reach a velocity magnitude lower or equal to this value when braking, velocity is zeroed.
  static constexpr float BRAKE_TO_STOP_VELOCITY = KINDA_SMALL_NUMBER;
  /// The minimum velocity magnitude the pawn should have when falling off a ledge.
  static constexpr float MIN_LEDGE_FALL_OFF_VELOCITY = 500.f;
  /// The minimum size of the deceleration vector when braking. Prevents the velocity from becoming very small when it approaches 0.
  static constexpr float MIN_DECELERATION = 5.f;
  /// When in a fluid, buoyant force will only be applied if the pawn's immersion exceeds this tolerance so it remains more stable at the
  /// water line.
  static constexpr float IMMERSION_DEPTH_TOLERANCE = 0.04f;
  /// Scale applied to the fluid friction of a physics volume when the pawn is in water.
  static constexpr float FLUID_FRICTION_SCALE = 8.f;
  /// How close to the water line the pawn still needs to be when trying to exit a fluid volume in order for @see FluidMinExitSpeed to be
  /// considered. Measured as a percentage of the collision height.
  static constexpr float EXIT_FLUID_IMMERSION_TOLERANCE = 0.1f;
  /// The minimum distance the pawn should maintain to the floor when grounded.
  static constexpr float MIN_DISTANCE_TO_FLOOR = 1.9f;
  /// The maximum distance the pawn should maintain to the floor when grounded.
  static constexpr float MAX_DISTANCE_TO_FLOOR = 2.4f;
  /// Reject impacts that are this close to the edge of the vertical portion of the collision shape when performing vertical sweeps.
  static constexpr float EDGE_TOLERANCE = 0.15f;
  /// The min size of the step up location delta to be applied, so we don't fail the step up due to an unwalkable surface when starting from
  /// a resting position.
  static constexpr float MIN_STEP_UP_DELTA = 5.f;

#pragma region Replication Interface

  void LoadServerStateReplicationPreset(ENetRole RecipientRole) override;

protected:

  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  EPreReplicatedHalfByte CurrentRootCollisionShapeAccessor{};
  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  EPreReplicatedVector CurrentRootCollisionExtentAccessor{};
  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  EPreReplicatedHalfByte MovementModeAccessor{};
  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  EPreReplicatedBool ReceivedUpwardForceAccessor{};
  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  EPreReplicatedBool HasAnimRootMotionAccessor{};

  void BindReplicationData_Implementation() override;
  void OnServerAuthPhysicsSimulationToggled_Implementation(bool bEnabled, FServerAuthPhysicsSettings Settings) override;
  void OnClientAuthPhysicsSimulationToggled_Implementation(bool bEnabled, FClientAuthPhysicsSettings Settings) override;
  void OnGMCDisabled_Implementation() override;
  void GenReplicatedTick_Implementation(float DeltaTime) override;
  void GenSimulatedTick_Implementation(float DeltaTime) override;
  void OnImmediateStateLoaded_Implementation(EImmediateContext Context) override;
  void OnSimulatedStateLoaded_Implementation(
    const FState& SmoothState,
    const FState& StartState,
    const FState& TargetState,
    ESimulatedContext Context
  ) override;
  FMove PreSimulateMovementForExtrapolation_Implementation(
    const FMove& Move,
    float ExtrapolationTime,
    const FState& StartState,
    const FState& TargetState,
    const FState& ExtrapolationState,
    const TArray<int32>& SkippedStateIndices
  ) override;

private:

  UFUNCTION(Server, Reliable, WithValidation)
  void Server_SendMoves_OrganicMovement(const TArray<FMove_OrganicMovement>& RemoteMoves);
  void Server_SendMoves_OrganicMovement_Implementation(const TArray<FMove_OrganicMovement>& RemoteMoves) {
       Server_SendMoves_OrganicMovement_Implementation2(RemoteMoves); }
  bool Server_SendMoves_OrganicMovement_Validate(const TArray<FMove_OrganicMovement>& RemoteMoves) { return
       Server_SendMoves_OrganicMovement_Validate2(RemoteMoves); }
  IMPLEMENT_CUSTOM_SERIALIZATION_SETTINGS(FMove_OrganicMovement, LocalMove_OrganicMovement, Server_SendMoves_OrganicMovement)

#pragma endregion

#pragma region Movement Physics

protected:

  UPROPERTY(BlueprintReadWrite, Category = "General Movement Component")
  /// The current movement mode of the pawn. Default values: 0 = None, 1 = Grounded, 2 = Airborne, 3 = Buoyant. Supports up to 16 replicated
  /// states.
  /// @attention Use @see SetMovementMode to prompt a call to @see OnMovementModeChanged. If you don't want the event to be triggered the
  /// movement mode can be set directly.
  /// @attention We don't use the enum (@see EGenMovementMode) directly for the movement mode to be able to simply bind it to a half-byte
  /// for replication.
  uint8 MovementMode{0};

  UPROPERTY(BlueprintReadWrite, Category = "General Movement Component")
  /// Holds information about the floor currently located underneath the pawn. Note that the pawn does not necessarily have to be grounded
  /// for this to be valid (@see FloorTraceLength).
  FFloorParams CurrentFloor;

  UPROPERTY(BlueprintReadWrite, Category = "General Movement Component")
  /// Whether the pawn received some form of upward force that should be considered when the movement mode is updated during the next tick.
  /// This flag gets reset automatically after it was processed (@see UpdateMovementModeDynamic).
  bool bReceivedUpwardForce{false};

  UPROPERTY(BlueprintReadWrite, Category = "General Movement Component")
  /// Holds whatever the value of @see MaxDesiredSpeed was when the pawn spawned.
  float DefaultSpeed{0.f};

  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  /// The current collision shape of the pawn (@see EGenCollisionShape). Mainly used for replication (@see REPLICATE_COLLISION).
  uint8 CurrentRootCollisionShape{0};

  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  /// The current extent of the pawn's root collision. Mainly used for replication (@see REPLICATE_COLLISION).
  FVector CurrentRootCollisionExtent{0};

  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  /// The input vector that is used for all physics calculations. May be different from the raw input vector received from the replication
  /// component (@see PreProcessInputVector).
  FVector ProcessedInputVector{0};

  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  /// Minimum Z value of the normal of a walkable surface. Computed from the walkable floor angle.
  float WalkableFloorZ{0.f};

  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  /// The current immersion depth of the pawn. Range is from 0 (not in fluid) to 1 (fully immersed).
  float CurrentImmersionDepth{0.f};

  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  /// The 2D direction in which the pawn should fall off a ledge when exceeding @see LedgeFallOffThreshold. Will be a zero vector if the
  /// pawn is currently not in the process of falling off.
  FVector LedgeFallOffDirection{0};

  UPROPERTY(BlueprintReadOnly, Category = "General Movement Component")
  /// If true, the pawn is currently stuck in geometry and cannot move.
  bool bStuckInGeometry{false};

  /// Used to maintain consistency between the root collision component and the bound data members. Mainly used for replication.
  /// @see REPLICATE_COLLISION
  /// @see CurrentRootCollisionShape
  /// @see CurrentRootCollisionExtent
  ///
  /// @returns      void
  virtual void MaintainRootCollisionCoherency();

  /// Clamps selected data members to their respective valid range when the actor is spawned and before movement is executed.
  ///
  /// @returns      void
  virtual void ClampToValidValues();

  /// Returns the current movement mode of the pawn.
  ///
  /// @returns      EGenMovementMode    The current movement mode.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  EGenMovementMode GetMovementMode() const;

  /// Main function for moving the pawn and updating all associated data.
  ///
  /// @param        DeltaSeconds    The current move delta time.
  /// @returns      void
  virtual void PerformMovement(float DeltaSeconds);

  /// Executes the movement physics based on the current movement mode for this tick.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  virtual void RunPhysics(float DeltaSeconds);

  /// Allows for pre-processing of the raw input vector. Called after the movement mode was updated.
  ///
  /// @param        RawInputVector    The original input vector reflecting the raw input data.
  /// @returns      FVector           The actual input vector to be used for all physics calculations.
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  FVector PreProcessInputVector(FVector RawInputVector);
  virtual FVector PreProcessInputVector_Implementation(FVector RawInputVector) { return RawInputVector; }

  /// Only called while the updated component is simulating physics. No other movement code apart from what is implemented here will be
  /// executed while physics simulation is active.
  ///
  /// @param        DeltaSeconds    The current move delta time.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  void PhysicsSimulationUpdate(float DeltaSeconds);
  virtual void PhysicsSimulationUpdate_Implementation(float DeltaSeconds) {}

  /// Called before any kind of movement related update has happened. This is the only movement event that is called even if the pawn cannot
  /// move (@see CanMove).
  ///
  /// @param        DeltaSeconds    The current move delta time.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  void PreMovementUpdate(float DeltaSeconds);
  virtual void PreMovementUpdate_Implementation(float DeltaSeconds);

  /// Called after movement was performed. If we are playing a montage the pose has been ticked and root motion was consumed at the time
  /// this function is called.
  ///
  /// @param        DeltaSeconds    The current move delta time.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  void PostMovementUpdate(float DeltaSeconds);
  virtual void PostMovementUpdate_Implementation(float DeltaSeconds) {}

  /// Called immediately before switching on the current movement mode and executing the appropriate physics. At this point the movement
  /// mode and floor have been updated, and the input vector has been pre-processed (@see GetProcessedInputVector).
  ///
  /// @param        DeltaSeconds    The current move delta time.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  void PrePhysicsUpdate(float DeltaSeconds);
  virtual void PrePhysicsUpdate_Implementation(float DeltaSeconds) {}

  /// Called immediately after the movement physics have run.
  ///
  /// @param        DeltaSeconds    The current move delta time.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  void PostPhysicsUpdate(float DeltaSeconds);
  virtual void PostPhysicsUpdate_Implementation(float DeltaSeconds);

  /// Called at the end of the current movement update. This is the preferred entry point for subclasses to implement custom logic if
  /// automatic handling of the default movement modes is desired.
  ///
  /// @param        DeltaSeconds    The current move delta time (may not be equal to the frame delta time).
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  void MovementUpdate(float DeltaSeconds);
  virtual void MovementUpdate_Implementation(float DeltaSeconds) {}

  /// Handles the physics for grounded movement.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void PhysicsGrounded(float DeltaSeconds);

  /// Handles the physics for airborne movement.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void PhysicsAirborne(float DeltaSeconds);

  /// Handles the physics for movement in a fluid.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void PhysicsBuoyant(float DeltaSeconds);

  /// Handles the physics for custom movement modes.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  void PhysicsCustom(float DeltaSeconds);
  virtual void PhysicsCustom_Implementation(float DeltaSeconds) {}

  /// Calculates the new movement velocity for based on the current pawn state.
  /// @attention Velocity from root motion animations is calculated separately (@see CalculateAnimRootMotionVelocity).
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void CalculateVelocity(float DeltaSeconds);

  /// Whether direct movement should be performed for bots. Only relevant when @see UNavMovementComponent::bUseAccelerationForPaths is set
  /// to false.
  ///
  /// @returns      bool    True to call @see BotDirectMove to calculate the move velocity, false to proceed with the regular velocity
  ///                       calculation instead.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  bool ShouldPerformDirectMove() const;
  virtual bool ShouldPerformDirectMove_Implementation() const;

  /// Event for calculating and setting the move velocity of bots when direct movement is used for path following. Direct movement (which
  /// can be performed when @see UNavMovementComponent::bUseAccelerationForPaths is set to false) is not based on move input, instead path
  /// following provides a velocity to reach the goal directly.
  ///
  /// @param        PathVelocity    The velocity requested by path following.
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  void BotDirectMove(const FVector& PathVelocity, float DeltaSeconds);
  virtual void BotDirectMove_Implementation(const FVector& PathVelocity, float DeltaSeconds);

  /// Apply velocity from move input.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void ApplyInputVelocity(float DeltaSeconds);

  /// Change the pawn's orientation.
  ///
  /// @param        bIsDirectBotMove    Whether the pawn is a bot executing direct movement (i.e. the pawn is not using acceleration values
  ///                                   to calculate its velocity).
  /// @param        DeltaSeconds        The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void ApplyRotation(bool bIsDirectBotMove, float DeltaSeconds);

  /// Apply velocity from external forces (e.g. gravity).
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void ApplyExternalForces(float DeltaSeconds);

  /// Apply acceleration to make the pawn fall off a ledge if required (@see MIN_LEDGE_FALL_OFF_VELOCITY).
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  virtual void ApplyLedgeFallOffAcceleration(float DeltaSeconds);

  /// Add forces that oppose the current velocity.
  /// @attention Might cause the velocity to be clamped to 0 (important to consider when maintaining framerate independency).
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void ApplyDeceleration(float DeltaSeconds);

  /// Slows the pawn down to max speed if necessary.
  /// @attention Might cause the velocity to be clamped (important to consider when maintaining framerate independency).
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void LimitSpeed(float DeltaSeconds);

  /// Enforces the max desired speed in the XY-plane.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  virtual void LimitSpeedXY(float DeltaSeconds);

  /// Enforces the max desired speed in all directions.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  virtual void LimitSpeedXYZ(float DeltaSeconds);

  /// Limits the max speed in downward Z-direction to the terminal velocity of the current physics volume if the pawn is falling down.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  virtual void LimitAirborneSpeedTerminalVelocity(float DeltaSeconds);

  /// Called from @see CalculateVelocity for custom velocity computations. This is called before any clamping of the velocity happens and
  /// should be preferred to overriding the CalculateVelocity function directly. Not called for bots executing a direct move.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  void CalculateVelocityCustom(float DeltaSeconds);
  virtual void CalculateVelocityCustom_Implementation(float DeltaSeconds) {}

  /// Recalculates the current immersion depth of the pawn and updates @see CurrentImmersionDepth with the new value.
  ///
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void UpdateImmersionDepth();

  /// Used to prevent the pawn from leaving a fluid volume in certain circumstances. We do not want the pawn to leave a fluid volume if it
  /// is just experiencing a minor upward force and is still close to the water line, otherwise there is a risk of repeatedly switching in
  /// and out of the buoyant state.
  ///
  /// @param        Floor                 The current floor parameters.
  /// @param        ImmersionTolerance    How close to the water line the pawn must still be for an adjustment to happen (as a percentage
  ///                                     of the current root collision height).
  /// @param        SpeedTolerance        No adjustment will be made if the pawn's current upward speed is equal to or greater than the
  ///                                     passed value. Passing 0 effectively "disables" this function.
  /// @param        DeltaSeconds          The delta time to use.
  /// @returns      bool                  True if an adjustment was made and the pawn should remain in a buoyant state, false otherwise.
  virtual bool CheckLeaveFluid(const FFloorParams& Floor, float ImmersionTolerance, float SpeedZTolerance, float DeltaSeconds);

  /// Sets @see bReceivedUpwardForce based on the velocity before and after @see MovementUpdate and the value of the current processed input
  /// vector (@see ProcessedInputVector). Called after the movement update has run and the mesh pose was ticked.
  /// @attention Does not consider the input vector if the input mode (@see EInputMode) is set to "AllAbsolute" or "None".
  ///
  /// @param        PreviousVelocity    The velocity value from before the movement update.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  void CheckReceivedUpwardForce(const FVector& PreviousVelocity);
  virtual void CheckReceivedUpwardForce_Implementation(const FVector& PreviousVelocity);

  /// Calculates the acceleration generated from the input vector and the configured input acceleration.
  ///
  /// @returns      FVector    The calculated input acceleration.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  FVector ComputeInputAcceleration() const;
  virtual FVector ComputeInputAcceleration_Implementation() const;

  /// Returns the current input modifier (e.g. from an analog stick) i.e. the size of the input vector (clamped to a max size of 1). When
  /// grounded, the negative Z component of the input vector is not factored in.
  ///
  /// @returns      float    The current input modifier in the range [0,1].
  virtual float ComputeInputModifier() const;

  /// Calculates the buoyancy force a pawn should experience when immersed in a fluid.
  ///
  /// @param        PawnMass               The mass of the pawn in kg.
  /// @param        Gravity                The gravity Z-component in cm/s^2.
  /// @param        BuoyancyCoefficient    Coefficient conflating quantities that we don't want to consider individually (liquid density,
  ///                                      body volume, etc.).
  /// @returns      FVector                The calculated buoyancy force.
  virtual float ComputeBuoyantForce(float PawnMass, float GravityZ, float BuoyancyCoefficient) const;

  /// Clamps the passed vector to the min deceleration in XY-direction and Z-direction.
  ///
  /// @param        Deceleration    The current deceleration vector.
  /// @returns      FVector         The clamped deceleration vector.
  virtual FVector ClampToMinDeceleration(const FVector& Deceleration) const;

  /// Makes the pawn follow its current movement base.
  ///
  /// @param        BaseVelocity    The velocity of the component the pawn is currently based on.
  /// @param        Floor           The current floor.
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  virtual void MoveWithBase(const FVector& BaseVelocity, FFloorParams& Floor, float DeltaSeconds);

  /// Get the velocity of the object the pawn is currently based on.
  ///
  /// @param        MovementBase    The component the pawn is currently based on.
  /// @returns      FVector         The (linear and angular) velocity of the movement base.
  virtual FVector ComputeBaseVelocity(UPrimitiveComponent* MovementBase);

  /// Returns the base velocity when the current movement base is a pawn. In a networked context we must take care not to use a values that
  /// are not synchronised between server and client.
  ///
  /// @param        PawnMovementBase    The current movement base of type pawn.
  /// @returns      FVector             The net-safe base velocity.
  virtual FVector GetPawnBaseVelocity(APawn* PawnMovementBase) const;

  /// Called at the end of @see ComputeBaseVelocity. Allows for user-defined post-processing of the calculated base velocity.
  /// @attention Called every update for which the pawn has a valid base, even if that base is not moveable.
  ///
  /// @param        MovementBase    The component the pawn is currently based on.
  /// @param        BaseVelocity    The base velocity that was calculated up to this point.
  /// @returns      FVector         The final base velocity to use.
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  FVector PostProcessBaseVelocity(UPrimitiveComponent* MovementBase, const FVector& BaseVelocity);
  virtual FVector PostProcessBaseVelocity_Implementation(UPrimitiveComponent* MovementBase, const FVector& BaseVelocity) { return BaseVelocity; }

  /// Called when the movement mode changes from grounded to airborne and the velocity of the movement base should be imparted. Allows for
  /// user-defined post-processing of the calculated velocity to impart.
  /// @attention Only called if @see ShouldImpartVelocityFromBase returned true.
  ///
  /// @param        MovementBase        The component of which the velocity should be imparted.
  /// @param        VelocityToImpart    The impart-velocity that was calculated up to this point.
  /// @returns      FVector             The final velocity to impart.
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  FVector PostProcessVelocityToImpart(UPrimitiveComponent* MovementBase, const FVector& VelocityToImpart);
  virtual FVector PostProcessVelocityToImpart_Implementation(UPrimitiveComponent* MovementBase, const FVector& VelocityToImpart) { return VelocityToImpart; }

  /// Called at the end of @see CalculateVelocity. Allows for user-defined post-processing of the calculated pawn velocity (by modifying the
  /// value of @see UMovementComponent::Velocity). Also called for bots executing a direct move.
  ///
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  void PostProcessPawnVelocity();
  virtual void PostProcessPawnVelocity_Implementation() {}

  /// Updates the movement mode dynamically (i.e. with regard to the current movement mode). Returning false indicates that the movement
  /// mode should still be updated statically (@see UpdateMovementModeStatic) afterwards, returning true will skip the static update.
  ///
  /// @param        Floor           The current floor parameters.
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      bool            If false, the movement mode will still be updated statically afterwards.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  bool UpdateMovementModeDynamic(const FFloorParams& Floor, float DeltaSeconds);
  virtual bool UpdateMovementModeDynamic_Implementation(const FFloorParams& Floor, float DeltaSeconds);

  /// Updates the movement mode statically (i.e. independent of the current movement mode).
  ///
  /// @param        Floor           The current floor parameters.
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  void UpdateMovementModeStatic(const FFloorParams& Floor, float DeltaSeconds);
  virtual void UpdateMovementModeStatic_Implementation(const FFloorParams& Floor, float DeltaSeconds);

  /// Called after the movement mode was updated dynamically and/or statically.
  /// @attention Do not confuse this function with @see OnMovementModeChanged.
  /// @see UpdateMovementModeDynamic
  /// @see UpdateMovementModeStatic
  ///
  /// @param        PreviousMovementMode    The movement mode we changed from (i.e. the value of @see MovementMode before it was updated).
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  void OnMovementModeUpdated(EGenMovementMode PreviousMovementMode);
  virtual void OnMovementModeUpdated_Implementation(EGenMovementMode PreviousMovementMode);

  /// Called when the updated component is stuck in geometry. If this happens no movement events after @see PreMovementUpdate will be called
  /// anymore.
  ///
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  void OnStuckInGeometry();
  virtual void OnStuckInGeometry_Implementation() {}

  /// Move the updated component along the floor while grounded. Movement velocity will always remain horizontal.
  ///
  /// @param        LocationDelta    The location delta to apply.
  /// @param        Floor            The current floor.
  /// @param        DeltaSeconds     The delta time to use.
  /// @returns      FVector          The applied location delta.
  virtual FVector MoveAlongFloor(const FVector& LocationDelta, FFloorParams& Floor, float DeltaSeconds);

  /// Move the updated component by the given location delta while in the air.
  ///
  /// @param        LocationDelta    The location delta to apply.
  /// @param        DeltaSeconds     The delta time to use.
  /// @returns      bool             True if the pawn landed on a walkable surface, false otherwise.
  virtual bool MoveThroughAir(const FVector& LocationDelta, float DeltaSeconds);

  /// Move the updated component by the given location delta while swimming through a fluid volume.
  ///
  /// @param        LocationDelta    The location delta to apply.
  /// @param        DeltaSeconds     The delta time to use.
  /// @returns      bool             True if the pawn left the fluid volume, false otherwise.
  virtual bool MoveThroughFluid(const FVector& LocationDelta, float DeltaSeconds);

  /// Helper function for moving through a fluid volume.
  ///
  /// @param        LocationDelta    The location delta to apply.
  /// @param        Hit              The hit result of the movement.
  /// @param        DeltaSeconds     The delta time to use.
  /// @returns      float            The remaining percentage of the location delta if the pawn has left the water during movement.
  virtual float Swim(const FVector& LocationDelta, FHitResult& Hit, float DeltaSeconds);

  /// Computes a vector that moves parallel to the hit ramp.
  ///
  /// @param        LocationDelta    The attempted location delta.
  /// @param        RampHit          The ramp that was hit.
  /// @returns      FVector          The new movement vector that moves parallel to the hit surface.
  virtual FVector ComputeRampVector(const FVector& LocationDelta, const FHitResult& RampHit) const;

  /// Hanldes stepping up barriers that do not exceed @see MaxStepUpHeight.
  ///
  /// @param        LocationDelta    The attempted location delta.
  /// @param        BarrierHit       The barrier that was hit.
  /// @param        Floor            The current floor.
  /// @param        OutForwardHit    The hit result of the step-up forward movement.
  /// @returns      bool             True if the pawn has scaled the barrier, false otherwise.
  virtual bool StepUp(
    const FVector& LocationDelta,
    const FHitResult& BarrierHit,
    FFloorParams& Floor,
    FHitResult* OutForwardHit = nullptr
  );

  /// Maintains the pawn's distance to the floor.
  ///
  /// @param        Floor    The current floor (should be up to date).
  /// @returns      bool     Whether the adjustment was successful and the pawn is within the set limits.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual bool MaintainDistanceToFloor(UPARAM(ref) FFloorParams& Floor);

  /// Determines the location of the water line. Respects the current movement state meaning the resulting position will be adjusted slighly
  /// towards the pawn's current physics volume.
  ///
  /// @param        LocationInWater       The location of the pawn inside the fluid volume.
  /// @param        LocationOutOfWater    The location of the pawn outside the fluid volume.
  /// @param        ImmersionDepth        The current immersion depth of the pawn.
  /// @returns      FVector               The location of the water line.
  virtual FVector FindWaterLine(const FVector& LocationInWater, const FVector& LocationOutOfWater, float ImmersionDepth) const;

  /// Handles the movement mode change from buoyant to airborne.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  virtual void ProcessLeftFluid(float DeltaSeconds);

  /// Wrapper around @see AdjustVelocityFromHit that prevents the pawn from gaining upward velocity from the hit adjustment.
  ///
  /// @param        Hit             The hit result of the collision.
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  void AdjustVelocityFromHitAirborne(const FHitResult& Hit, float DeltaSeconds);
  virtual void AdjustVelocityFromHitAirborne_Implementation(const FHitResult& Hit, float DeltaSeconds);

  /// Checks if the hit surface is a valid landing spot for the pawn (when trying to land after being airborne).
  ///
  /// @param        Hit         The hit surface.
  /// @returns      bool        True if the passed location is a valid landing spot, false otherwise.
  virtual bool IsValidLandingSpot(const FHitResult& Hit);

  /// Handles the movement mode change from airborne to grounded.
  ///
  /// @param        Hit             The hit result after landing.
  /// @param        DeltaSeconds    The delta time to use.
  /// @param        bUpdateFloor    Whether the current floor should be updated.
  /// @returns      void
  virtual void ProcessLanded(const FHitResult& Hit, float DeltaSeconds, bool bUpdateFloor);

  /// Called when the movement mode changes from airborne to grounded.
  ///
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  void OnLanded();
  virtual void OnLanded_Implementation() {}

  /// Calculates a new XY velocity for the pawn based on the fall control percentage and the input acceleration. Only applied while in the
  /// air and under the influence of gravity.
  ///
  /// @param        Control              The degree of control the pawn should have as a percentage of the passed acceleration.
  /// @param        InputAcceleration    The current input acceleration of the pawn.
  /// @param        DeltaSeconds         The delta time to use.
  /// @returns      void
  virtual void ApplyFallControl(float Control, const FVector& InputAcceleration, float DeltaSeconds);

  /// Check whether the pawn is perching further beyond a ledge than allowed by @see LedgeFallOffThreshold. For box collision the threshold
  /// is assumed to be either 1 (if LedgeFallOffThreshold >= 0.5) or 0 (if LedgeFallOffThreshold < 0.5).
  ///
  /// @param        ImpactPoint       The impact point to test for (usually from the hit result of the floor sweep).
  /// @param        PawnLowerBound    The lower bound of the pawn's collision.
  /// @param        PawnCenter        The center of the pawn's collision.
  /// @returns      bool              True if the pawn is exceeding the threshold, false otherwise.
  virtual bool IsExceedingFallOffThreshold(const FVector& ImpactPoint, const FVector& PawnLowerBound, const FVector& PawnCenter) const;

  /// Prevents the pawn from boosting up slopes when airborne by limiting the computed slide vector.
  ///
  /// @param        SlideResult    The computed slide vector.
  /// @param        Delta          The attempted movement location delta.
  /// @param        Time           The amount of the delta to apply.
  /// @param        Normal         The hit normal.
  /// @param        Hit            The original hit result that was used to compute the slide vector.
  /// @returns      FVector        The adjusted slide vector.
  virtual FVector HandleSlopeBoosting(
    const FVector& SlideResult,
    const FVector& Delta,
    float Time,
    const FVector& Normal,
    const FHitResult& Hit
  ) const;

  /// Determines whether the velocity of the movement base should be imparted.
  ///
  /// @param        MovementBase    The current movement base of the pawn.
  /// @returns      bool            True if the base velocity should be imparted, false otherwise.
  virtual bool ShouldImpartVelocityFromBase(UPrimitiveComponent* MovementBase) const;

  /// Called when the movement mode is changed through a call to @see SetMovementMode.
  /// @attention Do not confuse this function with @see OnMovementModeUpdated.
  ///
  /// @param        PreviousMovementMode    The previous movement mode we changed from.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  void OnMovementModeChanged(EGenMovementMode PreviousMovementMode);
  virtual void OnMovementModeChanged_Implementation(EGenMovementMode PreviousMovementMode);

  /// Can be overriden to return the input acceleration for custom movement modes. Called directly from @see GetInputAcceleration if the
  /// current movement mode is none of the default ones.
  ///
  /// @returns      float    The current input acceleration.
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  float GetInputAccelerationCustom() const;
  virtual float GetInputAccelerationCustom_Implementation() { return 0.f; }

  /// Can be overriden to return the braking deceleration for custom movement modes. Called directly from @see GetBrakingDeceleration if the
  /// current movement mode is none of the default ones.
  ///
  /// @returns      float    The current braking deceleration.
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  float GetBrakingDecelerationCustom() const;
  virtual float GetBrakingDecelerationCustom_Implementation() { return 0.f; }

  /// Can be overriden to return the over-max-speed-deceleration for custom movement modes. Called directly from
  /// @see GetOverMaxSpeedDeceleration if the current movement mode is none of the default ones.
  ///
  /// @returns      float    The deceleration applied when the pawn exceeds the max desired speed.
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  float GetOverMaxSpeedDecelerationCustom() const;
  virtual float GetOverMaxSpeedDecelerationCustom_Implementation() { return 0.f; }

  /// Delegate called when the root collision touches another primitive component.
  /// @see UPrimitiveComponent::OnComponentBeginOverlap
  UFUNCTION()
  virtual void RootCollisionTouched(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
  );

  /// Applies impact forces to the hit component when using physics interaction (@see bEnablePhysicsInteraction).
  ///
  /// @param        Impact                The hit result of the impact.
  /// @param        ImpactAcceleration    The acceleration of the pawn at the time of impact.
  /// @param        ImpactVelocity        The velocity of the pawn at the time of impact.
  /// @returns      void
  virtual void ApplyImpactPhysicsForces(const FHitResult& Impact, const FVector& ImpactAcceleration, const FVector& ImpactVelocity);

  /// Applies a downward force when walking on top of physics objects.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  virtual void ApplyDownwardForce(float DeltaSeconds);

  /// Applies a repulsion force to touched physics objects.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  virtual void ApplyRepulsionForce(float DeltaSeconds);

public:

  /// Returns the current max speed the pawn is allowed to have.
  /// @attention This does not simply retrieve @see MaxDesiredSpeed. The final speed may be influenced by other factors such as the velocity
  /// requested from path following or a currently applied analog input modifier.
  ///
  /// @returns      float    The current max speed the pawn should have.
  float GetMaxSpeed() const override;

  /// Returns the pre-processed input vector (@see PreProcessInputVector).
  /// @attention Not available until after the movement mode was updated meaning @see PrePhysicsUpdate is the earliest event for which the
  /// return value can be non-zero.
  ///
  /// @returns      FVector    The processed input vector used for all physics calculations.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual FVector GetProcessedInputVector() const;

  /// Returns the currently set input acceleration for the current movement mode.
  ///
  /// @returns      float    The current input acceleration.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual float GetInputAcceleration() const;

  /// Returns the currently set braking deceleration for the current movement mode.
  ///
  /// @returns      float    The current braking deceleration.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual float GetBrakingDeceleration() const;

  /// Returns the currently set deceleration applied when the pawn exceeds the max desired speed for the current movement mode.
  ///
  /// @returns      float    The current braking deceleration.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual float GetOverMaxSpeedDeceleration() const;

  /// Checks if the hit object is a walkable surface.
  ///
  /// @param        Hit     The hit result to be checked.
  /// @returns      bool    True if the surface is walkable, false if not.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual bool HitWalkableFloor(const FHitResult& Hit) const;

  /// Update the current movement mode. Calls @see OnMovementModeChanged if "NewMovementMode" is different from the current movement mode.
  ///
  /// @param        NewMovementMode    The new movement mode.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void SetMovementMode(EGenMovementMode NewMovementMode);

  /// Update the current movement mode. Calls @see OnMovementModeChanged if "NewMovementMode" is different from the current movement mode.
  ///
  /// @param        NewMovementMode    The new movement mode.
  /// @returns      void
  virtual void SetMovementMode(uint8 NewMovementMode);

  /// Update the current movement mode with the equivalent values from the built-in enum.
  ///
  /// @param        NewMovementMode    The new movement mode.
  /// @returns      void
  virtual void SetMovementMode(EMovementMode NewMovementMode);

  /// Returns the current immersion depth of the pawn (0 = not in a fluid volume, 1 = fully immersed).
  ///
  /// @returns      float    The current immersion depth of the pawn.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual float GetCurrentImmersionDepth() const;

  /// Returns the component the pawn is currently based on (if any). The pawn is only ever considered based if the current movement mode is
  /// "Grounded".
  ///
  /// @returns      UPrimitiveComponent*    The component the pawn is currently based on, nullptr if it currently has no movement base.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual UPrimitiveComponent* GetMovementBase() const;

  /// Returns the actor the pawn is currently based on (if any). The pawn is only ever considered based if the current movement mode is
  /// "Grounded".
  ///
  /// @returns      AActor*    The actor the pawn is currently based on, nullptr if it currently has no movement base.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual AActor* GetMovementBaseActor() const;

  /// Check if the pawn can receive input.
  ///
  /// @returns      bool    True if this pawn receives input, false otherwise.
  bool HasMoveInputEnabled() const override;

  /// Set the max walkable slope for the pawn by angle. Automatically updates the walkable floor Z.
  ///
  /// @param        NewWalkableFloorAngle    The new walkable floor angle in degrees.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void SetWalkableFloorAngle(float NewWalkableFloorAngle);

  /// Set the max walkable slope for the pawn by Z value. Automatically updates the walkable floor angle.
  ///
  /// @param        NewWalkableFloorZ    The new walkable floor Z value.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void SetWalkableFloorZ(float NewWalkableFloorZ);

  /// Checks whether the current velocity is exceeding the given max speed.
  ///
  /// @param        MaxSpeed    The current max speed of the pawn.
  /// @returns      bool        True if the current velocity is exceeding the given max speed, false otherwise.
  bool IsExceedingMaxSpeed(float MaxSpeed) const override;

  /// Returns the current (scaled) gravity Z component. Upward gravity is not supported, a value larger than 0 will be clamped.
  ///
  /// @returns      float    The current gravity Z component.
  float GetGravityZ() const override;

  /// Returns the current gravity as vector in the format (0, 0, GravityZ).
  ///
  /// @returns      FVector    The current gravity acceleration vector.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  FVector GetGravity() const;

  /// Completely disables kinematic movement.
  /// @attention This will set the movement mode (@see EGenMovementMode) to "None".
  ///
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void DisableMovement();

  /// Checks whether we are affected by gravity.
  ///
  /// @returns      bool    True if the pawn is influenced by gravity, false otherwise.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  bool IsAffectedByGravity() const;

  /// Checks whether we currently have the airborne movement state.
  ///
  /// @returns      bool    True if airborne, false otherwise.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  bool IsAirborne() const;

  /// Checks whether the pawn's Z velocity is currently exceeding the threshold for remaining grounded/being able to land.
  ///
  /// @returns      bool    True if the Z velocity is larger than @see MaxGroundedVelocityZ, false otherwise.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  bool IsExceedingMaxGroundedVelocityZ() const;

  /// Checks whether we currently have the airborne movement state, which is the equivalent of "MOVE_Falling" of the engine's built-in enum
  /// (@see EMovementMode). It is recommended to use @see IsAirborne instead of this function.
  ///
  /// @returns      bool    True if airborne, false otherwise.
  bool IsFalling() const override;

  /// Checks whether we currently have the grounded movement state.
  ///
  /// @returns      bool    True if grounded, false otherwise.
  bool IsMovingOnGround() const override;

  /// Checks whether we currently have the buoyant movement state.
  ///
  /// @returns      bool    True if swimming, false otherwise.
  bool IsSwimming() const override;

  /// Checks whether we are currently flying. We are flying if there is no gravity being applied to the pawn and we are airborne.
  ///
  /// @returns      bool    True if flying, false otherwise.
  bool IsFlying() const override;

#pragma endregion

#pragma region Root Motion

public:

  /// Sets a reference (@see SkeletalMesh) to the skeletal mesh component of the owning pawn. This is automatically done once when beginning
  /// play (taking the first skeletal mesh in the pawn's scene component hierarchy), but if the mesh is changed at any point after that the
  /// reference needs to be updated manually.
  /// @attention Setting a skeletal mesh reference will add the movement component as a tick prerequisite component for the mesh.
  ///
  /// @param        Mesh    The skeletal mesh to use. Passing nullptr clears the reference.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual void SetSkeletalMeshReference(USkeletalMeshComponent* Mesh);

  /// Returns the scaling factor applied to any animation root motion translation on this pawn.
  ///
  /// @returns      float    The scaling factor for animation root motion.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  float GetAnimRootMotionTranslationScale() const;

  /// Sets the scaling factor applied to any animation root motion translation on this pawn.
  ///
  /// @param        Scale    The scaling factor to use.
  /// @returns      void
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  void SetAnimRootMotionTranslationScale(float Scale = 1.f);

  /// Returns whether we currently have animation root motion to consider. Valid throughout the whole movement tick even when montages
  /// instances are cleared regularly for networked play (@see StepMontagePolicy). Should be preferred over using
  /// @see UGenMovementComponent::IsPlayingRootMotion which can be misleading in a networked context.
  ///
  /// @returns      bool    True if there currently is animation root motion to consider, false otherwise.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  bool HasAnimRootMotion() const;

protected:

  /// Blocks the automatic pose tick by ensuring that @see USkeletalMeshComponent::ShouldTickPose will return false on the currently set
  /// skeletal mesh.
  ///
  /// @returns      void
  virtual void BlockSkeletalMeshPoseTick() const;

  /// Whether the pawn's pose should be ticked manually this iteration because we are playing a montage.
  ///
  /// @param        bOutSimulate    Only relevant if the function return value is true. Whether the pose tick should only be simulated
  ///                               because this is a client replay, a remote move execution or a sub-stepped move iteration.
  /// @returns      bool            True if the pawn's pose should be ticked manually, false otherwise.
  virtual bool ShouldTickPose(bool* bOutSimulate = nullptr) const;

  /// Ticks the pawn's pose and consumes root motion if applicable.
  /// @attention Root motion from animations can only be handled correctly in networked games if the root motion mode (@see ERootMotionMode)
  /// is set to "RootMotionFromMontagesOnly".
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @param        bSimulate       Whether the pose tick should only be simulated (e.g. for client replay or remote move execution).
  /// @returns      void
  virtual void TickPose(float DeltaSeconds, bool bSimulate);

  /// Calls @see Client_DoNotCombineNextMove if a montage is currently blending. Due to the way montages are handled in multiplayer we
  /// should not combine moves while a montage is blending in or out.
  ///
  /// @returns      void
  virtual void Client_CheckMontageBlend();

  /// Checks whether the currently available root motion movement parameters should not be applied due to the configured values of
  /// @see bApplyRootMotionDuringBlendIn and @see bApplyRootMotionDuringBlendOut.
  ///
  /// @param        RootMotionMontage            The montage of the currently playing root motion montage instance.
  /// @param        RootMotionMontagePosition    The position in the currently playing root motion montage instance.
  /// @returns      bool                         True if the current root motion parameters should be discarded, false otherwise.
  virtual bool ShouldDiscardRootMotion(UAnimMontage* RootMotionMontage, float RootMotionMontagePosition) const;

  /// Clears montages at the end of the tick according to the passed policy.
  ///
  /// @param        Mesh      The skeletal mesh of the owning pawn.
  /// @param        Policy    The montage stepping policy that is currently being used (@see StepMontagePolicy).
  /// @returns      void
  virtual void ResetMontages(USkeletalMeshComponent* Mesh, EStepMontagePolicy Policy) const;

  /// Applies rotation from animation root motion to the updated component.
  /// @attention The pawn postition will only be adjusted to account for collisions when rotating around the yaw axis. Roll- and pitch-
  /// rotations will be set if they won't cause any blocking collisions, but no adjustment to the pawn position will be made.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  virtual void ApplyAnimRootMotionRotation(float DeltaSeconds);

  /// Calculates the velocity from animation root motion.
  ///
  /// @param        DeltaSeconds    The delta time to use.
  /// @returns      void
  virtual void CalculateAnimRootMotionVelocity(float DeltaSeconds);

  /// Allows for post-processing of the calculated root motion velocity.
  ///
  /// @param        RootMotionVelocity    The velocity calculated purely from the root motion animation.
  /// @param        DeltaSeconds          The delta time to use.
  /// @returns      FVector               The final velocity the pawn should assume.
  UFUNCTION(BlueprintNativeEvent, Category = "General Movement Component")
  FVector PostProcessAnimRootMotionVelocity(const FVector& RootMotionVelocity, float DeltaSeconds);
  virtual FVector PostProcessAnimRootMotionVelocity_Implementation(const FVector& RootMotionVelocity, float DeltaSeconds);

  /// Gets the position the passed montage is currently at (if being stepped). If nullptr is passed the position of the first found
  /// currently playing montage is returned. Includes montages that are blending out. Considers the current value of @see StepMontagePolicy
  /// and always returns -1 if the found montage does not fit the policy.
  ///
  /// @param        Mesh       The skeletal mesh of the owning pawn.
  /// @param        Montage    The montage to query the position for. If nullptr the first found currently playing montage will be queried.
  /// @returns      float      The position of the queried montage. Will return -1 if the passed montage is not playing, if no montage is
  ///                          playing at all, or if the montage does not fit the currently set montage stepping policy.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual float GetMontagePositionByPolicy(USkeletalMeshComponent* Mesh, UAnimMontage* Montage = nullptr) const;

  /// Gets the current weight/blended value of the passed montage (if being stepped). If nullptr is passed the weight of the first found
  /// currently playing montage is returned. Includes montages that are blending out. Considers the current value of @see StepMontagePolicy
  /// and always returns -1 if the found montage does not fit the policy.
  ///
  /// @param        Mesh       The skeletal mesh of the owning pawn.
  /// @param        Montage    The montage to query the weight for. If nullptr the first found currently playing montage will be queried.
  /// @returns      float      The blended value of the queried montage. Will return -1 if the passed montage is not playing, if no montage
  ///                          is playing at all, or if the montage does not fit the currently set montage stepping policy.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  virtual float GetMontageWeightByPolicy(USkeletalMeshComponent* Mesh, UAnimMontage* Montage = nullptr) const;

  UPROPERTY(Transient)
  /// The current root motion parameters.
  FRootMotionMovementParams RootMotionParams;

  UPROPERTY(BlueprintReadWrite, Category = "General Movement Component")
  /// Whether we currently have any root motion from animations to consider.
  bool bHasAnimRootMotion{false};

  UPROPERTY(BlueprintReadWrite, Category = "General Movement Component")
  /// Scaling factor applied to animation root motion translation on this pawn.
  float AnimRootMotionTranslationScale{1.f};

  UPROPERTY(BlueprintReadWrite, Category = "General Movement Component")
  /// Reference to the skeletal mesh of the owning pawn.
  USkeletalMeshComponent* SkeletalMesh{nullptr};

#pragma endregion

#pragma region Simulated Pawns

private:

  /// The previous movement mode of a simulated pawn.
  EGenMovementMode PreviousMovementModeSimulated{0};

protected:

  /// Used to maintain consistency between the root collision component and the bound data members for smoothed pawns. If
  /// @see bSmoothCollisionLocation is false (recommended) the root collision shape and extent will both be set from the target state of the
  /// interpolation, otherwise the extent will be interpolated.
  /// @see REPLICATE_COLLISION
  /// @see CurrentRootCollisionShape
  /// @see CurrentRootCollisionExtent
  ///
  /// @param        SmoothState    The interpolated state.
  /// @param        StartState     The start state of the interpolation.
  /// @param        TargetState    The target state of the interpolation.
  /// @returns      void
  virtual void MaintainRootCollisionCoherencySimulated(const FState& SmoothState, const FState& StartState, const FState& TargetState);

  /// Returns the movement mode the pawn had during the previous simulated tick.
  /// @attention All functions with the "Simulated" postfix should only be used for non-gameplay critical functionality (effects,
  /// animations, etc.) and they are not called on a dedicated server.
  ///
  /// @returns      EGenMovementMode    The previous movement mode of a simulated pawn.
  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  EGenMovementMode GetPreviousMovementModeSimulated() const;

  /// Called when a simulated (i.e. remotely controlled) pawn was updated. This is the preferred entry point for subclasses to implement
  /// custom logic for simulated pawns.
  /// @attention All functions with the "Simulated" postfix should only be used for non-gameplay critical functionality (effects,
  /// animations, etc.) and they are not called on a dedicated server.
  ///
  /// @param        DeltaSeconds    The current delta time.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, BlueprintCosmetic, Category = "General Movement Component")
  void MovementUpdateSimulated(float DeltaSeconds);
  virtual void MovementUpdateSimulated_Implementation(float DeltaSeconds) {}

  /// Called when the movement mode changes from airborne to grounded for a simulated (i.e. remotely controlled) pawn. This is called after
  /// @see MovementUpdateSimulated and @see OnMovementModeChangedSimulated (if applicable) have run.
  /// @attention All functions with the "Simulated" postfix should only be used for non-gameplay critical functionality (effects,
  /// animations, etc.) and they are not called on a dedicated server.
  ///
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintCosmetic, Category = "General Movement Component")
  void OnLandedSimulated();
  virtual void OnLandedSimulated_Implementation() {}

  /// Called when the movement mode changes for a simulated (i.e. remotely controlled) pawn. This is called before @see OnLandedSimulated
  /// (if applicable) but after @see MovementUpdateSimulated have run.
  /// @attention All functions with the "Simulated" postfix should only be used for non-gameplay critical functionality (effects,
  /// animations, etc.) and they are not called on a dedicated server.
  ///
  /// @param        PreviousMovementMode    The previous movement mode we changed from.
  /// @returns      void
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintCosmetic, Category = "General Movement Component")
  void OnMovementModeChangedSimulated(EGenMovementMode PreviousMovementMode);
  virtual void OnMovementModeChangedSimulated_Implementation(EGenMovementMode PreviousMovementMode) {}

#pragma endregion

#pragma region AI Interfaces

public:

  ///~ Begin UNavMovementComponent Interface
  void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed) override;
  void RequestPathMove(const FVector& MoveInput) override;
  bool CanStartPathFollowing() const override;
  bool CanStopPathFollowing() const override;
  float GetPathFollowingBrakingDistance(float MaxSpeed) const override;
  ///~ End UNavMovementComponent Interface

protected:

  /// The velocity requested by path following (@see RequestDirectMove).
  FVector RequestedVelocity{0};

  /// Whether max speed was requested by path following (@see RequestDirectMove).
  bool bRequestedMoveWithMaxSpeed{false};

  /// Adjust the nav movement output according to the nav agent properties.
  ///
  /// @param        NavOutput    The output from the nav movement component to be adjusted (the move input or a requested velocity).
  /// @returns      FVector      The adjusted vector.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  FVector NavAgentPropsAdjustment(const FVector& NavOutput);
  virtual FVector NavAgentPropsAdjustment_Implementation(const FVector& NavOutput);

  /// Calculates the current braking distance of the pawn for path following.
  ///
  /// @param        RootMotionVelocity    The current max speed of the pawn.
  /// @param        DeltaSeconds          The delta time to use.
  /// @returns      float                 The calculated braking distance.
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "General Movement Component")
  float CalculatePathFollowingBrakingDistance(float MaxSpeed, float DeltaSeconds) const;
  virtual float CalculatePathFollowingBrakingDistance_Implementation(float MaxSpeed, float DeltaSeconds) const;

public:

  ///~ Begin IRVOAvoidanceInterface
  void SetRVOAvoidanceUID(int32 UID) override;
  int32 GetRVOAvoidanceUID() override;
  void SetRVOAvoidanceWeight(float Weight) override;
  float GetRVOAvoidanceWeight() override;
  FVector GetRVOAvoidanceOrigin() override;
  float GetRVOAvoidanceRadius() override;
  float GetRVOAvoidanceHeight() override;
  float GetRVOAvoidanceConsiderationRadius() override;
  FVector GetVelocityForRVOConsideration() override;
  int32 GetAvoidanceGroupMask() override;
  int32 GetGroupsToAvoidMask() override;
  int32 GetGroupsToIgnoreMask() override;
#if UE_VERSION_OLDER_THAN(4, 26, 0)
  void SetAvoidanceGroup(int32 GroupFlags);
  void SetGroupsToAvoid(int32 GroupFlags);
  void SetGroupsToIgnore(int32 GroupFlags);
#else
  void SetAvoidanceGroupMask(int32 GroupFlags) override;
  void SetGroupsToAvoidMask(int32 GroupFlags) override;
  void SetGroupsToIgnoreMask(int32 GroupFlags) override;
#endif
  ///~ End IRVOAvoidanceInterface

  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  void SetAvoidanceGroupMask(const FNavAvoidanceMask& GroupMask);

  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  void SetGroupsToAvoidMask(const FNavAvoidanceMask& GroupMask);

  UFUNCTION(BlueprintCallable, Category = "General Movement Component")
  void SetGroupsToIgnoreMask(const FNavAvoidanceMask& GroupMask);

  /// Locks the avoidance velocity for the specified duration.
  ///
  /// @param        Avoidance    The avoidance manager.
  /// @param        Duration     Duration of the lock.
  /// @returns      void
  void SetAvoidanceVelocityLock(class UAvoidanceManager* Avoidance, float Duration);

protected:

  UPROPERTY(BlueprintReadOnly, Transient, Category = "RVO Avoidance")
  /// UID generated by the avoidance manager.
  int32 AvoidanceUID{0};

  /// Allows for user-defined post-processing of the calculated avoidance velocity.
  ///
  /// @param        NewVelocity    In: the calculated avoidance velocity. Out: the post-processed avoidance velocity.
  /// @returns      void
  virtual void PostProcessAvoidanceVelocity(FVector& NewVelocity) {}

  /// Whether RVO avoidance should be computed for this pawn.
  ///
  /// @returns      bool    True if this pawn is a server bot and has avoidance enabled.
  virtual bool ShouldComputeAvoidance();

  /// Updates the current velocity with the avoidance velocity.
  ///
  /// @param        DeltaTime    The delta time to use.
  /// @returns      void
  virtual void CalculateAvoidanceVelocity(float DeltaTime);

  /// Resets all data members involved in avoidance calculations to their default values.
  ///
  /// @returns      void
  virtual void ResetAvoidanceData();

private:

  /// Whether we are currently using avoidance.
  bool bIsUsingAvoidanceInternal{false};

  /// Whether the avoidance was already updated this frame.
  bool bWasAvoidanceUpdated{false};

  /// The remaining time of the avoidance velocity lock.
  float AvoidanceLockTimer{0.f};

  /// Forced avoidance velocity used when the there is an avoidance lock duration.
  FVector AvoidanceLockVelocity{0};

  /// Registers the movement component with the avoidance manager.
  ///
  /// @returns      void
  void EnableRVOAvoidance();

  /// Checks whether enabling/disabling of avoidance was requested by the user (@see bUseAvoidance).
  ///
  /// @returns      void
  void CheckAvoidance();

  /// Updates data in the RVO avoidance manager at the end of the tick if necessary.
  ///
  /// @returns      void
  void UpdateAvoidance();

#pragma endregion

public:

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tempo", meta = (ClampMin = "0", UIMin = "0"))
  /// The amount of acceleration applied to the pawn by the controller's directional input when grounded.
  float InputAccelerationGrounded{6000.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tempo", meta = (ClampMin = "0", UIMin = "0"))
  /// The amount of acceleration applied to the pawn by the controller's directional input when airborne.
  float InputAccelerationAirborne{1800.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tempo", meta = (ClampMin = "0", UIMin = "0"))
  /// The amount of acceleration applied to the pawn by the controller's directional input when in a fluid volume.
  float InputAccelerationBuoyant{1000.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tempo", meta = (ClampMin = "0", UIMin = "0"))
  /// How much deceleration is applied to the pawn when grounded.
  float BrakingDecelerationGrounded{3000.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tempo", meta = (ClampMin = "0", UIMin = "0"))
  /// How much deceleration is applied to the pawn when airborne.
  float BrakingDecelerationAirborne{200.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tempo", meta = (ClampMin = "0", UIMin = "0"))
  /// How much deceleration is applied to the pawn when in a fluid volume.
  float BrakingDecelerationBuoyant{1000.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tempo", meta = (ClampMin = "0", UIMin = "0"))
  /// The max speed the pawn should have. When this limit is exceeded, the appropriate over-max-speed-deceleration will be applied.
  float MaxDesiredSpeed{800.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tempo", meta = (ClampMin = "0", UIMin = "0"))
  /// How much deceleration is applied when the pawn is grounded and is exceeding the max desired speed.
  float OverMaxSpeedDecelerationGrounded{10000.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tempo", meta = (ClampMin = "0", UIMin = "0"))
  /// How much deceleration is applied when the pawn is airborne and is exceeding the max desired speed. In negative Z direction the max
  /// speed is determined by the terminal velocity of the current physics volume.
  float OverMaxSpeedDecelerationAirborne{1200.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tempo", meta = (ClampMin = "0", UIMin = "0"))
  /// How much deceleration is applied when the pawn is in a fluid and is exceeding the max desired speed.
  float OverMaxSpeedDecelerationBuoyant{3000.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Operation", meta = (ClampMin = "0", UIMin = "0", UIMax = "1"))
  /// If greater than 0 airborne input acceleration will be applied in a different way to allow for more precise maneuvering. The fall
  /// control is a multiplier for the configured value of @see InputAccelerationAirborne and will determine how much control the pawn can
  /// exert over its XY direction while in the air and under the influence of gravity. The fall control has no effect on the deceleration
  /// behaviour (@see BrakingDecelerationAirborne, @see OverMaxSpeedDecelerationAirborne).
  float FallControl{0.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Operation")
  /// When true, the pawn will smoothly rotate around the yaw axis to face the current input direction.
  bool bOrientToInputDirection{true};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Operation")
  /// When true, the pawn will smoothly rotate around the yaw axis to face the current control rotation direction. This setting takes
  /// precedence over "bOrientToInputDirection" if both options are set.
  bool bOrientToControlRotationDirection{false};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Operation", meta = (ClampMin = "0", UIMin = "0"))
  /// When orienting the pawn's rotation to the input or control rotation direction, this is the rate of rotation.
  float RotationRate{650.f};

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Operation", meta =
    (ClampMin = "0", ClampMax = "90", UIMin = "0", UIMax = "75"))
  /// Max angle in degrees of a surface the pawn can still walk on. Should only be set through @see SetWalkableFloorAngle.
  float WalkableFloorAngle{45.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Operation")
  /// If true, we will maintain the regular ground speed when walking up or down slopes by rescaling the location delta. If false, no
  /// scaling will be applied and the pawn will effectively move faster on slopes due to the additional Z component in the location delta.
  bool bRescaleSlopeDelta{true};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Operation", meta = (ClampMin = "0", UIMin = "0", UIMax = "100"))
  /// The maximum height the pawn can step up to while grounded.
  float MaxStepUpHeight{50.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Operation", meta = (ClampMin = "0", UIMin = "0", UIMax = "100"))
  /// When walking down a slope or off a ledge, the pawn will remain grounded if the floor underneath is closer than this threshold.
  float MaxStepDownHeight{50.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Operation", meta = (ClampMin = "-1", UIMin = "500", UIMax = "2000"))
  /// The max upward velocity the pawn should be able to absorb while grounded. While the Z velocity exceeds this value the pawn will not be
  /// able to land on the ground. Set to -1 to disable this functionality completely.
  float MaxGroundedVelocityZ{800.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Operation")
  /// Whether the pawn should assume the velocity of a moveable base while grounded.
  bool bMoveWithBase{true};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Operation")
  /// Whether the pawn is able to walk off ledges which exceed the max step down height when grounded.
  bool bCanWalkOffLedges{true};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Operation", meta =
    (ClampMin = "0", ClampMax = "1", UIMin = "0", UIMax = "1", EditCondition = "bCanWalkOffLedges"))
  /// When standing on a ledge the pawn will fall off if its collision shape perches further beyond the end of the ledge than the set
  /// threshold allows. This is percentage based with the center of the collision being 0 (i.e. the pawn will fall off as early as possible)
  /// and the outer boundary of the collision being 1 (i.e. the pawn will fall off as late as possible). For box collisions the threshold
  /// is internally treated as either 1 (if >= 0.5) or 0 (if < 0.5).
  float LedgeFallOffThreshold{0.5f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Operation", meta =
    (ClampMin = "0.0001", ClampMax = "1", UIMin = "0.1", UIMax = "1"))
  /// How deeply we need to be immersed in a fluid to enter the buoyant movement state with 1 being fully immersed in the fluid volume. The
  /// fluid volume should have the "Physics on Contact" flag enabled for this to work correctly.
  float BuoyantStateMinImmersion{0.8f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Operation", meta =
    (ClampMin = "0.0001", ClampMax = "1", UIMin = "0.1", UIMax = "1"))
  /// How deeply we need to be immersed in a fluid while still grounded to experience a slow down from the fluid volume. Will be clamped if
  /// the value exceeds the buoyant state min immersion. The fluid volume should have the "Physics on Contact" flag enabled for this to work
  /// correctly.
  float PartialImmersionThreshold{0.5f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Operation", meta = (ClampMin = "0", UIMin = "0"))
  /// Factor that determines how much slow down we experience while partially immersed in a fluid during grounded movement.
  float PartialImmersionSlowDown{10.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Operation", meta = (ClampMin = "0", UIMin = "0"))
  /// When in a buoyant state this is the minimum upward speed (i.e. positive Z-velocity) the pawn needs to have to be able to exit the
  /// fluid volume. Disabling this functionality (by setting the value to 0) may cause the pawn to repeatedly switch in and out of the
  /// buoyant state when close to the water line.
  float FluidMinExitSpeed{20.f};

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Operation")
  /// Only relevant when playing a montage with animation root motion: if false, no root motion will be applied while the montage is
  /// blending in.
  bool bApplyRootMotionDuringBlendIn{true};

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement|Operation")
  /// Only relevant when playing a montage with animation root motion: if false, no root motion will be applied while the montage is
  /// blending out.
  bool bApplyRootMotionDuringBlendOut{true};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics", meta = (ClampMin = "0", UIMin = "0"))
  /// Scale the effects of gravity acting on this pawn by this factor. Upward gravity is not supported.
  float GravityScale{1.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics", meta = (ClampMin = "0", UIMin = "0", UIMax = "1"))
  /// Friction of the ground i.e. how slippery the surface is. Only applies to grounded movement.
  float GroundFriction{1.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics", meta = (UIMin = "-0.5", UIMax = "0.5"))
  /// Fluid buoyancy.
  float Buoyancy{0.05f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics")
  /// Whether to impart the linear velocity of the current movement base when falling off it. Velocity is never imparted from a base that
  /// is simulating physics. Only applies if "MoveWithBase" is true.
  bool bImpartLinearBaseVelocity{true};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Physics")
  /// Whether to impart the angular velocity of the current movement base when falling off it. Velocity is never imparted from a base that
  /// is simulating physics. Only applies if "MoveWithBase" is true.
  bool bImpartAngularBaseVelocity{true};

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement", AdvancedDisplay)
  /// If true, the mass of the pawn will be taken into account when imparting the velocity of a base the pawn has just left.
  bool bConsiderMassOnImpartVelocity{false};

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement", AdvancedDisplay)
  /// Montages must be handled according to the set policy. The policy determines which types of montages have to be stepped manually via
  /// @see UGenMovementComponent::StepMontage in the movement component and which have to be played externally via Unreal's default node.
  /// This setting affects both local and simulated pawns and controls which types of montages are cleared at the end of each tick. Stepped
  /// montages take precedence over other montages.
  EStepMontagePolicy StepMontagePolicy{EStepMontagePolicy::RootMotionOnly};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", AdvancedDisplay)
  /// When true, input modification is ignored and we always accelerate up to the full max speed even if the analog stick is not at full
  /// tilt.
  bool bIgnoreInputModifier{false};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", AdvancedDisplay, meta =
    (ClampMin = "1", UIMin = "10", EditCondition = "!bIgnoreInputModifier"))
  /// The walking speed that we should accelerate up to when walking at minimum analog stick tilt.
  float MinAnalogWalkSpeed{100.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", AdvancedDisplay)
  /// If true, the contact normal of a hit will also be considered to check whether a floor is walkable when landing. This will often allow
  /// the pawn to land on hard edges that usually produce opposing impact normals.
  bool bLandOnEdges{true};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", AdvancedDisplay)
  /// If true, we will check whether something has moved into the pawn each frame and resolve the penetration if necessary. This is very
  /// expensive, deactivating can yield a large performance boost.
  bool bResolveExternalPenetrations{true};

  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement", AdvancedDisplay)
  /// If true, the extent of the root collision shape will be interpolated for simulated pawns in multiplayer. If false, the extent will
  /// always be set directly from the target state of the interpolation.
  bool bInterpolateCollisionExtent{true};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", AdvancedDisplay, meta = (ClampMin = "0.0001", UIMin = "1"))
  /// How far downwards the trace should go when updating the floor.
  float FloorTraceLength{500.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", AdvancedDisplay, meta = (ClampMin = "0", UIMin = "0"))
  /// Max distance allowed for depenetration when moving out of anything but pawns.
  float MaxDepenetrationWithGeometry{100.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", AdvancedDisplay, meta = (ClampMin = "0", UIMin = "0"))
  /// Max distance allowed for depenetration when moving out of other pawns.
  float MaxDepenetrationWithPawn{100.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Interaction")
  /// Whether the pawn should interact with physics objects in the world.
  bool bEnablePhysicsInteraction{true};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Interaction", meta =
    (UIMin = "0", EditCondition = "bEnablePhysicsInteraction"))
  /// Multiplier for the force that is applied to physics objects that are touched by the pawn.
  float TouchForceScale{1.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Interaction", meta =
    (ClampMin = "0", UIMin = "0", EditCondition = "bEnablePhysicsInteraction"))
  /// The minimum force applied to physics objects touched by the pawn.
  float MinTouchForce{0.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Interaction", meta =
    (ClampMin = "0", UIMin = "0", EditCondition = "bEnablePhysicsInteraction"))
  /// The maximum force applied to physics objects touched by the pawn.
  float MaxTouchForce{250.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Interaction", meta =
    (EditCondition = "bEnablePhysicsInteraction"))
  /// If true, "TouchForceScale" is applied per kilogram of mass of the affected object.
  bool bScaleTouchForceToMass{true};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Interaction", meta =
    (UIMin = "0", EditCondition = "bEnablePhysicsInteraction"))
  /// Multiplier for the force that is applied when the player collides with a blocking physics object.
  float PushForceScale{750000.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Interaction", meta =
    (UIMin = "0", EditCondition = "bEnablePhysicsInteraction"))
  /// Multiplier for the initial impulse force applied when the pawn bounces into a blocking physics object.
  float InitialPushForceScale{500.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Interaction", meta =
    (EditCondition = "bEnablePhysicsInteraction"))
  /// If true, "PushForceScale" is applied per kilogram of mass of the affected object.
  bool bScalePushForceToMass{false};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Interaction", meta =
    (EditCondition = "bEnablePhysicsInteraction"))
  /// If true, the applied push force will try to get the touched physics object to the same velocity as the pawn, not faster. This will
  /// only ever scale the force down and will never apply more force than calculated with regard to "PushForceScale".
  bool bScalePushForceToVelocity{true};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Interaction", meta =
    (EditCondition = "bEnablePhysicsInteraction"))
  /// If true, the push force location is adjusted using "PushForceZOffsetFactor". If false, the impact point is used.
  bool bUsePushForceZOffset{false};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Interaction", meta =
    (UIMin = "-1", UIMax = "1", EditCondition = "bEnablePhysicsInteraction && bUsePushForceZOffset"))
  /// Z-offset for the location the force is applied to the touched physics object (0 = center, 1 = top, -1 = bottom).
  float PushForceZOffsetFactor{-0.75f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Interaction", meta =
    (UIMin = "0", EditCondition = "bEnablePhysicsInteraction"))
  /// Multiplier for the gravity force applied to physics objects the pawn is walking on.
  float DownwardForceScale{1.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Interaction", meta =
    (ClampMin = "0", UIMin = "0", EditCondition = "bEnablePhysicsInteraction"))
  /// The force applied constantly per kilogram of mass of the pawn to all overlapping components.
  float RepulsionForce{2.5f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO Avoidance")
  /// Whether avoidance should be used for bots.
  bool bUseAvoidance{false};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO Avoidance")
  /// Indicates RVO behavior.
  float AvoidanceWeight{0.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO Avoidance")
  /// The radius around the pawn for which to consider avoidance of other agents.
  float AvoidanceConsiderationRadius{500.f};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO Avoidance")
  /// This pawn's avoidance group.
  FNavAvoidanceMask AvoidanceGroup;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO Avoidance")
  /// This pawn will avoid other agents that belong to one of the groups specified in the mask.
  FNavAvoidanceMask GroupsToAvoid;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO Avoidance")
  /// This pawn will ignore other agents that belong to one of the groups specified in the mask.
  FNavAvoidanceMask GroupsToIgnore;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NavMovement", meta = (DisplayAfter = "bUseFixedBrakingDistanceForPaths"))
  /// If true, the magnitude of the requested velocity will be used as max speed for direct bot movement.
  bool bUseRequestedVelocityMaxSpeed{false};
};

FORCEINLINE EGenMovementMode UGenOrganicMovementComponent::GetMovementMode() const
{
  return static_cast<EGenMovementMode>(MovementMode);
}

FORCEINLINE FVector UGenOrganicMovementComponent::GetProcessedInputVector() const
{
  return ProcessedInputVector;
}

FORCEINLINE float UGenOrganicMovementComponent::GetCurrentImmersionDepth() const
{
  return CurrentImmersionDepth;
}

FORCEINLINE bool UGenOrganicMovementComponent::IsAffectedByGravity() const
{
  return GetGravityZ() != 0.f;
}

FORCEINLINE bool UGenOrganicMovementComponent::IsMovingOnGround() const
{
  return MovementMode == static_cast<uint8>(EGenMovementMode::Grounded);
}

FORCEINLINE bool UGenOrganicMovementComponent::IsAirborne() const
{
  return MovementMode == static_cast<uint8>(EGenMovementMode::Airborne);
}

FORCEINLINE bool UGenOrganicMovementComponent::IsSwimming() const
{
  return MovementMode == static_cast<uint8>(EGenMovementMode::Buoyant);
}

FORCEINLINE bool UGenOrganicMovementComponent::IsFlying() const
{
  return IsAirborne() && GetGravityZ() == 0.f;
}

FORCEINLINE bool UGenOrganicMovementComponent::IsFalling() const
{
  return IsAirborne();
}

FORCEINLINE bool UGenOrganicMovementComponent::IsExceedingMaxGroundedVelocityZ() const
{
  return MaxGroundedVelocityZ >= 0.f && GetVelocity().Z > MaxGroundedVelocityZ;
}

FORCEINLINE bool UGenOrganicMovementComponent::HasAnimRootMotion() const
{
  return bHasAnimRootMotion;
}

FORCEINLINE float UGenOrganicMovementComponent::GetAnimRootMotionTranslationScale() const
{
  return AnimRootMotionTranslationScale;
}

FORCEINLINE void UGenOrganicMovementComponent::SetAnimRootMotionTranslationScale(float Scale)
{
  AnimRootMotionTranslationScale = Scale;
}

FORCEINLINE EGenMovementMode UGenOrganicMovementComponent::GetPreviousMovementModeSimulated() const
{
  return PreviousMovementModeSimulated;
}
