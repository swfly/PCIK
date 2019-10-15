// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNode_CharacterGroundingIK.h"
#include "AnimInstanceProxy.h"
#include "DrawDebugHelpers.h"
#include "AnimationRuntime.h"
#include "AnimationCoreLibrary.h"
#include "TwoBoneIK.h"
#include "Kismet/KismetSystemLibrary.h"


void FAnimNode_CharacterGroundingIK::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	Root.Initialize(RequiredBones);
	LeftFoot.Initialize(RequiredBones);
	RightFoot.Initialize(RequiredBones);
	LeftFootPlacementRef.Initialize(RequiredBones);
	RightFootPlacementRef.Initialize(RequiredBones);

	FCompactPoseBoneIndex IKBoneCompactPoseIndex = LeftFoot.GetCompactPoseIndex(RequiredBones);
	CachedLowerLimbIndexL = FCompactPoseBoneIndex(INDEX_NONE);
	CachedUpperLimbIndexL = FCompactPoseBoneIndex(INDEX_NONE);
	if (IKBoneCompactPoseIndex != INDEX_NONE)
	{
		CachedLowerLimbIndexL = RequiredBones.GetParentBoneIndex(IKBoneCompactPoseIndex);
		if (CachedLowerLimbIndexL != INDEX_NONE)
		{
			CachedUpperLimbIndexL = RequiredBones.GetParentBoneIndex(CachedLowerLimbIndexL);
		}
	}
	IKBoneCompactPoseIndex = RightFoot.GetCompactPoseIndex(RequiredBones);
	CachedLowerLimbIndexR = FCompactPoseBoneIndex(INDEX_NONE);
	CachedUpperLimbIndexR = FCompactPoseBoneIndex(INDEX_NONE);
	if (IKBoneCompactPoseIndex != INDEX_NONE)
	{
		CachedLowerLimbIndexR = RequiredBones.GetParentBoneIndex(IKBoneCompactPoseIndex);
		if (CachedLowerLimbIndexR != INDEX_NONE)
		{
			CachedUpperLimbIndexR = RequiredBones.GetParentBoneIndex(CachedLowerLimbIndexR);
		}
	}
}

void FAnimNode_CharacterGroundingIK::TwoBoneIK(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms, FCompactPoseBoneIndex end, FCompactPoseBoneIndex low, FCompactPoseBoneIndex up, float offset, float baseOffset)
{
	if (abs(offset) < 0.1)
		return;
	// Get Local Space transforms for our bones. We do this first in case they already are local.
	// As right after we get them in component space. (And that does the auto conversion).
	// We might save one transform by doing local first...
	// Now get those in component space...
	FTransform LowerLimbCSTransform = Output.Pose.GetComponentSpaceTransform(low);
	FTransform UpperLimbCSTransform = Output.Pose.GetComponentSpaceTransform(up);
	FTransform EndBoneCSTransform = Output.Pose.GetComponentSpaceTransform(end);
	// Get current position of root of limb.
	// All position are in Component space.
	const FVector RootPos = UpperLimbCSTransform.GetTranslation();
	const FVector InitialJointPos = LowerLimbCSTransform.GetTranslation();
	FVector InitialEndPos = EndBoneCSTransform.GetTranslation();
	InitialEndPos += FVector(0, 0, offset);
	auto jointPos = LowerLimbCSTransform.GetTranslation() + FVector(0, offset/2, offset/2);
	AnimationCore::SolveTwoBoneIK(UpperLimbCSTransform, LowerLimbCSTransform, EndBoneCSTransform, jointPos, InitialEndPos, false, 1, 1);
	//return;
	UpperLimbCSTransform.AddToTranslation(FVector(0, 0, baseOffset));
	LowerLimbCSTransform.AddToTranslation(FVector(0, 0, baseOffset));
	EndBoneCSTransform.AddToTranslation(FVector(0, 0, baseOffset));
	OutBoneTransforms.Add(FBoneTransform(up, UpperLimbCSTransform));
	OutBoneTransforms.Add(FBoneTransform(low, LowerLimbCSTransform));
	OutBoneTransforms.Add(FBoneTransform(end, EndBoneCSTransform));
}

