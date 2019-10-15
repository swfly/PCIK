// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BoneControllers/AnimNode_SkeletalControlBase.h"
#include "AnimNode_CharacterGroundingIK.generated.h"

/**
 * 
 */
USTRUCT(BlueprintInternalUseOnly)
struct FAnimNode_CharacterGroundingIK : public FAnimNode_SkeletalControlBase
{
	GENERATED_USTRUCT_BODY()
		//FPoseLink - this can be any combination 
			//of other nodes, not just animation sequences
		//	so you could have an blend space leading into 
			//a layer blend per bone to just use the arm
		//	and then pass that into the PoseLink
	UPROPERTY(EditAnywhere, Category = IK)
		FBoneReference LeftFoot;
	UPROPERTY(EditAnywhere, Category = IK)
		FBoneReference LeftFootPlacementRef;
	UPROPERTY(EditAnywhere, Category = IK)
		FBoneReference RightFoot;
	UPROPERTY(EditAnywhere, Category = IK)
		FBoneReference RightFootPlacementRef;
	UPROPERTY(EditAnywhere, Category = IK)
		FBoneReference Root;
	UPROPERTY(EditAnywhere, Category = IK, meta = (PinShownByDefault))
		bool useFeetHeight = false;
	UPROPERTY(EditAnywhere, Category = IK, meta = (PinShownByDefault))
		float deltaTime;
	UPROPERTY(EditAnywhere, Category = IK, meta = (PinShownByDefault))
		float smoothSpeed;
	UPROPERTY(EditAnywhere, Category = Debug)
		bool showGroundingIcon = false;
	UPROPERTY(EditAnywhere, Category = IK, meta = (PinShownByDefault))
		TEnumAsByte<ETraceTypeQuery> TraceChannel;
	float currentOffset = 0;

	FCompactPoseBoneIndex CachedLowerLimbIndexL = FCompactPoseBoneIndex(INDEX_NONE);
	FCompactPoseBoneIndex CachedUpperLimbIndexL = FCompactPoseBoneIndex(INDEX_NONE);

	FCompactPoseBoneIndex CachedLowerLimbIndexR = FCompactPoseBoneIndex(INDEX_NONE);
	FCompactPoseBoneIndex CachedUpperLimbIndexR = FCompactPoseBoneIndex(INDEX_NONE);
	float leftOffset;
	float rightOffset;

	void TwoBoneIK(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms, FCompactPoseBoneIndex end, FCompactPoseBoneIndex low, FCompactPoseBoneIndex up, float offset,float baseOffset);
public:
	virtual void EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms) override;
	bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
	void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;
};