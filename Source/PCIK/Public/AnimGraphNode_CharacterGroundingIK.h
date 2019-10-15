// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_SkeletalControlBase.h"
#include "AnimNode_CharacterGroundingIK.h"
#include "AnimGraphNode_CharacterGroundingIK.generated.h"

/**
 * 
 */
UCLASS()
class UAnimGraphNode_CharacterGroundingIK : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category=Settings)
		FAnimNode_CharacterGroundingIK Node;

public:
	// UEdGraphNode interface
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FString GetNodeCategory() const override;
	virtual const FAnimNode_SkeletalControlBase* GetNode() const override { return &Node; }
};