void FAnimNode_CharacterGroundingIK::EvaluateSkeletalControl_AnyThread(FComponentSpacePoseContext& Output, TArray<FBoneTransform>& OutBoneTransforms)
{
	auto component = Output.AnimInstanceProxy->GetSkelMeshComponent();
	const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
	// Get indices of the lower and upper limb bones and check validity.
	bool bInvalidLimb = false;

	//Get ground offset
	auto rootPos = component->GetComponentLocation();
	FHitResult hit;
	FCollisionQueryParams params;
	params.AddIgnoredActor(component->GetOwner());
	float rootHeight = rootPos.Z;
	float groundHeight = rootHeight;
	if (UKismetSystemLibrary::LineTraceSingle(component, rootPos + FVector(0, 0, 50), rootPos + FVector(0, 0, -50),
		TraceChannel,true,TArray<AActor*>(), showGroundingIcon ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, hit,true))
	{
		groundHeight = hit.Location.Z;
	}

	//Left foot
	FCompactPoseBoneIndex IKBoneCompactPoseIndex = LeftFootPlacementRef.GetCompactPoseIndex(BoneContainer);
	bool leftClipped = false;
	FTransform EndBoneTransformCSL = Output.Pose.GetComponentSpaceTransform(IKBoneCompactPoseIndex);
	auto refLocationL = Output.AnimInstanceProxy->GetSkelMeshComponent()->GetComponentTransform().TransformPosition(EndBoneTransformCSL.GetLocation());
	float leftFootHeight = refLocationL.Z;
	float leftGroundHeight = leftFootHeight;
	//if (component->GetWorld()->LineTraceSingleByChannel(hit, refLocationL + FVector(0, 0, 50), refLocationL + FVector(0, 0, useFeetHeight ? -50 : 0), ECollisionChannel::ECC_Visibility, params))
	if (UKismetSystemLibrary::LineTraceSingle(component, refLocationL + FVector(0, 0, 50), refLocationL + FVector(0, 0, useFeetHeight ? -50 : groundHeight - rootHeight),
		TraceChannel, true, TArray<AActor*>(), showGroundingIcon ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, hit, true, FLinearColor::Red, FLinearColor::Green, 1))
	{
		leftClipped = true;
		leftGroundHeight = hit.Location.Z + EndBoneTransformCSL.GetLocation().Z;
	}

	//Right foot
	IKBoneCompactPoseIndex = RightFootPlacementRef.GetCompactPoseIndex(BoneContainer);
	bool rightClipped = false;
	FTransform EndBoneTransformCSR = Output.Pose.GetComponentSpaceTransform(IKBoneCompactPoseIndex);
	auto refLocationR = Output.AnimInstanceProxy->GetSkelMeshComponent()->GetComponentTransform().TransformPosition(EndBoneTransformCSR.GetLocation());
	float rightFootHeight = refLocationR.Z;
	float rightGroundHeight = rightFootHeight;
	//if (component->GetWorld()->LineTraceSingleByChannel(hit, refLocationR + FVector(0, 0, 50), refLocationR + FVector(0, 0, useFeetHeight ?-50:0), ECollisionChannel::ECC_Visibility, params))
	if (UKismetSystemLibrary::LineTraceSingle(component, refLocationR + FVector(0, 0, 50), refLocationR + FVector(0, 0, useFeetHeight ? -50 : groundHeight - rootHeight),
		TraceChannel, true, TArray<AActor*>(), showGroundingIcon?EDrawDebugTrace::ForDuration: EDrawDebugTrace::None, hit, true,FLinearColor::Red,FLinearColor::Green,1))
	{
		rightClipped = true;
		rightGroundHeight = hit.Location.Z + EndBoneTransformCSR.GetLocation().Z;
	}

	float finalOffset = 0;
	float leftFootOffset = 0;
	float rightFootOffset = 0;
	{
		//Recalibrate offsets for root and feet
		TArray<float> offsets;
		if (useFeetHeight)
		{
			offsets.Add(leftGroundHeight - leftFootHeight);
			offsets.Add(rightGroundHeight - rightFootHeight);
		}
		else
			offsets.Add(groundHeight - rootHeight);
		finalOffset = FMath::Min<float>(offsets);
		currentOffset = FMath::FInterpTo(currentOffset, finalOffset, deltaTime, smoothSpeed);
		auto rootIndex = Root.GetCompactPoseIndex(BoneContainer);
		FTransform rootTransform = Output.Pose.GetComponentSpaceTransform(rootIndex);
		rootTransform.AddToTranslation(FVector(0, 0, currentOffset));
		//Output.Pose.SetComponentSpaceTransform(rootIndex, rootTransform);
		OutBoneTransforms.Add(FBoneTransform(rootIndex, rootTransform));
		//UE_LOG(LogTemp, Warning, TEXT("%f:%f,%f,%f<><><>%f,%f,%f"), currentOffset,groundHeight, leftGroundHeight, rightGroundHeight, rootHeight, leftFootHeight, rightFootHeight);
		//Do IK
		if (useFeetHeight)
		{
			leftFootOffset = leftGroundHeight - leftFootHeight - currentOffset;
			rightFootOffset = rightGroundHeight - rightFootHeight - currentOffset;
		}
		else
		{
			if (leftClipped)
				leftFootOffset = leftGroundHeight - leftFootHeight - currentOffset;
			if (rightClipped)
				rightFootOffset = rightGroundHeight - rightFootHeight - currentOffset;
		}
		leftOffset = FMath::FInterpTo(leftOffset, leftFootOffset, deltaTime, smoothSpeed);
		rightOffset = FMath::FInterpTo(rightOffset, rightFootOffset, deltaTime, smoothSpeed);
		TwoBoneIK(Output, OutBoneTransforms, LeftFoot.GetCompactPoseIndex(BoneContainer), CachedLowerLimbIndexL, CachedUpperLimbIndexL, leftOffset, currentOffset);
		TwoBoneIK(Output, OutBoneTransforms, RightFoot.GetCompactPoseIndex(BoneContainer), CachedLowerLimbIndexR, CachedUpperLimbIndexR, rightOffset, currentOffset);

	}
	if (showGroundingIcon)
	{
		//Draw ground
		DrawDebugSphere(Output.AnimInstanceProxy->GetSkelMeshComponent()->GetWorld(), rootPos + FVector(0,0,groundHeight), 15, 5, FColor(255, 0, 0), false, -1, 0, 1);
		//Left foot
		DrawDebugSphere(Output.AnimInstanceProxy->GetSkelMeshComponent()->GetWorld(), refLocationL + FVector(0, 0, leftFootOffset + currentOffset), 15, 5, FColor(255, 0, 0), false, -1, 0, 1);
		//Right foot
		DrawDebugSphere(Output.AnimInstanceProxy->GetSkelMeshComponent()->GetWorld(), refLocationR + FVector(0, 0, rightFootOffset + currentOffset), 15, 5, FColor(255, 0, 0), false, -1, 0, 1);
	}
	//OutBoneTransforms.Add(FBoneTransform(IKBoneCompactPoseIndex, EndBoneLocalTransform));
}


bool FAnimNode_CharacterGroundingIK::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
	if (!LeftFoot.IsValidToEvaluate(RequiredBones))
	{
		return false;
	}
	if (!LeftFootPlacementRef.IsValidToEvaluate(RequiredBones))
	{
		return false;
	}
	if (!RightFoot.IsValidToEvaluate(RequiredBones))
	{
		return false;
	}
	if (!RightFootPlacementRef.IsValidToEvaluate(RequiredBones))
	{
		return false;
	}
	if (!Root.IsValidToEvaluate(RequiredBones))
	{
		return false;
	}

	if (CachedUpperLimbIndexL == INDEX_NONE || CachedLowerLimbIndexL == INDEX_NONE)
	{
		return false;
	}
	if (CachedUpperLimbIndexR == INDEX_NONE || CachedLowerLimbIndexR == INDEX_NONE)
	{
		return false;
	}

	return true;
}